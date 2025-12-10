# arsh - A Custom Unix Shell in C

`arsh` is a robust, custom implementation of a Unix shell, written from scratch in C. This project demonstrates a deep understanding of system programming concepts, including process management, signal handling, and inter-process communication.

## Features

-   **Interactive REPL**: A continuous Read-Eval-Print Loop that accepts and executes user commands.
-   **Custom Prompt**: informative prompt displaying user, hostname, and current working directory.
-   **Command Execution**: Seamless execution of external programs using `fork` and `execvp`.
-   **Built-in Commands**:
    -   `cd`: Change the current working directory.
    -   `help`: Display information about the shell.
    -   `exit`: Terminate the shell session.
    -   `export`: Set environment variables (e.g., `KEY=VALUE`).
    -   `unset`: Remove environment variables.
-   **I/O Redirection**:
    -   `>`: Redirect standard output to a file (overwrite).
    -   `>>`: Redirect standard output to a file (append).
    -   `<`: Redirect standard input from a file.
-   **Piping**: Chain commands using `|` to pass output from one process as input to another.
-   **Logical Operators**:
    -   `&&`: Execute the following command only if the previous one succeeds.
    -   `||`: Execute the following command only if the previous one fails.
-   **Wildcard Expansion**: Globbing support for `*` and `?` patterns.
-   **Environment Variables**:
    -   Expand variables using `$VAR`.
    -   Access exit status of the last command with `$?`.
-   **Job Control**:
    -   Run commands in the background with `&`.
    -   Automatic reaping of zombie processes.
-   **Signal Handling**: Graceful handling of signals like `SIGINT` (Ctrl+C).
-   **Script Execution**: Ability to run commands from a script file provided as an argument.
-   **Line Editing & History**:
    -   Navigate command history with Up/Down arrow keys.
    -   Edit the current line using Left/Right arrows, Home, End, and Backspace.

## Project Structure

The codebase is organized for modularity and maintainability:

```
.
├── include/        # Header files defining interfaces
│   ├── builtins.h
│   ├── executor.h
│   ├── input.h
│   ├── parser.h
│   └── shell.h
├── src/            # Source code implementations
│   ├── builtins.c  # Built-in command logic
│   ├── executor.c  # Process creation and execution
│   ├── input.c     # Input reading and history management
│   ├── main.c      # Entry point and main loop
│   └── parser.c    # Command parsing and tokenization
├── Makefile        # Build configuration
└── README.md       # Project documentation
```

## Getting Started

### Prerequisites

-   GCC (GNU Compiler Collection)
-   Make

### Build

To compile the project, simply run `make` in the root directory. This will compile the source files and generate the `arsh` executable.

```bash
make
```

To clean up build artifacts (object files and the executable):

```bash
make clean
```

### Run

Start the shell interactively:

```bash
./arsh
```

Execute a script file:

```bash
./arsh script.txt
```

## Implementation Details

The shell operates through a structured lifecycle:

1.  **Initialization**: Sets up signal handlers and environment.
2.  **Read**: Captures user input or reads from a file.
3.  **Parse**: Tokenizes the input, handling quotes and special characters.
4.  **Expand**: Processes environment variables and wildcard patterns.
5.  **Execute**:
    -   Identifies and runs built-in commands directly.
    -   Manages pipelines and redirections.
    -   Forks child processes for external commands.
    -   Waits for foreground processes to complete.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
