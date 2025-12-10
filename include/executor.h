#ifndef EXECUTOR_H
#define EXECUTOR_H

int arsh_launch(char **args);
int arsh_execute(char **args);
int arsh_launch_pipeline(char **args);
int arsh_launch_pipe(char **args, int pipe_pos);
int arsh_logic_split(char **args);

#endif
