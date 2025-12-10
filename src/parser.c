#include "../include/parser.h"
#include "../include/shell.h"

#define arsh_TOK_BUFSIZE 64
#define arsh_TOK_DELIM " \t\r\n\a"

char **arsh_split_line(char *line) {
  int bufsize = arsh_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "arsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = malloc(strlen(line) + 1);
  if (!token) {
    fprintf(stderr, "arsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  int i = 0;        // index for input 'line'
  int j = 0;        // index for corrent 'token' buffer
  int in_quote = 0; // falg for quotes

  while (line[i] != '\0') {
    char c = line[i];
    if (c == '"') {
      in_quote = !in_quote;
    } else if (strchr(arsh_TOK_DELIM, c) != NULL && !in_quote) {
      // found a delimiter and we are not in quotes
      if (j > 0) {
        token[j] = '\0';
        tokens[position++] = strdup(token);
        j = 0;

        if (position >= bufsize) {
          bufsize += arsh_TOK_BUFSIZE;
          tokens = realloc(tokens, bufsize * sizeof(char *));
          if (!tokens) {
            fprintf(stderr, "arsh: allocation error\n");
            exit(EXIT_FAILURE);
          }
        }
      }
    } else {
      token[j] = c;
      j++;
    }
    i++;
  }

  if (j > 0) {
    token[j] = '\0';
    tokens[position++] = strdup(token);
  }

  free(token);
  tokens[position] = NULL;

  return tokens;
}

char **arsh_expand_wildcards(char **args) {
  int bufsize = 64;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));

  if (!tokens) {
    fprintf(stderr, "arsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; args[i] != NULL; i++) {
    if (strchr(args[i], '*') != NULL || strchr(args[i], '?') != NULL) {
      glob_t glob_result;

      int return_value =
          glob(args[i], GLOB_NOCHECK | GLOB_TILDE, NULL, &glob_result);

      if (return_value == 0) {
        for (size_t j = 0; j < glob_result.gl_pathc; j++) {
          tokens[position++] = strdup(glob_result.gl_pathv[j]);

          if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char *));
          }
        }
      } else {
        tokens[position++] = strdup(args[i]);
      }
      globfree(&glob_result);
    } else {
      tokens[position++] = strdup(args[i]);

      if (position >= bufsize) {
        bufsize += 64;
        tokens = realloc(tokens, bufsize * sizeof(char *));
      }
    }
  }
  tokens[position] = NULL;
  return tokens;
}

char **arsh_expand_env_vars(char **args) {
  int bufsize = 64;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));

  if (!tokens) {
    fprintf(stderr, "arsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; args[i] != NULL; i++) {
    char *arg = args[i];

    //'$' check
    if (arg[0] == '$' && strlen(arg) > 1) {
      if (strcmp(arg, "$?") == 0) {
        char buffer[16];
        snprintf(buffer, 16, "%d", last_exit_status);
        tokens[position++] = strdup(buffer);
      } else {
        char *env_val = getenv(arg + 1);

        if (env_val != NULL) {
          tokens[position++] = strdup(env_val);
        } else {
          tokens[position++] = strdup("");
        }
      }
    } else {
      tokens[position++] = strdup(arg);
    }

    if (position >= bufsize) {
      bufsize += 64;
      tokens = realloc(tokens, bufsize * sizeof(char *));
    }
  }

  tokens[position] = NULL;
  return tokens;
}
