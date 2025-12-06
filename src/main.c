#define _POSIX_C_SOURCE 200809L

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

volatile sig_atomic_t is_running_command = 0;

// signal handler
void sigint_handler(int signo)
{
    if (!is_running_command)
    {
        printf("\n> ");
        fflush(stdout);
    }
    else
    {
        printf("\n");
    }
}

/*
    function declarations for builtin shell commands
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
    list of builtin commands
*/
char *builtin_str[] = {
    "cd",
    "help",
    "exit"};

int (*builtin_func[])(char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit};

int lsh_num_biultins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/*
    Builtin function implementations
*/
int lsh_cd(char **args)
{
    if (args[1] == NULL)
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args)
{
    printf("Arpan Biswas's SHELL\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < lsh_num_biultins(); i++)
    {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char **args)
{
    return 0;
}

int lsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    // trun the flag on before forking
    is_running_command = 1;

    pid = fork();
    if (pid == 0)
    {
        // child process
        // scan for redirection
        for (int i = 0; args[i] != NULL; i++)
        {
            // output redirection
            if (strcmp(args[i], ">") == 0)
            {
                if (args[i + 1] == NULL)
                {
                    fprintf(stderr, "lsh: expected argument to \">\"\n");
                    exit(EXIT_FAILURE);
                }

                char *filename = args[i + 1];

                int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fd == -1)
                {
                    perror("lsh: open");
                    exit(EXIT_FAILURE);
                }

                // redirection
                if (dup2(fd, STDOUT_FILENO) == -1)
                {
                    perror("lsh: dup2");
                    exit(EXIT_FAILURE);
                }

                close(fd);

                // remove ">" and filename from command
                args[i] = NULL;
                continue;
            }

            // input redirection
            if (strcmp(args[i], "<") == 0)
            {
                if (args[i + 1] == NULL)
                {
                    fprintf(stderr, "lsh: expected argement to \">\"\n");
                    exit(EXIT_FAILURE);
                }

                char *filename = args[i + 1];

                int fd = open(filename, O_RDONLY);

                if (fd == -1)
                {
                    perror("lsh: open input file");
                    exit(EXIT_FAILURE);
                }

                if (dup2(fd, STDIN_FILENO) == -1)
                {
                    perror("lsh: dup2 input");
                    exit(EXIT_FAILURE);
                }

                close(fd);

                args[i] = NULL;
                continue;
            }
        }

        if (execvp(args[0], args) == -1)
            perror("lsh");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("lsh");
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    // turn the flag off after the child is done
    is_running_command = 0;

    return 1;
}

int lsh_execute(char **args)
{
    if (args[0] == NULL)
    {
        // An empty command was entered
        return 1;
    }

    for (int i = 0; i < lsh_num_biultins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;
        if (position >= bufsize)
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void)
{
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();

        // end of file check
        if (c == EOF)
            exit(EXIT_SUCCESS);

        if (c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;
        if (position >= bufsize)
        {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

    } while (status);
    free(line);
    free(args);
}

int main(int argc, char **argv)
{

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("lsh: signal");
    }

    lsh_loop();

    return EXIT_SUCCESS;
}