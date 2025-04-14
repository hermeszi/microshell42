#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define TYPE_END 0
#define TYPE_PIPE 1
#define TYPE_BREAK 2

typedef struct s_cmd
{
	char **args;
	int type;
	int pipe[2];
} t_cmd;

// Error handling functions
void	fatal_error(void)
{
	write(STDERR, "error: fatal\n", 13);
	exit(1);
}

void	exec_error(char *str)
{
	write(STDERR, "error: cannot execute ", 22);
	write(STDERR, str, strlen(str));
	write(STDERR, "\n", 1);
}

void	cd_arg_error(void)
{
	write(STDERR, "error: cd: bad arguments\n", 25);
}

void	cd_path_error(char *path)
{
	write(STDERR, "error: cd: cannot change directory to ", 38);
	write(STDERR, path, strlen(path));
	write(STDERR, "\n", 1);
}

// Command parsing
int	is_pipe(char *str)
{
	return (strcmp(str, "|") == 0);
}

int	is_break(char *str)
{
	return (strcmp(str, ";") == 0);
}

int	get_cmds_count(char **av)
{
	int i = 0;
	int count = 0;

	while (av[i])
	{
		if (!is_break(av[i]) && !is_pipe(av[i]))
		{
			count++;
			while (av[i] && !is_break(av[i]) && !is_pipe(av[i]))
				i++;
		}
		else
			i++;
	}
	return (count);
}

// Handle built-in cd command
int	exec_cd(char **args)
{
	// Check for correct number of arguments
	if (!args[1] || args[2])
	{
		cd_arg_error();
		return (1);
	}
	
	// Try to change directory
	if (chdir(args[1]) != 0)
	{
		cd_path_error(args[1]);
		return (1);
	}
	return (0);
}

// Check if command is 'cd'
int	is_cd(char *str)
{
	return (strcmp(str, "cd") == 0);
}

// Parse arguments into command structures
t_cmd	*parse_cmds(char **av)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int cmd_count = get_cmds_count(av);
	t_cmd *cmds = malloc(sizeof(t_cmd) * (cmd_count + 1));
	
	if (!cmds)
		fatal_error();
	
	while (av[i])
	{
		if (!is_break(av[i]) && !is_pipe(av[i]))
		{
			// Count arguments for this command
			int start = i;
			int arg_count = 0;
			
			while (av[i] && !is_break(av[i]) && !is_pipe(av[i]))
			{
				arg_count++;
				i++;
			}
			
			// Allocate space for arguments
			cmds[j].args = malloc(sizeof(char *) * (arg_count + 1));
			if (!cmds[j].args)
				fatal_error();
			
			// Copy arguments
			for (k = 0; k < arg_count; k++)
				cmds[j].args[k] = av[start + k];
			cmds[j].args[k] = NULL;
			
			// Set command type
			if (av[i] && is_pipe(av[i]))
			{
				cmds[j].type = TYPE_PIPE;
				i++;
			}
			else if (av[i] && is_break(av[i]))
			{
				cmds[j].type = TYPE_BREAK;
				i++;
			}
			else
				cmds[j].type = TYPE_END;
			
			j++;
		}
		else
			i++;
	}
	
	// Add sentinel value
	cmds[j].args = NULL;
	return (cmds);
}

// Execute a command with piping
int	exec_cmd(t_cmd *cmd, int in_fd, char **env)
{
	pid_t pid;
	int status = 0;
	
	// Handle built-in cd command
	if (cmd->args && is_cd(cmd->args[0]))
		return (exec_cd(cmd->args));
	
	// Create pipe if needed
	if (cmd->type == TYPE_PIPE)
	{
		if (pipe(cmd->pipe) != 0)
			fatal_error();
	}
	
	// Fork process
	pid = fork();
	if (pid < 0)
		fatal_error();
	
	if (pid == 0) // Child process
	{
		// Handle input redirection
		if (in_fd != STDIN)
		{
			if (dup2(in_fd, STDIN) < 0)
				fatal_error();
			close(in_fd);
		}
		
		// Handle output redirection for pipes
		if (cmd->type == TYPE_PIPE)
		{
			if (dup2(cmd->pipe[1], STDOUT) < 0)
				fatal_error();
			close(cmd->pipe[0]);
			close(cmd->pipe[1]);
		}
		
		// Execute command
		if (cmd->args && cmd->args[0])
		{
			execve(cmd->args[0], cmd->args, env);
			exec_error(cmd->args[0]);
		}
		exit(1);
	}
	else // Parent process
	{
		// Close pipe ends appropriately
		if (in_fd != STDIN)
			close(in_fd);
		
		if (cmd->type == TYPE_PIPE)
			close(cmd->pipe[1]);
		
		// Wait for non-piped commands to finish
		if (cmd->type != TYPE_PIPE)
		{
			waitpid(pid, &status, 0);
			return (WEXITSTATUS(status));
		}
		
		return (0);
	}
}

// Execute all commands in the array
int	exec_cmds(t_cmd *cmds, char **env)
{
	int i = 0;
	int in_fd = STDIN;
	int status = 0;
	
	while (cmds[i].args)
	{
		status = exec_cmd(&cmds[i], in_fd, env);
		
		// Set input for next command if piped
		if (cmds[i].type == TYPE_PIPE)
			in_fd = cmds[i].pipe[0];
		else
			in_fd = STDIN;
		
		i++;
	}
	
	// Wait for any remaining processes
	while (waitpid(-1, NULL, 0) > 0)
		;
		
	return (status);
}

// Free allocated memory
void	free_cmds(t_cmd *cmds)
{
	int i = 0;
	
	while (cmds[i].args)
	{
		free(cmds[i].args);
		i++;
	}
	free(cmds);
}

int	main(int ac, char **av, char **env)
{
	t_cmd *cmds;
	int status;
	
	if (ac < 2)
		return (0);
	
	// Parse commands from arguments
	cmds = parse_cmds(av + 1);
	
	// Execute commands
	status = exec_cmds(cmds, env);
	
	// Clean up
	free_cmds(cmds);
	
	return (status);
}