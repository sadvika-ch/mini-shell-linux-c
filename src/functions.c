#include "minishell.h"

extern char prompt[100];
int status;
pid_t c1;
void scan_input(char *prompt, char *input_string){
    while(1){
        //printing the prompt
        printf(GREEN"%s"RESET,prompt);
        fflush(stdout);
        //reading the input commands from user
        input_string[0] = '\0';
        scanf("%[^\n]",input_string);
        getchar();    //to remove the \n from the input string
        // fflush(stdout);
        if ( input_string[0] == '\0')   //if given just \n, again continuing 
        {
            continue;
        }
        //checking PSI is passed or not
        if(strstr(input_string,"PS1")){
            //Checking if spaces are present
            if(strchr(input_string,' ')){
                printf(RED"%s -> Invalid PS1 format\n"RESET,input_string);
            }
            else{
                char *eq = strchr(input_string, '=');   
                if(eq)                          // only copy if = actually exists
                    strcpy(prompt, eq + 1);     //copying from next char to =
                else
                    printf(RED"%s -> Invalid PS1 format\n"RESET,input_string);  //if == not there then printing error
                }
        }
        //if jobs or fg or bg called then calling signal handler function
        if(!strcmp(input_string,"jobs") || !strcmp(input_string,"fg") || !strcmp(input_string,"bg")){
            signal_handler_commands(input_string);
            continue;
        }
        //getting first word
        char * cmd=get_command(input_string);
        //sending it to check command type function to check the type of it
        int type=check_command_type(cmd);
        free(cmd);   //freeing the cmd so we can store next word again
       // printf("%d\n",type);
        if(type==BUILTIN){
            execute_internal_commands(input_string);
        }
        else if(type==EXTERNAL){
            c1=fork();
            if(c1>0){
                //waiting for child
                waitpid(c1,&status,WUNTRACED);
            }
            else if(c1==0){
                //registering to default handler
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                execute_external_commands(input_string); 

            }
        }
        else{
            printf(RED"%s: command not found\n"RESET,input_string);
        }


    }
}

//for external commands
void execute_external_commands(char *input_string){
    char temp[100];
    strcpy(temp, input_string);

    //converting the commands given by user into 2d array
    char *args[20];
    int i = 0,pipe_flag=0;

    //we used strtok and making tokens using space as delimeter
    char *token = strtok(temp, " ");
    while(token != NULL){
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    int argc = i;
    i=0;
    while(args[i]){   //checking if piep is passed or not
        if(strcmp(args[i],"|")==0){
            pipe_flag=1;
            break;
        }
        i++;
    }
    if(pipe_flag)  //if pipe passed calling pipe function
        my_pipe(argc,args);
    else{              //else calling execvp to execute the command
        execvp(args[0], args);   
    } 
}

//for internal commands
void execute_internal_commands(char * input_string){
    //exit
    if(strcmp(input_string,"exit")==0){
        exit(0);  //calling exit lib function
    }
    //pwd
    else if(strcmp(input_string,"pwd")==0){
        char buf[50];
        getcwd(buf,sizeof(buf));   //getting the current working directly path and storing it in a buffer 
        printf(YELLOW"%s\n"RESET,buf);       //then printing it
    }
    //cd
    else if(strstr(input_string,"cd")){
        //printf("%s\n", input_string + 3);
        chdir(input_string + 3);      //changing the directry to the given path
    }
    else if (strstr(input_string,"echo ")){
        //echo $$
        if(strcmp(input_string,"echo $$")==0){
            //prints the pid
            printf(YELLOW"%d\n"RESET,getpid());
        }
        //echo $?
        else if(strcmp(input_string,"echo $?")==0){
            //prints the exit status of previous executed command
            if(WIFEXITED(status))
                printf(YELLOW"Exited normally with status -> %d\n"RESET,WEXITSTATUS(status));
            else 
                printf(YELLOW"Exited abnormally with status -> %d\n"RESET,status); 
        }
        //echo $SHELL
        else if(strcmp(input_string,"echo $SHELL")==0){
            char buf[50];
            getcwd(buf,sizeof(buf)); //getting the current working directly path and storing it in a buffer
            printf(YELLOW"%s\n"RESET,buf); //printing it
        }
        else{       //else echo will act as printf, whatever passed after echo should print in the stdout
            char buf[50];
            char *space = strchr(input_string, ' ');  //printing the content after the first space
            strcpy(buf,space+1);
            printf(BLUE"%s\n"RESET,buf);
        }
    }
    //clear
    else if(strcmp(input_string,"clear")==0){
        system("clear");
    }
}
