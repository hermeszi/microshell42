#ifndef MICROSHELL_H
# define MICROSHELL_H

#include <unistd.h>     // fork, execve, chdir, dup, dup2, pipe, close
#include <sys/wait.h>   // waitpid
#include <stdlib.h>     // malloc, free, exit
#include <stdio.h>      // perror
#include <string.h>     // strcmp, strncmp
#include <signal.h>     // signal, kill

#endif