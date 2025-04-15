#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int exec_recursive(char **argv, int start, int in_fd, char **env);

// Error handling functions
void fatal_error(void)
{
    write(STDERR, "error: fatal\n", 13);
    exit(1);
}

void exec_error(char *str)
{
    write(STDERR, "error: cannot execute ", 22);
    write(STDERR, str, strlen(str));
    write(STDERR, "\n", 1);
}

void cd_arg_error(void)
{
    write(STDERR, "error: cd: bad arguments\n", 25);
}

void cd_path_error(char *path)
{
    write(STDERR, "error: cd: cannot change directory to ", 38);
    write(STDERR, path, strlen(path));
    write(STDERR, "\n", 1);
}

// Helper functions
int is_pipe(char *str)
{
    return (strcmp(str, "|") == 0);
}

int is_break(char *str)
{
    return (strcmp(str, ";") == 0);
}

int is_cd(char *str)
{
    return (strcmp(str, "cd") == 0);
}

// int print_cwd(void)
// {
//     char cwd[1024];
    
//     if (getcwd(cwd, sizeof(cwd)) != NULL)
//     {
//         printf("Current working directory: %s\n", cwd);
//     } 
//     else
//     {
//         perror("getcwd() error");
//         return 1;
//     }
    
//     return 0;
// }

// Execute built-in cd command
int exec_cd(char **args)
{
    if (!args[1] || args[2])
    {
        cd_arg_error();
        return 1;
    }
    if (chdir(args[1]) != 0)
    {
        cd_path_error(args[1]);
        return 1;
    }
    return 0;
}

// Find the end of the current command (pipe or break or end)
int find_cmd_end(char **argv, int start)
{
    int i = start;
    
    while (argv[i] && !is_pipe(argv[i]) && !is_break(argv[i]))
        i++;
    
    return i;
}

// Prepare command arguments
char **prepare_cmd_args(char **argv, int start, int end)
{
    char **cmd = malloc(sizeof(char *) * (end - start + 1));
    if (!cmd)
        fatal_error();
        
    for (int i = 0; i < end - start; i++)
        cmd[i] = argv[start + i];
    cmd[end - start] = NULL;
    
    return cmd;
}

// Setup input/output redirections
void setup_redirections(int in_fd, int *pipe_fd, int need_pipe)
{
    // Setup input redirection
    if (in_fd != STDIN)
    {
        if (dup2(in_fd, STDIN) < 0)
            fatal_error();
        close(in_fd);
    }
    
    // Setup output redirection for pipes
    if (need_pipe)
    {
        if (dup2(pipe_fd[1], STDOUT) < 0)
            fatal_error();
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }
}

// Handle child process execution
void child_process(char **argv, int start, int end, int in_fd, int *pipe_fd, int need_pipe, char **env)
{
    // Setup redirections
    setup_redirections(in_fd, pipe_fd, need_pipe);
    
    // Prepare command arguments
    char **cmd = prepare_cmd_args(argv, start, end);
    
    // Execute command
    execve(cmd[0], cmd, env);
    exec_error(cmd[0]);
    free(cmd);
    exit(1);
}

// Handle parent process logic and recursion
int parent_process(char **argv, int end, int in_fd, int *pipe_fd, int need_pipe, pid_t pid, char **env)
{
    int status = 0;
    
    // Close unused file descriptors
    if (in_fd != STDIN)
        close(in_fd);
        
    if (need_pipe)
        close(pipe_fd[1]);
    
    // If this is a pipe, recursively execute the next command
    if (need_pipe)
    {
        status = exec_recursive(argv, end + 1, pipe_fd[0], env);
    }
    else
    {
        // Wait for the child process
        waitpid(pid, &status, 0);
        
        // If we've reached a semicolon, recursively execute the next command set
        if (argv[end] && is_break(argv[end]))
            status = exec_recursive(argv, end + 1, STDIN, env);
    }
    
    return WEXITSTATUS(status);
}

// Handle CD command and continue with pipe or semicolon if needed
int handle_cd_and_continue(char **argv, int start, int end, char **env)
{
    int status;
    
    // Extract cd arguments
    char *cd_args[3] = {argv[start], end - start > 1 ? argv[start + 1] : NULL, NULL};
    
    // Execute CD command
    status = exec_cd(cd_args);
    
    // Check if there's a pipe or semicolon after CD to continue processing
    if (argv[end])
    {
        if (is_pipe(argv[end]))
        {
            // CD doesn't output anything to pipe, so just continue with next command
            return exec_recursive(argv, end + 1, STDIN, env);
        }
        else if (is_break(argv[end]))
        {
            // Continue with next command after semicolon
            return exec_recursive(argv, end + 1, STDIN, env);
        }
    }
    
    return status;
}

// Execute a command recursively
int exec_recursive(char **argv, int start, int in_fd, char **env)
{
    int end = find_cmd_end(argv, start);
    int status = 0;
    int pipe_fd[2];
    pid_t pid;
    
    // Handle empty command
    if (start >= end)
        return 0;
        
    // Handle built-in cd
    if (is_cd(argv[start]))
    {
        return handle_cd_and_continue(argv, start, end, env);
    }
    
    // Check if we need to pipe
    int need_pipe = argv[end] && is_pipe(argv[end]);
    
    // Create pipe if needed
    if (need_pipe)
    {
        if (pipe(pipe_fd) != 0)
            fatal_error();
    }
    
    // Fork process
    pid = fork();
    if (pid < 0)
        fatal_error();
    
    if (pid == 0) // Child process
    {
        child_process(argv, start, end, in_fd, pipe_fd, need_pipe, env);
    }
    else // Parent process
    {
        return parent_process(argv, end, in_fd, pipe_fd, need_pipe, pid, env);
    }
    
    return status; // This point is never reached but avoids compiler warning
}

int main(int ac, char **av, char **env)
{
    int status;
    
    if (ac < 2)
        return 0;
    
    status = exec_recursive(av + 1, 0, STDIN, env);
    
    // Wait for any remaining processes
    while (waitpid(-1, NULL, 0) > 0)
        ;
        
    return status;
}