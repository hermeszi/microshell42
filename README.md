# microshell42

```sh
Assignment name  : microshell
Expected files   : microshell.c
Allowed functions: malloc, free, write, close, fork, waitpid, signal, kill, exit, chdir, execve, dup, dup2, pipe, strcmp, strncmp
--------------------------------------------------------------------------------------

Write a program that will behave like executing a shell command
- The command line to execute will be the arguments of this program
- Executable's path will be absolute or relative but your program must not build a path (from the PATH variable for example)
- You must implement "|" and ";" like in bash
	- we will never try a "|" immediately followed or preceded by nothing or "|" or ";"
- Your program must implement the built-in command cd only with a path as argument (no '-' or without parameters)
	- if cd has the wrong number of argument your program should print in STDERR "error: cd: bad arguments" followed by a '\n'
	- if cd failed your program should print in STDERR "error: cd: cannot change directory to path_to_change" followed by a '\n' with path_to_change replaced by the argument to cd
	- a cd command will never be immediately followed or preceded by a "|"
- You don't need to manage any type of wildcards (*, ~ etc...)
- You don't need to manage environment variables ($BLA ...)
- If a system call, except execve and chdir, returns an error your program should immediatly print "error: fatal" in STDERR followed by a '\n' and the program should exit
- If execve failed you should print "error: cannot execute executable_that_failed" in STDERR followed by a '\n' with executable_that_failed replaced with the path of the failed executable (It should be the first argument of execve)
- Your program should be able to manage more than hundreds of "|" even if we limit the number of "open files" to less than 30.

for example this should work:
$>./microshell /bin/ls "|" /usr/bin/grep microshell ";" /bin/echo i love my microshell
microshell
i love my microshell
$>

Hints:
Don't forget to pass the environment variable to execve

Hints:
Do not leak file descriptors!
```

## 📌 Subject

Recreate a simple shell that can execute commands with basic shell features.

## 🚀 Allowed Functions

- `malloc`, `free`, `write`, `close`, `fork`, `waitpid`, `signal`, `kill`, `exit`, `chdir`, `execve`, `dup`, `dup2`, `pipe`, `strcmp`, `strncmp`

## 🎯 Program Behavior

- The program will execute shell commands based on the arguments provided.
- The executable's path will be **absolute or relative**.  
  **No path resolution using the `PATH` variable**.
- Supports **`;`** (command separator) and **`|`** (pipes) like Bash.
- **Syntax rules**:
  - **No `|` immediately followed or preceded** by nothing, another `|`, or `;`.
  - **`cd` is the only built-in command**, and it:
    - Accepts only a **path** as an argument (no `-` or empty arguments).
    - Prints an error on incorrect usage:
      ```
      error: cd: bad arguments
      ```
    - Prints an error if it fails:
      ```
      error: cd: cannot change directory to path_to_change
      ```
    - **Never appears before or after a `|`**.

## ❌ Features NOT Required

- No need to handle **wildcards** (`*`, `~`, etc.).
- No need to manage **environment variables** (`$VAR`).

## ⚠️ Error Handling

- If a **system call (except `execve` and `chdir`) fails**, print:
  ```
  error: fatal
  ```
  and exit immediately.
- If `execve` fails, print:
  ```
  error: cannot execute executable_that_failed
  ```
  where `executable_that_failed` is the failed program.

## 🔥 Performance

- The program should handle **hundreds of `|` pipes**, even with a file descriptor limit of `< 30`.

## 📌 Example

```sh
$> ./microshell /bin/ls "|" /usr/bin/grep microshell ";" /bin/echo i love my microshell
microshell
i love my microshell
$>
```

Run with:
```sh
./microshell
```
