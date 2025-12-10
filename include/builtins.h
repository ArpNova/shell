#ifndef BUILTINS_H
#define BUILTINS_H

int arsh_cd(char **args);
int arsh_help(char **args);
int arsh_exit(char **args);
int arsh_export(char **args);
int arsh_unset(char **args);
int arsh_num_biultins();

extern char *builtin_str[];
extern int (*builtin_func[])(char **);

#endif
