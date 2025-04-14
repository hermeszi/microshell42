#include <unistd.h>     // fork, execve, chdir, dup, dup2, pipe, close
#include <sys/wait.h>   // waitpid
#include <stdlib.h>     // malloc, free, exit
#include <stdio.h>      // perror
#include <string.h>     // strcmp, strncmp
#include <signal.h>     // signal, kill

// Token identifiers
# define CMD 0
# define PIPE 1
# define BREAK 2

// Error messages
# define ERR_FATAL "error: fatal\n"
# define ERR_EXEC "error: cannot execute %s\n"
# define ERR_CD_ARGS "error: cd: bad arguments\n"
# define ERR_CD_DIR "error: cd: cannot change directory to %s\n"

typedef struct	s_cmd
{
	char    **args;
	int     type;
	int     pipe_fd[2];
} t_cmd;

int	is_pipe(char* str)
{
	if (strncmp(str, "|", 2) == 0)
		return (1);
	else
		return (0);
}

int	is_colon(char* str)
{
	if (strncmp(str, ";", 2) == 0)
		return (1);
	else
		return (0);
}

void	print_token(char **argv)
{
	int	i;

	i = 1;
	while (argv[i])
	{
		if(is_pipe(argv[i]))
			printf("-[PIP]-");
		else if (is_colon(argv[i]))
			printf("-[BRK]\n");
		else
			printf("-[%s]-", argv[i]);
		i++;
	}
	printf("\n");
}

int	count_commands(char **argv)
{
	int i = 1;
	int count = 0;
	
	while (argv[i])
	{
		if (!is_pipe(argv[i]) && !is_colon(argv[i]))
		{
			count++;
			while (argv[i] && !is_pipe(argv[i]) && !is_colon(argv[i]))
				i++;
		}
		else
			i++;
	}
	return (count);
}

void free_commands(t_cmd *cmds)
{
    int i = 0;
    
    while (cmds[i].args)
    {
        free(cmds[i].args);
        i++;
    }
    free(cmds);
}

int	main(int argc, char **argv, char **env)
{
	//t_cmd	*cmds;
	//int		status;
	(void) env;

	if (argc < 2)
		return (write(2, ERR_FATAL, strlen(ERR_FATAL)), 1);
	print_token(argv);
	printf("Count_command: %d\n",count_commands(argv));

	return (0);
}
