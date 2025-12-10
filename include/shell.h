#ifndef SHELL_H
#define SHELL_H

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <fcntl.h>
#include <glob.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#ifndef GLOB_TILDE
/* Some platforms (non-GNU) don't provide GLOB_TILDE; define as no-op flag */
#define GLOB_TILDE 0
#endif

#define HISTORY_MAX 50

extern volatile sig_atomic_t is_running_command;
extern int last_exit_status;
extern char *history[HISTORY_MAX];
extern int history_count;

#endif
