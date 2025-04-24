#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int	run(char **av, int start, int in_fd, char **env);

void ft_putstr(char *str, char *arg)
{
	if (str)
	{
		while(*str)
			write(STDERR, str++, 1);
	}
	if (arg)
	{
		while(*arg)
			write(STDERR, arg++, 1);
	}
	write(STDERR, "\n", 1);
}

void fatal_error(void)
{
	ft_putstr("error: fatal", NULL);
	exit(1);
}

int	is_pipe(char *str)
{
	return (strcmp(str, "|") == 0);
}

int	is_break(char *str)
{
	return (strcmp(str, ";") == 0);
}

int	is_cd(char *str)
{
	return (strcmp(str, "cd") == 0);
}

int do_cd(char **av, int start, int end, char **env)
{
    int status = 0;

    if (end - start != 2)
    {
        ft_putstr("error: cd: bad arguments",NULL);
        status = 1;
    }
    else if (chdir(av[start + 1]) != 0)
    {
        ft_putstr("error: cd: cannot change directory to ", av[start + 1]);
        status = 1;
    }
    if (av[end] && is_break(av[end]))
    {
        status = run(av, end + 1, STDIN, env);
    }
    return (status);    
}

char **parse_cmd(char **av, int start, int end)
{
    int len = end - start;
    char **cmd;
    int i;

    if (len <= 0)
        return (NULL);

    cmd = malloc (sizeof(char *) * (len + 1));

    for (i=0; i < len; i++)
    {
        cmd[i] = av[start + i];
    }
    cmd[len] = NULL;
    return (cmd);
}

int child(char **av, int start, int end, int in_fd, int got_pipe, int *pipe_fd, char **env)
{
    char **cmd;

    //redir
    if (in_fd != STDIN)
    {
        if (dup2(in_fd, STDIN) < 0)
            fatal_error();        
        close(in_fd);
    }
    if(got_pipe)
    {
        if (dup2(pipe_fd[1], STDOUT) < 0)
            fatal_error();
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    cmd = parse_cmd(av, start, end);
    execve(cmd[0],cmd,env);
	ft_putstr("error: cannot execute ", cmd[0]);
	free(cmd);
    exit(1);

}

int parent(char **av, pid_t pid, int end, int in_fd, int got_pipe, int *pipe_fd, char **env)
{
    int status;

    if (in_fd != STDIN)
    {   
        close(in_fd);
    }
    if (got_pipe)
	{
		close(pipe_fd[1]);
		status = run(av, end + 1, pipe_fd[0], env);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (av[end] && is_break(av[end]))
			status = run(av, end + 1, STDIN, env);
	}
	return (WEXITSTATUS(status));

}

int run(char **av, int start, int in_fd, char **env)
{
    pid_t   pid;
    int     pipe_fd[2];
    int     end;
    int     got_pipe = 0;

    //move start
    while (av[start] && is_break(av[start]))
        start++;
    if (!av[start])
        return (0);
    
    //check end
    end = start;
    while (av[end] && !is_break(av[end]) && !is_pipe(av[end]))
        end++;
    if (start >= end)
        return (0);

    //check cd
    if (is_cd(av[start]))
    {
        return (do_cd(av, start, end, env));
    }

    //check pipe
    got_pipe = av[end] && is_pipe(av[end]);
    if (got_pipe)
    {
        pipe(pipe_fd);
    }

    pid = fork();
    
    if (pid == 0)
    {
        child(av, start, end, in_fd, got_pipe, pipe_fd, env);
    }
    else
    {
        return (parent(av, pid, end, in_fd, got_pipe, pipe_fd, env));
    }
    return (1);
}

int main(int ac, char **av, char **env)
{
    int status;

    if (ac < 2)
        return (1);
    status = run(av + 1, 0, STDIN, env);
    while (waitpid(-1, NULL, 0) > 0)
    ;

    return (status);
}