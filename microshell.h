#ifndef MICROSHELL_H
# define MICROSHELL_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int	run(char **av, int start, int in_fd, char **env);


#endif