#include "../include/input.h"
#include "../include/shell.h"

struct termios orig_termios;


void disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void add_to_history(char *line) {
  if (line[0] == '\0')
    return;

  if (history_count > 0 && strcmp(history[history_count - 1], line) == 0)
    return;

  if (history_count < HISTORY_MAX) {
    history[history_count++] = strdup(line);
  } else {
    free(history[0]);
    for (int i = 1; i < HISTORY_MAX; i++) {
      history[i - 1] = history[i];
    }
    history[HISTORY_MAX - 1] = strdup(line);
  }
}

#define arsh_RL_BUFSIZE 1024
char *arsh_read_line(FILE *stream) {
  if (stream != stdin) {
    char *line = NULL;
    size_t bufsize = 0;
    if (getline(&line, &bufsize, stream) == -1)
      return NULL;
    line[strcspn(line, "\n")] = 0;
    return line;
  }

  enableRawMode();

  int bufsize = arsh_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  buffer[0] = '\0';
  int history_index = history_count;

  while (1) {
    char c;
    if (read(STDIN_FILENO, &c, 1) <= 0)
      break; // EOF

    // Ctrl+D (EOT)
    if (c == 4) {
      if (strlen(buffer) == 0) {
        free(buffer);
        disableRawMode();
        return NULL;
      } else {
        continue;
      }
    }

    // esc sequence
    if (c == '\x1b') {
      char seq[3];
      if (read(STDIN_FILENO, &seq[0], 1) == 0)
        continue;
      if (read(STDIN_FILENO, &seq[1], 1) == 0)
        continue;

      if (seq[0] == '[') {
        // arrow keys
        if (seq[1] == 'A') {
          // UP
          if (history_index > 0) {
            history_index--;

            while (position > 0) {
              printf("\b \b");
              fflush(stdout);
              position--;
            }

            strcpy(buffer, history[history_index]);
            position = strlen(buffer);
            printf("%s", buffer);
            fflush(stdout);
          }
        } else if (seq[1] == 'B') {
          // DOWN
          if (history_index < history_count) {
            history_index++;

            while (position > 0) {
              printf("\b \b");
              fflush(stdout);
              position--;
            }

            if (history_index < history_count) {
              strcpy(buffer, history[history_index]);
              position = strlen(buffer);
              printf("%s", buffer);
              fflush(stdout);
            } else {
              buffer[0] = '\0';
              position = 0;
            }
          }
        } else if (seq[1] == 'C') {
          // RIGHT
          if (position < (int)strlen(buffer)) {
            printf("\033[C");
            fflush(stdout);
            position++;
          }
        } else if (seq[1] == 'D') {
          // LEFT
          if (position > 0) {
            printf("\033[D");
            fflush(stdout);
            position--;
          }
        }
        // HOME KEY (Standard: \x1b[H or \x1b[1~)
        else if (seq[1] == 'H' || seq[1] == '1') {
          while (position > 0) {
            printf("\b");
            fflush(stdout);
            position--;
          }
        }
        // END KEY (Standard: \x1b[F or \x1b[4~)
        else if (seq[1] == 'F' || seq[1] == '4') {
          int len = strlen(buffer);
          while (position < len) {
            printf("\033[C");
            fflush(stdout);
            position++;
          }
        }
      }
      // Some terminals use 'O' for Home/End (e.g. \x1bOH)
      else if (seq[0] == 'O') {
        if (seq[1] == 'H') {
          while (position > 0) {
            printf("\b");
            fflush(stdout);
            position--;
          }
        } else if (seq[1] == 'F') {
          int len = strlen(buffer);
          while (position < len) {
            printf("\033[C");
            fflush(stdout);
            position++;
          }
        }
      }
      continue;
    }

    // normal keys
    if (c == '\n') {
      buffer[strlen(buffer)] = '\0';
      printf("\n");
      fflush(stdout);
      disableRawMode();
      add_to_history(buffer);
      return buffer;
    } else if (c == 127) {
      if (position > 0) {
        int len = strlen(buffer);
        if (position == len) {
          position--;
          buffer[position] = '\0';
          printf("\b \b");
          fflush(stdout);
        } else {
          memmove(&buffer[position - 1], &buffer[position], len - position + 1);
          position--;

          // redraw
          printf("\b");
          printf("%s", &buffer[position]);
          printf(" ");
          fflush(stdout);

          for (int k = position; k < len; k++) {
            printf("\b");
            fflush(stdout);
          }
        }
      }
    } else if (c == '\t') {
      // tabs(future feature)
    } else if (isprint(c)) {
      // regular char
      int len = strlen(buffer);
      if (position == len) {
        buffer[position] = c;
        buffer[position + 1] = '\0';
        position++;
        printf("%c", c);
        fflush(stdout);
      } else {
        memmove(&buffer[position + 1], &buffer[position], len - position + 1);
        buffer[position] = c;
        printf("%s", &buffer[position]);
        fflush(stdout);
        position++;
        for (int k = position; k <= len; k++) {
          printf("\b");
          fflush(stdout);
        }
      }
    }
  }

  disableRawMode();
  free(buffer); // Free buffer on EOF or error
  return NULL;
}

void arsh_print_prompt() {
  char hostname[1024];
  gethostname(hostname, 1024);

  char *username = getenv("USER");

  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m$ ",
           username ? username : "user", hostname, cwd);
  } else {
    printf("> ");
  }

  fflush(stdout);
}
