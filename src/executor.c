#include "../include/executor.h"
#include "../include/builtins.h"
#include "../include/parser.h"
#include "../include/shell.h"

int arsh_launch(char **args) {
  pid_t pid;
  int status;
  int background = 0; // 0 = foreground, 1 = background

  // check for "&"
  int i = 0;
  while (args[i] != NULL) {
    i++;
  }
  if (i > 0 && strcmp(args[i - 1], "&") == 0) {
    background = 1;
    args[i - 1] = NULL;
  }

  // ONLY set this for FOREGROUND commands.
  if (!background) {
    is_running_command = 1;
  }

  pid = fork();
  if (pid == 0) {
    // child process
    // scan for redirection
    for (int i = 0; args[i] != NULL; i++) {
      // append redirection
      if (strcmp(args[i], ">>") == 0) {
        if (args[i + 1] == NULL) {
          fprintf(stderr, "arsh: expected argument to \">>\"\n");
          exit(EXIT_FAILURE);
        }

        char *filename = args[i + 1];
        int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);

        if (fd == -1) {
          perror("arsh: open append");
          exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) == -1) {
          perror("arsh: dup2 append");
          exit(EXIT_FAILURE);
        }
        close(fd);

        args[i] = NULL;
        continue;
      }

      // output redirection
      if (strcmp(args[i], ">") == 0) {
        if (args[i + 1] == NULL) {
          fprintf(stderr, "arsh: expected argument to \">\"\n");
          exit(EXIT_FAILURE);
        }

        char *filename = args[i + 1];

        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd == -1) {
          perror("arsh: open");
          exit(EXIT_FAILURE);
        }

        // redirection
        if (dup2(fd, STDOUT_FILENO) == -1) {
          perror("arsh: dup2");
          exit(EXIT_FAILURE);
        }

        close(fd);

        // remove ">" and filename from command
        args[i] = NULL;
        continue;
      }

      // input redirection
      if (strcmp(args[i], "<") == 0) {
        if (args[i + 1] == NULL) {
          fprintf(stderr, "arsh: expected argement to \">\"\n");
          exit(EXIT_FAILURE);
        }

        char *filename = args[i + 1];

        int fd = open(filename, O_RDONLY);

        if (fd == -1) {
          perror("arsh: open input file");
          exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDIN_FILENO) == -1) {
          perror("arsh: dup2 input");
          exit(EXIT_FAILURE);
        }

        close(fd);

        args[i] = NULL;
        continue;
      }
    }

    if (execvp(args[0], args) == -1)
      perror("arsh");
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("arsh");
    is_running_command = 0;
  } else { // parent process
    if (!background) {
      // foreground: wait for the child to finish
      do {
        waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));

      if (WIFEXITED(status)) {
        last_exit_status = WEXITSTATUS(status);
      }
      is_running_command = 0;
    } else {
      // background
      printf("[Process Started] PID: %d\n", pid);
    }
  }

  return 1;
}

// handle && and || logic
int arsh_logic_split(char **args) {
  int split_idx = -1;
  int type = 0; // 1 for &&, 2 for ||

  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "&&") == 0) {
      split_idx = i;
      type = 1;
      break;
    }

    else if (strcmp(args[i], "||") == 0) {
      split_idx = i;
      type = 2;
      break;
    }
  }

  if (split_idx == -1) {
    return arsh_launch_pipeline(args);
  }

  args[split_idx] = NULL;
  char **cmd2 = &args[split_idx + 1];

  int loop_status = arsh_logic_split(args);

  if (type == 1) {
    if (last_exit_status == 0) {
      return arsh_logic_split(cmd2);
    }
  } else if (type == 2) {
    if (last_exit_status != 0) {
      return arsh_logic_split(cmd2);
    }
  }

  return loop_status;
}

// pipe
int arsh_launch_pipe(char **args, int pipe_pos) {
  int pipefd[2];
  pid_t p1, p2;

  args[pipe_pos] = NULL;
  char **cmd2 = &args[pipe_pos + 1];

  if (pipe(pipefd) < 0) {
    perror("arsh: pipe");
    return 1;
  }

  is_running_command = 1;

  // left child
  p1 = fork();
  if (p1 < 0) {
    perror("fork");
    return 1;
  }

  if (p1 == 0) {
    // child1
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    if (execvp(args[0], args) == -1) {
      perror("arsh: exec p1");
    }
    exit(EXIT_FAILURE);
  }

  // right child
  p2 = fork();
  if (p2 < 0) {
    perror("fork");
    return 1;
  }

  if (p2 == 0) {
    // child2
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);

    if (execvp(cmd2[0], cmd2) == -1) {
      perror("arsh: exec p2");
    }
    exit(EXIT_FAILURE);
  }

  // parent
  close(pipefd[0]);
  close(pipefd[1]);

  waitpid(p1, NULL, 0);
  waitpid(p2, NULL, 0);

  is_running_command = 0;
  return 1;
}

int arsh_launch_pipeline(char **args) {
  if (args[0] == NULL)
    return 1;

  // scan pipe
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      if (args[i + 1] == NULL) {
        fprintf(stderr, "arsh: pipe missing second command\n");
        return 1;
      }
      return arsh_launch_pipe(args, i);
    }
  }

  // check builtins
  for (int i = 0; i < arsh_num_biultins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  // normal launch
  return arsh_launch(args);
}

int arsh_execute(char **args) {
  if (args[0] == NULL) {
    // An empty command was entered
    return 1;
  }

  // expand env
  char **env_args = arsh_expand_env_vars(args);

  // expand wildcards
  char **expanded_args = arsh_expand_wildcards(env_args);

  // cleanup snapshot
  int env_count = 0;
  while (env_args[env_count] != NULL) {
    free(env_args[env_count]);
    env_count++;
  }
  free(env_args);

  int count = 0;
  while (expanded_args[count] != NULL)
    count++;

  char **cleanup_list = malloc((count + 1) * sizeof(char *));
  for (int i = 0; i < count; i++) {
    cleanup_list[i] = expanded_args[i];
  }
  cleanup_list[count] = NULL;

  int status = arsh_logic_split(expanded_args);

  // clean-up
  for (int i = 0; i < count; i++) {
    free(cleanup_list[i]);
  }

  free(cleanup_list);
  free(expanded_args);

  return status;
}
