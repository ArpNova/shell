# Simple Shell in C

This is a custom implementation of a Unix shell, written from scratch in C. The project aims to understand the inner workings of a shell, including process creation, execution, and signal handling.

## Features

-   **REPL (Read-Eval-Print Loop)**: The shell runs in a continuous loop, waiting for user input.
-   **Custom Prompt**: Displays the current user, hostname, and working directory.
-   **Input Parsing**: Reads commands from stdin and tokenizes them into arguments.
-   **Command Execution**: Executes commands by forking processes and using `execvp`.
-   **Built-in Commands**:
    -   `cd`: Change directory.
    -   `help`: Display help information.
    -   `exit`: Exit the shell.
    -   `export`: Set environment variables (KEY=VALUE).
    -   `unset`: Unset environment variables.
-   **Input/Output Redirection**:
    -   `>`: Redirect output to a file (overwrite).
    -   `>>`: Redirect output to a file (append).
    -   `<`: Redirect input from a file.
-   **Pipes**: Support for `|` to pipe output of one command to another.
-   **Logical Operators**:
    -   `&&`: Execute the second command only if the first succeeds.
    -   `||`: Execute the second command only if the first fails.
-   **Wildcard Expansion**: Support for `*` and `?` using globbing.
-   **Environment Variables**: 
    -   Support for `$VAR` expansion.
    -   Support for `$?` to get the exit status of the last command.
-   **Background Processes**: Support for running commands in the background using `&`.
-   **Signal Handling**: Graceful handling of `SIGINT` (Ctrl+C).
-   **Zombie Process Reaping**: Automatically cleans up terminated background processes.
-   **Script Execution**: Can execute commands from a file provided as an argument.

## Project Structure

-   `src/main.c`: Contains the complete source code for the shell, including:
    -   Main loop (REPL).
    -   Input reading and parsing.
    -   Command execution logic.
    -   Built-in command implementations.
    -   Signal handling.
    -   Wildcard and environment variable expansion.

## How to Build and Run

To compile the shell, use a C compiler like `gcc`:

```bash
gcc src/main.c -o myshell
```

To run the shell interactively:

```bash
./myshell
```

To run a script file:

```bash
./myshell script.txt
```

## Implementation Details

The shell follows a standard lifecycle:

1.  **Read**: Reads a line of input from the user or file.
2.  **Parse**: Splits the line into tokens.
3.  **Expand**: Expands environment variables and wildcards.
4.  **Execute**:
    -   Checks for built-in commands.
    -   Handles pipes and logical operators.
    -   Forks and executes external commands.
    -   Manages input/output redirection.

## Future Improvements

-   Command history with arrow key support.
-   Tab completion for filenames and commands.
-   More robust error handling.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
