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

int	find_end(char **av, int start)
{
	int	i = start;

	while(av[i] && !is_break(av[i]) && !is_pipe(av[i]))
		i++;
	return (i);
}

int		cd(char **av, int start, int end, char **env)
{
	int	status = 0;

	if (end - start < 2)
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
		if (is_break(av[end]))
		{
			return (run(av, end + 1, STDIN, env));
		}
	}
	return (status);
}

char **parse_cmd(char **av, int start, int end)
{
	char	**cmd;
	int		len = end - start;
	int		i;

	cmd = malloc(sizeof(char*) * (len + 1));
	if (!cmd)
		fatal_error();
	for (i=0; i < len; i++)
	{
		cmd[i] = av[start + i];
	}
	cmd[len] = NULL;
	return (cmd);
}

void	child(char **av, int start, int end, int in_fd, int *pipe_fd, int got_pipe, char **env)
{
	char	**cmd;

	//redirection
	if (in_fd != STDIN)
	{
		if(dup2(in_fd, STDIN) < 0)
		{
			//write(2, "child", 5);
			fatal_error();
		}
		if(close(in_fd) != 0)
		{
			//write(2, "child1", 6);
			fatal_error();
		}
	}
	if (got_pipe)
	{
		if (dup2(pipe_fd[1], STDOUT) < 0)
		{
			//write(2, "child2", 6);
			fatal_error();
		}
		if((close(pipe_fd[1]) != 0) || (close(pipe_fd[0]) != 0))
		{
			//write(2, "child3", 6);
			fatal_error();
		}
	}
	cmd = parse_cmd(av, start, end);
	execve(cmd[0], cmd, env);
	ft_putstr("error: cannot execute ", cmd[0]);
	free(cmd);
	exit(1);
}

int		parent(char **av, int end, pid_t pid, int in_fd, int *pipe_fd, int got_pipe, char **env)
{
	int status = 0;

	//close pipe
	if (in_fd != STDIN)
	{
		if(close(in_fd) != 0)
		{
			//write(2, "parent", 6);
			fatal_error();
		}
	}
	if(got_pipe)
	{
		if(close(pipe_fd[1]) != 0)
		{
			//write(2, "parent", 6);
			fatal_error();
		}
		status = run(av, end + 1, pipe_fd[0], env);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (av[end] && is_break(av[end]))
			status = run(av, end + 1, STDIN, env);
	}
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	return (status);
}

int	run(char **av, int start, int in_fd, char **env)
{
	int		pipe_fd[2];
	pid_t	pid;
	int		got_pipe;
	int		end;

	//move start
	while(av[start] && is_break(av[start]))
		start++;
	
	if (!av[start])
		return (0);

	//find end
	end = find_end(av, start);

	if (start >= end)
		return (0);

	//check cd
	if(av[start] && is_cd(av[start]))
		return (cd(av, start, end, env));
	
	got_pipe = av[end] && is_pipe(av[end]);
	//check pipe
	if (got_pipe)
	{
		if (pipe(pipe_fd) != 0)
		{
			//write(2, "pipe", 4);
			fatal_error();
		}
	}

	pid = fork();
	if (pid < 0)
	{
		//write(2, "fork", 4);
		fatal_error();
	}
	
	if(pid == 0)
	{
		child(av, start, end, in_fd, pipe_fd, got_pipe, env);
	}
	else
	{
		return (parent(av, end, pid, in_fd, pipe_fd, got_pipe, env));
	}
	return (1);
}

int	main(int ac, char **av, char **env)
{
	int	status = 0;

	if (ac < 2)
		return (0);

	status = run(av + 1, 0, STDIN, env);

	while(waitpid(-1, NULL, 0) > 0)
	;
	return (status);
}