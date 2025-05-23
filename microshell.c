#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2


int	run(char **av, int start, int in_fd, char **env);

//#include "microshell.h"
//./microshell ";" ";" /bin/pwd ";" echo NO ";" /bin/ls "|" /bin/grep micro "|" /bin/wc -l ";" cd .. ";" /bin/pwd ";" ";" 


void	ft_putstr(char* str, char *arg)
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
	write (STDERR, "\n", 1);
}
void	fatal_error(void)
{
	ft_putstr("error: fatal", NULL);
	exit (1);
}
int	is_pipe(char* str)
{
	return (strcmp(str, "|") == 0);
}
int	is_break(char* str)
{
	return (strcmp(str, ";") == 0);
}
int	is_cd(char* str)
{
	return (strcmp(str, "cd") == 0);
}

void	redirection(int got_pipe, int in_fd, int *pipe_fd)
{
	if(in_fd != STDIN)
	{
		if (dup2(in_fd, STDIN) < 0)
			fatal_error();
		if (close(in_fd) != 0)
			fatal_error();
	}
	if (got_pipe)
	{
		if (dup2(pipe_fd[1], STDOUT) < 0)
			fatal_error();
		if (close(pipe_fd[0]) != 0 || close(pipe_fd[1]) != 0 )
			fatal_error();
	}
	
}
char	**parse_arg(char **av, int start, int end)
{
	int	len = end - start;
	int	i;
	char **cmd;

	if (len <= 0)
		return (NULL);
	
	cmd = malloc(sizeof(char *) * (len + 1));
	if(!cmd)
		return (NULL);

	//arg_dup
	for (i = 0; i < len; i++)
		cmd[i] = av[start + i];
	cmd[len] = NULL;

	return (cmd);
}

void	child(char **av, int start, int end, int in_fd, int *pipe_fd, int got_pipe, char **env)
{
	redirection(got_pipe, in_fd, pipe_fd);
	char **cmd = parse_arg(av, start, end);
	if (!cmd)
		exit(1);
	execve(cmd[0], cmd, env);
	ft_putstr("error: cannot execute ", cmd[0]);
	free(cmd);
	exit(1);

}

int parent(char **av, int end, pid_t pid, int in_fd, int *pipe_fd, int got_pipe, char **env)
{
	int	status;

	//close previous pipe fd
	if (in_fd != STDIN)
	{
		if (close(in_fd) != 0)
			fatal_error();
	}
	
	if (got_pipe) //program continues with pipe
	{
		if (close(pipe_fd[1]) != 0)
			fatal_error();
		status = run(av, end + 1, pipe_fd[0], env);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (av[end] && is_break(av[end])) //program continues with break
			status = run(av, end + 1, STDIN, env);
	}
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	return (1);
}
int	handle_cd(char **av, int start, int end, char **env)
{
	int	status = 0;

	if (end - start != 2)
	{
		ft_putstr("error: cd: bad arguments", NULL);
		status = 1;
	}
	else
	{
		if (chdir(av[start + 1]) != 0)
		{
			ft_putstr("error: cd: cannot change directory to ", av[start + 1]);
			status = 1;
		}
	}
	if (av[end])
	{
		if (is_pipe(av[end]) || is_break(av[end]))
		{
			return (run(av, end + 1, STDIN, env));
		}
	}
	return (status);
}

int	run(char **av, int start, int in_fd, char **env)
{
	int		pipe_fd[2];
	pid_t	pid;
	int		end;
	int		got_pipe = 0;

	//move start
	while (av[start] && is_break(av[start]))
		start++;

	//find end
	end = start;
	while (av[end] && !is_pipe(av[end]) && !is_break(av[end]))
		end++;
	
	//check for end of program
	if(!av[start] || start >= end)
		return (0);

	//check cd
	if (av[start] && is_cd(av[start]))
		return(handle_cd(av, start, end, env));

	//check for pipe
	got_pipe = av[end] && is_pipe(av[end]);
	if (got_pipe) //make pipe
	{
		if (pipe(pipe_fd) != 0)
			fatal_error();
	}

	//fork
	pid = fork();
	if (pid < 0)
		fatal_error();
	
	if (pid == 0) //child
	{
		child(av, start, end, in_fd, pipe_fd, got_pipe, env);
	}
	else //parent
	{
		return(parent(av, end, pid, in_fd, pipe_fd, got_pipe, env));
	}

	return (1);
	
}

int	main (int ac, char **av, char **env)
{
	int status = 0;

	//check arg count
	if (ac < 2)
		return (1);
	
	status = run(av + 1, 0, STDIN, env);
	
	//wait for zombie
	while (waitpid(-1, NULL, 0) > 0)
	;

	return (status);
}


















/*
int	find_end(char **argv, int start)
{
	int i = start;

	while (argv[i] && !is_pipe(argv[i]) && !is_break(argv[i]))
		i++;
	return (i);
}
void	set_redirection(int in_fd, int *pipe_fd, int got_pipe)
{
	if (in_fd != STDIN)
	{
		if (dup2(in_fd, STDIN) < 0)
			fatal_error();
		close (in_fd);
	}
	if (got_pipe)
	{
		if (dup2(pipe_fd[1], STDOUT) < 0)
			fatal_error();
		close(pipe_fd[0]);
		close(pipe_fd[1]); 
	}
}
char**	parse_argv(char **av, int start, int end)
{
	if (start >= end)
		return (NULL);
	char **cmd = malloc(sizeof(char*) * (end - start + 1));
	if (!cmd)
		return (NULL);
	for (int i = 0; i < end - start; i++)
		cmd[i] = av[start + i];
	cmd[end - start] = NULL;
	return (cmd);
}
int handle_cd(char **av, int start, int end, char **env)
{
    
    // Check if correct number of arguments (cd + path)
    if (end - start != 2)
    {
        ft_putstr("error: cd: bad arguments", NULL);
        return 1;
    }
    
    // Try to change directory
    if (chdir(av[start + 1]) != 0)
    {
        ft_putstr("error: cd: cannot change directory to ", av[start + 1]);
        return (1);
    }
    
    // After CD completes, check for pipe or semicolon
    if (av[end])
    {
        if (is_pipe(av[end]))
        {
            return run(av, end + 1, STDIN, env);
        }
        else if (is_break(av[end])) {
            return run(av, end + 1, STDIN, env);
        }
    }
    return (0);
}

void	child(char **av, int start, int end, int in_fd, int *pipe_fd, int found_pipe, char **env)
{
	//redir
	set_redirection(in_fd, pipe_fd, found_pipe);
	//parse argv
	char **cmd = parse_argv(av, start, end);
	//exec
	execve(cmd[0], cmd, env);
	ft_putstr("error: cannot execute ", cmd[0]);
	free(cmd);
	exit(1);
}

int parent (char **av, int end, pid_t pid, int in_fd, int *pipe_fd, int found_pipe, char **env)
{
	int	status = 0;

	//close fd
	if (in_fd != STDIN)
		close(in_fd);
	if (found_pipe)
		close(pipe_fd[1]);

	if (found_pipe)
	{
		status = run(av, end + 1, pipe_fd[0], env);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (av[end] && is_break(av[end]))
			status = run(av, end + 1, STDIN, env);
	}
	return WEXITSTATUS(status);
}

int	run(char **av, int start, int in_fd, char **env)
{
	int		status = 0;
	int		pipe_fd[2];
	pid_t	pid;
	int		end = find_end(av, start);
	int		found_pipe;

	//check end of command
	if (start >= end)
		return (0);
	//handle cd
	if (start < end && is_cd(av[start])) {
		return handle_cd(av, start, end, env);
	}
	//check_pipe
	found_pipe = (av[end] && is_pipe(av[end]));
	if (found_pipe)
	{
		if (pipe(pipe_fd) != 0)
			fatal_error();
	}
	//fork
	pid = fork();
	if (pid < 0)
		fatal_error();
	if (pid == 0)
	{
		child(av, start, end, in_fd, pipe_fd, found_pipe, env);
	}
	else
	{
		return (parent(av, end, pid, in_fd, pipe_fd, found_pipe, env));
	}


	return (status);
}

int	main (int ac, char **av, char **env)
{
	int	status = 0;

	if (ac < 2)
		return (0);
	status = run(av + 1, 0, STDIN, env);
	while (waitpid(-1, NULL, 0) > 0)
	;
	return(status);
}
*/
