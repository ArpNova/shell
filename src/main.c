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
    int background = 0;//0 = foreground, 1 = background
    
    //check for "&"
    int i = 0;
    while(args[i] != NULL){
        i++;
    }
    if(i > 0 && strcmp(args[i-1], "&") == 0){
        background = 1;
        args[i-1] = NULL;
    }

    // ONLY set this for FOREGROUND commands.
    if(!background){
        is_running_command = 1;
    }

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
        is_running_command = 0;
    }
    else
    { // parent process
        if(!background){
            //foreground: wait for the child to finish
            do{
                wpid = waitpid(pid, &status, WUNTRACED);
            }while (!WIFEXITED(status) && !WIFSIGNALED(status));
            
            is_running_command = 0;
        }
        else{
            //background
            printf("[Process Started] PID: %d\n", pid);
        }
    }

    return 1;
}

//pipe
int lsh_launch_pipe(char **args, int pipe_pos){
    int pipefd[2];
    pid_t p1, p2;

    args[pipe_pos] = NULL;
    char **cmd2 = &args[pipe_pos + 1];

    if(pipe(pipefd) < 0){
        perror("lsh: pipe");
        return 1;
    }

    is_running_command = 1;

    //left child
    p1 = fork();
    if(p1 < 0){
        perror("fork");
        return 1;
    }

    if(p1 == 0){
        //child1
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if(execvp(args[0], args) == -1){
            perror("lsh: exec p1");
        }
        exit(EXIT_FAILURE);
    }

    //right child
    p2 = fork();
    if(p2 < 0){
        perror("fork");
        return 1;
    }

    if(p2 == 0){
        //child2
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        if(execvp(cmd2[0], cmd2) == -1){
            perror("lsh: exec p2");
        }
        exit(EXIT_FAILURE);
    }

    //parent
    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);

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

    //scan for pipe
    for(int i = 0; args[i] != NULL; i++){
        if(strcmp(args[i], "|") == 0){
            if(args[i+1] == NULL){
                fprintf(stderr, "lsh: ppipe missing second command\n");
                return 1;
            }
            return lsh_launch_pipe(args, i);
        }
    }

    //check builtins
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
        // Zombie Reaper 
        int zombie_status;
        pid_t zombie_pid;
        while((zombie_pid = waitpid(-1, &zombie_status, WNOHANG)) > 0){
            printf("[Process %d exited]\n", zombie_pid);
        } 

        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while (status);
    
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