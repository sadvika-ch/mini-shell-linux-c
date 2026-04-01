#include "minishell.h"

/*to store all the external commands in 2d array*/
void extract_external_commands(char **external_commands){
    //opening the file using open system call in read only mode
    int fd=open("external.txt",O_RDONLY);
    int i=0,count=0;
    char ch;
    char buf[50];
    while((read(fd,&ch,1))!=0){
        //if newline comes storing to buf
        if(ch=='\n'){
            //handling the \r in the file (sometime the newline can be \r\n and sometime it can be just \n)
            if(i > 0 && buf[i-1] == '\r')
            i--;
            buf[i]='\0';
            //allocating memory dynamically
            external_commands[count] = malloc(50);
            //copying every word till newline in the particular index and increasing the index of the 2d array
            strcpy(external_commands[count],buf);
            count++;
            i=0;
        }
        else{
            //storing the characters into buf till newline
            buf[i]=ch;
            i++;
        }
    }
    //this is for the last command, at last we dont have any newline
    if(i > 0){
        buf[i] = '\0';
        external_commands[count] = malloc(strlen(buf) + 1);
        strcpy(external_commands[count], buf);
        count++;
    }
    //storing null at the last index of 2d array
    external_commands[count]=NULL;
    //closing the file using close()
    close(fd);

}