#include "../include/builtins.h"
#include "../include/shell.h"

char *builtin_str[] = {"cd", "help", "exit", "export", "unset"};

int (*builtin_func[])(char **) = {&arsh_cd, &arsh_help, &arsh_exit,
                                  &arsh_export, &arsh_unset};

int arsh_num_biultins() { return sizeof(builtin_str) / sizeof(char *); }

int arsh_cd(char **args) {
  if (args[1] == NULL)
    fprintf(stderr, "arsh: expected argument to \"cd\"\n");
  else {
    if (chdir(args[1]) != 0) {
      perror("arsh");
    }
  }
  return 1;
}

int arsh_help(char **args) {
  (void)args;
  printf("Welcome to arsh\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (int i = 0; i < arsh_num_biultins(); i++) {
    printf(" %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int arsh_exit(char **args) {
  (void)args;
  return 0;
}

int arsh_export(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "arsh: expected argument to \"export\"\n");
  }

  // split KEY=VALUE string
  char *arg = args[1];
  char *equal_sign = strchr(arg, '=');

  if (equal_sign == NULL) {
    fprintf(stderr, "arsh: invalid format (use KEY=VALUE)\n");
    return 1;
  }
  // terminate key string at the '='
  *equal_sign = '\0';

  char *key = arg;
  char *value = equal_sign + 1;

  if (setenv(key, value, 1) != 0) {
    perror("arsh");
  }
  return 1;
}

int arsh_unset(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "arsh: expected argument to \"unset\"\n");
    return 1;
  }

  if (unsetenv(args[1]) != 0) {
    perror("arsh");
  }

  return 1;
}
