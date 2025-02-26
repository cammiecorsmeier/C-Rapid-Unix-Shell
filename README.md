# Rapid Unix Shell (rush)

## Overview
**rush (Rapid Unix SHell)** is a simple, efficient Unix shell that executes user commands by creating child processes. It follows standard shell behavior, including executing commands, managing paths, handling built-in commands, supporting output redirection, and executing parallel commands.

## Features

### 1. Basic Shell Functionality
- The shell runs an interactive loop that:
  - Prints the prompt `rush> `
  - Accepts and parses user input
  - Executes the specified command
  - Waits for the command to complete before accepting the next input
- The shell exits when the user types `exit`.
- Uses `getline()` to read input and `strsep()` to parse commands.
- Utilizes `fork()`, `execv()`, and `wait()/waitpid()` to execute commands.

### 2. Path Management
- The shell maintains a search path for locating executables.
- Initially, the path contains only `/bin`.
- The `path` built-in command modifies the search path.
- Uses `access()` to check for executable files in directories specified by the path.

### 3. Built-in Commands
- **exit**: Terminates the shell immediately.
- **cd**: Changes the working directory. Accepts exactly one argument.
- **path**: Sets the directories to search for executables. If empty, only built-in commands will work.

### 4. Output Redirection
- Supports redirection using `>`.
- Redirects standard output to a specified file.
- Overwrites the file if it already exists.
- Only one redirection per command is allowed.
- Built-in commands do not support redirection.

### 5. Parallel Command Execution
- Supports multiple commands separated by `&`.
- Executes commands in parallel.
- Waits for all commands to complete before displaying the next prompt.

### 6. Error Handling
- A single standard error message is printed for all errors:
  ```c
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
  ```
- Errors include:
  - Invalid syntax
  - Incorrect argument counts for built-in commands
  - Command execution failures
  - Path resolution failures
- The shell ignores empty input lines.

### 7. Whitespace Handling
- Ignores extra spaces and tabs in input.
- Operators (`>` and `&`) must be surrounded by spaces.

## Compilation & Execution
### Compile:
```sh
gcc -o rush rush.c
```
### Run:
```sh
./rush
```

## Usage Examples
### Running a command:
```sh
rush> ls -la
```
### Changing directory:
```sh
rush> cd /home/user
```
### Modifying path:
```sh
rush> path /bin /usr/bin
```
### Output redirection:
```sh
rush> ls -la > output.txt
```
### Running parallel commands:
```sh
rush> ls & echo "Hello" & pwd
```
### Exiting the shell:
```sh
rush> exit
```

## Development Approach
- Implemented basic command execution first.
- Added path handling and built-in commands.
- Implemented output redirection.
- Implemented parallel command execution.
- Tested extensively with different input cases.

## Conclusion
This project provides a functional Unix shell with fundamental features like command execution, path handling, redirection, and parallel processing. It ensures robust error handling and efficient process management.

