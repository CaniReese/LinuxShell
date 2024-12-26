//G221210092 ASIM BINGOL - https://github.com/ASIMBINGOL
//Y245060003 MESUTCAN OZKAN - https://github.com/CaniReese
//B231210061 METEHAN OZTURK - https://github.com/metepukkada
//G221210008 YUSUF TAHA EZGIN - https://github.com/yusuftahaezgin
//G221210014 AHMET SEVLI - https://github.com/Ahmet-Sevli1

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

void sigchld_handler(int signo);
void display_prompt();
int parse_command(char *input, char **args);
void handle_redirection(char **args);
void handle_pipe(char *input);
void handle_semicolon(char *input);
void handle_background_commands(char *input);
void execute_command(char **args, int background);

#endif
