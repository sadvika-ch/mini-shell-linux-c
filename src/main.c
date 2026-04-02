#include "minishell.h"

//2d array to store external commands
char * ext_cmd[200];
//to print our prompt
char prompt[100]="MiniShell:$ ";
//to scan input
char input_string[50];
int main(){
    //registering the handlers so controller goes to our own handler instead of default handler
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);
    //clear the screen
    system("clear");
    printf(ORANGE "=================================\n" RESET);
    printf(ORANGE "      WELCOME TO MINI SHELL      \n" RESET);
    printf(ORANGE "=================================\n" RESET);
    //converting the external commands from file to 2d array
    extract_external_commands(ext_cmd);
    //calling scan input function to scan and do the rest of the process
    scan_input(prompt,input_string);
}
/*this function will act like get word, it will return the pointer to the first word*/
char * get_command(char * input_string){
    //aalocating memory dynamically as we are accessing it from other funcions also
    char * input = malloc(strlen(input_string) + 1);
    //copying input string to input
    strcpy(input, input_string);
    
    // find space and cut there
    char *space = strchr(input, ' ');
    if(space)
        *space = '\0';
    //returning the word upto first space
    return input;
}

/*It will check the command type , whether it is external , internal or nothing*/
int check_command_type(char * cmd){
    //storing buildin commands
    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "clear" , "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};
    int i=0;
    
    //checking if the command matches with the builtin commands
    while(builtins[i]){
        if(strcmp(cmd,builtins[i])==0)
        //if yes returning the macro
            return BUILTIN;
        i++;    
    }
    //checking if the command matches with the external commands
    i=0;
    while(ext_cmd[i]){
        if(strcmp(cmd,ext_cmd[i])==0)
        //if yes returning the macro
            return EXTERNAL;
        i++;    
    }
    //else returning NO_COMMAND macro
    return NO_COMMAND;
}
