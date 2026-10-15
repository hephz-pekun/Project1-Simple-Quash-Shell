# Quash - Custom UNIX Shell

**Student:** Iyanuoluwa Olanipekun

## Project Overview

Quash is a custom UNIX command shell written in C. The goal of this project was to gain hands-on experience with process creation, process management, command execution, signal handling, environment variables, and I/O redirection.

Quash provides functionality similar to standard UNIX shells such as Bash by accepting commands from the user, executing built-in commands directly, and spawning child processes to execute external programs.

---

## Features Implemented

### Current Working Directory Prompt

The shell dynamically displays the current working directory as the command prompt.

Example:

```bash
/home/codio/workspace>
```

This functionality is implemented using the `getcwd()` system call.

---

## Built-In Commands

### pwd

Prints the current working directory.

Example:

```bash
pwd
```

Output:

```bash
/home/codio/workspace
```

---

### cd

Changes the current working directory.

Examples:

```bash
cd ..
cd /home/codio
cd $HOME
```

This functionality is implemented using `chdir()`.

---

### echo

Prints text and expanded environment variables.

Examples:

```bash
echo hello world
echo $HOME
echo $greeting
```

---

### env

Displays environment variables.

Examples:

```bash
env
env HOME
env greeting
```

If a variable name is provided, the shell displays the value of that variable. If no argument is provided, all environment variables are displayed.

---

### setenv

Creates or updates environment variables.

Supported formats:

```bash
setenv greeting hello
setenv greeting=hello
```

Example:

```bash
setenv greeting hello
echo $greeting
```

Output:

```bash
hello
```

---

### exit

Terminates the shell.

Example:

```bash
exit
```

---

## Environment Variable Expansion

The shell supports environment variable substitution using the `$` prefix.

Examples:

```bash
echo $HOME
echo $PATH
echo $greeting
```

Variable expansion is performed during tokenization using the `getenv()` function before command execution.

---

## Process Execution

Commands that are not recognized as built-in commands are executed using child processes.

The shell performs the following steps:

1. Creates a child process using `fork()`.
2. Executes the target program using `execvp()`.
3. Waits for the child process to finish using `waitpid()`.

Examples:

```bash
ls
ls -l
cat output.txt
sleep 5
```

---

## Background Processes

The shell supports background execution using the `&` operator.

Example:

```bash
sleep 20 &
```

Output:

```bash
[background pid 240]
```

The shell immediately returns to the prompt without waiting for the process to complete.

---

## Signal Handling

The shell handles `SIGINT` (Ctrl+C) to prevent accidental termination.

Example:

```bash
sleep 20
^C
/home/codio/workspace>
```

The shell remains running while the foreground process receives the interrupt signal.

This feature is implemented using:

```c
signal(SIGINT, sigint_handler);
```

---

## Process Timeout

Foreground processes are automatically terminated if they run longer than ten seconds.

Example:

```bash
sleep 20
```

Output:

```bash
Process exceeded 10 seconds. Killing process.
```

This functionality uses:

```c
waitpid()
kill()
SIGKILL
```

---

## Output Redirection

The shell supports output redirection using `>`.

Examples:

```bash
ls > output.txt
ls >output.txt
```

The output of the command is written to the specified file.

Implementation uses:

```c
open()
dup2()
```

---

## Input Redirection

The shell supports input redirection using `<`.

Examples:

```bash
cat < testData
cat <testData
```

Input is redirected from a file into the program's standard input.

Implementation uses:

```c
open()
dup2()
```

---

## Design Decisions

The shell was implemented using a command-processing loop that continuously:

1. Displays the current working directory prompt.
2. Reads user input.
3. Tokenizes the command line.
4. Expands environment variables.
5. Processes built-in commands.
6. Handles redirection and background execution.
7. Creates child processes for external programs.
8. Waits for foreground processes to terminate.

This design keeps the code organized and makes it easy to extend with additional shell functionality.

---

## System Calls and Library Functions Used

```c
getcwd()
chdir()
getenv()
setenv()
fork()
execvp()
waitpid()
signal()
kill()
open()
dup2()
strtok()
```

These functions provide directory management, environment variable support, process creation, signal handling, process control, and I/O redirection.

---

## Testing

The following commands were used to test shell functionality:

```bash
pwd

cd ..
pwd

echo hello world

setenv greeting hello
echo $greeting

env greeting

ls
ls -l

sleep 5

sleep 20

sleep 20 &

ls > output.txt
cat output.txt

cat < testData

exit
```

All required functionality was verified through manual testing in the Codio terminal.

---

## Known Limitations

The shell currently does not support pipeline operations.

Example not implemented:

```bash
cat testData | grep this
```

Pipeline functionality using `|` would require additional implementation using the `pipe()` system call and multiple child processes.

---

## Conclusion

This project provided practical experience with UNIX operating system concepts including process creation, process synchronization, signal handling, environment variable management, command execution, and file descriptor redirection.

Quash successfully implements all required project features, including command execution, built-in commands, background processing, signal handling, process timeouts, input redirection, and output redirection.
