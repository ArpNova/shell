#include "../include/executor.h"
#include "../include/input.h"
#include "../include/parser.h"
#include "../include/shell.h"

// Global variables definition
volatile sig_atomic_t is_running_command = 0;
int last_exit_status = 0;
char *history[HISTORY_MAX];
int history_count = 0;

// signal handler
void sigint_handler(int signo) {
  (void)signo; // unused
  if (!is_running_command) {
    printf("\n");
    arsh_print_prompt();
    fflush(stdout);
  } else {
    printf("\n");
  }
}

void arsh_loop(FILE *stream) {
  char *line;
  char **args;
  int status;

  do {
    // Zombie Reaper
    int zombie_status;
    pid_t zombie_pid;
    while ((zombie_pid = waitpid(-1, &zombie_status, WNOHANG)) > 0) {
      printf("[Process %d exited]\n", zombie_pid);
    }

    if (stream == stdin) {
      arsh_print_prompt();
    }

    line = arsh_read_line(stream);
    if (line == NULL) {
      printf("\n");
      exit(EXIT_SUCCESS);
    }

    args = arsh_split_line(line);
    status = arsh_execute(args);

    free(line);

    int i = 0;
    if (args != NULL) {
      while (args[i] != NULL) {
        free(args[i]);
        i++;
      }
      free(args);
    }

  } while (status);
}

int main(int argc, char **argv) {
  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("arsh: signal");
  }

  if (argc == 1) {
    arsh_loop(stdin);
  } else if (argc == 2) {
    FILE *f = fopen(argv[1], "r");
    if (!f) {
      perror("arsh");
      return EXIT_FAILURE;
    }
    arsh_loop(f);
    fclose(f);
  } else {
    fprintf(stderr, "Usage: %s [script_file]\n", argv[0]);
  }

  return EXIT_SUCCESS;
}
