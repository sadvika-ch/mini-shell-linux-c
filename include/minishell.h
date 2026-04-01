#ifndef MAIN_H
#define MAIN_H

/*including header files*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
 
/*defining macros*/
#define BUILTIN		1
#define EXTERNAL	2
#define NO_COMMAND  3

/* ANSI Color Macros */
#define RESET   "\033[0m"

#define ORANGE "\033[38;5;208m"
#define BLACK   "\033[1;30m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"


/*function prototypes*/
void scan_input(char *prompt, char *input_string);
char *get_command(char *input_string);
void signal_handler_commands(char *input_string);
int check_command_type(char *command);
void execute_internal_commands(char *input_string);
void execute_external_commands(char *input_string);
void signal_handler(int signum);
void extract_external_commands(char **external_commands);
int my_pipe(int argc , char * argv[]);

#endif