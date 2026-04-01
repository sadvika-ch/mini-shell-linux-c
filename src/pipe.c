#include "minishell.h"

/*execute the commands with pipes (multiple commands)*/
int my_pipe(int argc , char * argv[]){
    //Validating arguments
    if (argc < 3){
        printf(RED"Usage: %s command [args]\n"RESET, argv[0]);
        return 1;
    }
    //array to store the command indexes
    int arr[argc];
    int j=0,count=1;
    //cmd index starting from 0
    arr[j]=0;
    j++;
    //getting command indexes
    for(int i=0;i<argc;i++){
        if(strcmp(argv[i],"|")==0){
            count++;
            arr[j]=i+1;
            j++;
            argv[i]=NULL;
        }
    }

    //creating the child process 
    int restore = dup(0);   // save original stdin BEFORE loop
    for(int i=0;i<count;i++){
        int fd[2];
        //create the pipe
        if(i<count-1){
            if(pipe(fd)==-1){
                perror(RED"pipe"RESET);
                return -1;
            }
        }
        pid_t c1=fork();
        if(c1>0){      //parent
            if(i < count-1){
                close(fd[1]);
                dup2(fd[0], 0);
                close(fd[0]);
            }
     
        }
        if(c1==0){   //child
            if(i < count-1){
                close(fd[0]);
                dup2(fd[1], 1);
                close(fd[1]);
            }
            //executing the commands
            execvp(argv[arr[i]],argv+arr[i]);
        }
    }
    // wait for all children
    dup2(restore,0);  //restoring the stdin so we can give the next command
    close(restore);
    for (int i = 0; i < count; i++)
        wait(NULL);
    return 0;
}