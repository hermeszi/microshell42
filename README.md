# microshell42

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
