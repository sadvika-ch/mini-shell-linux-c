#include "minishell.h"

int job_counter;        // global counter to assign unique job numbers [1],[2],...
#define RUNNING 1       // job status : running in background
#define STOPPED 0       // job status : stopped (Ctrl+Z)

// linked list node to store each stopped/running background job
typedef struct node
{
    int job_num;        // job number shown in [1],[2],... format
    int status;         // RUNNING or STOPPED
    pid_t pid;          // process id of the job
    char *input;        // command string eg: "sleep 20"
    struct node *link;  // pointer to next node
} Slist;

Slist *head = NULL;     // head of the jobs linked list

// function declarations
void insert_last(Slist **hd, int pid);
void delete_last(Slist **hd);
void print_list(Slist *hd);

// globals defined in other files
extern char input_string[50];   // current command typed by user
extern char prompt[100];        // shell prompt string eg: "MiniShell:$"
extern pid_t c1;                // pid of current foreground child process
extern int status;              // exit/stop status of child process

/* SIGNAL HANDLER handles SIGINT (Ctrl+C), SIGTSTP (Ctrl+Z), SIGCHLD (child done)   */

void signal_handler(int signum){
    if (signum == SIGINT){
        // Ctrl+C pressed
        // if no command is running, just reprint the prompt
        if (input_string[0] == '\0'){
            printf(GREEN"\n%s"RESET, prompt);
            fflush(stdout);
        }
        // if a child is running, default handler kills it (child resets to SIG_DFL)
    }
    else if (signum == SIGTSTP){
        // Ctrl+Z pressed
        if (input_string[0] == '\0'){
            // no child running, just reprint prompt
            printf(GREEN"\n%s"RESET, prompt);
            fflush(stdout);
        }
        /*It walks the linked list checking if c1 (foreground pid) already exists
        If found → just updates status = STOPPED (this happens when fg brought a job back and you Ctrl+Z it again — same pid, already in list)
        If not found → calls insert_last() which adds it as a new STOPPED job and assigns a job number like [1], [2]*/
        else{
            // child is running — stop it and add to jobs list
            int already_exists = 0;
            Slist *tmp = head;

            // check if this pid is already in the list
            // (eg: fg brought it back and Ctrl+Z pressed again)
            while (tmp){
                if (tmp->pid == c1){
                    // pid already exists, just update status to STOPPED
                    already_exists = 1;
                    tmp->status = STOPPED;
                    printf(YELLOW"[%d]   Stopped                 %s\n"RESET, tmp->job_num, input_string);
                    break;
                }
                tmp = tmp->link;
            }

            // if pid not already in list, insert as new job
            if (!already_exists){
                insert_last(&head, c1);
                // job_counter holds the new job's number after insert_last
                printf(YELLOW"[%d]+  Stopped                 %s\n"RESET, job_counter, input_string);
            }
        }
    }
    /*SIGCHLD fires when a background child finishes.
     We call waitpid(-1, WNOHANG) in a while loop — -1 to reap any child, WNOHANG to not block, and loop because multiple 
     children could finish at once. We only handle bg jobs here because fg jobs are already waited on directly
      via waitpid in the fg command, so the shell knows when they finish. For bg jobs, shell has no idea when they finish, 
      so SIGCHLD notifies us — we walk the list to find the finished pid, print Done, and delete it from the list.*/
    else if (signum == SIGCHLD){
        // a child process finished
        pid_t done;
        // loop to reap ALL finished children (not just one)
        // using -1 to catch any child, WNOHANG to not block
        while ((done = waitpid(-1, &status, WNOHANG)) > 0){
            // find the finished job in the list to print its details
            Slist *tmp = head;
            while (tmp){
                if (tmp->pid == done){
                    // print Done message and reprint prompt
                    printf(BLUE"\n[%d]   Done   %s\n"RESET, tmp->job_num, tmp->input);
                    printf(GREEN"%s"RESET, prompt);
                    fflush(stdout);
                    break;
                }
                tmp = tmp->link;
            }
            // remove the finished job from the list
            delete_last(&head);
        }
    }
}

/*  BUILTIN COMMANDS: fg, bg, jobs                                      */
void signal_handler_commands(char *input_string){
    if (strcmp(input_string, "fg") == 0){
        // fg : bring last stopped job to foreground
        int id;
        if (head == NULL){
            printf(RED"-bash: fg: current: no such job\n"RESET);
            return;
        }

        Slist *temp = head;

        if (temp->link == NULL){
            // only one job in list
            id = temp->pid;
            strcpy(input_string, temp->input);  // update input_string so Ctrl+Z shows correct name
            printf(BLUE"%s\n"RESET, temp->input);         // print command name like bash does
        }
        else{
            // traverse to second last node to reach last node
            while (temp->link->link){
                temp = temp->link;
            }
            id = temp->link->pid;                     // last job's pid
            strcpy(input_string, temp->link->input);  // update input_string
            printf(BLUE"%s\n"RESET, temp->link->input);        // print command name
        }

        c1 = id;                            // update c1 so SIGTSTP knows correct foreground pid
        kill(id, SIGCONT);                  // send SIGCONT to resume the stopped process
        waitpid(id, &status, WUNTRACED);    // wait for it to finish OR get stopped again

        //after givinf fg, if the process terminated then we have to delete that from the process
        //WIFSTOPPED(status) returns non-zero (true) if the child was stopped by a signal, and zero (false) if it exited normally.
        if (!WIFSTOPPED(status)){  
            // process finished normally, remove from list
            // if Ctrl+Z during fg, SIGTSTP handler already updated the node status
            delete_last(&head);
        }
    }
    else if (strcmp(input_string, "bg") == 0){
        // bg : resume last stopped job in background
        int id;
        if (head == NULL){
            printf(RED"-bash: bg: current: no such job\n"RESET);
            return;
        }

        Slist *temp = head;

        if (temp->link == NULL){
            // only one job in list
            id = temp->pid;
            temp->status = RUNNING;     // update status to RUNNING
            printf(YELLOW"[%d] %s &\n"RESET, temp->job_num, temp->input);
        }
        else{
            // traverse to second last to reach last node
            while (temp->link->link){
                temp = temp->link;
            }
            id = temp->link->pid;
            temp->link->status = RUNNING;   // update status to RUNNING
            printf(YELLOW"[%d] %s &\n"RESET, temp->link->job_num, temp->link->input);
        }

        kill(id, SIGCONT);              // resume the stopped process in background
        // re-register SIGCHLD handler so we get notified when bg job finishes
        signal(SIGCHLD, signal_handler);
        // NOTE: do NOT delete from list here
        // job stays in list as RUNNING until SIGCHLD fires when it finishes
    }
    else if (strcmp(input_string, "jobs") == 0){
        // jobs : print all current jobs with their statuss
        print_list(head);
    }
}

/*  PRINT ALL JOBS                                                      */
void print_list(Slist *hd){
    if (hd == NULL){
        // list is empty, print nothing (bash behavior)
        // printf("INFO : List is empty\n");
    }
    else{
        while (hd){
            if (hd->status == RUNNING)  //printing just how bash prints
                printf(WHITE"[%d]   Running                 %s &\n"RESET, hd->job_num, hd->input);
            else
                printf(WHITE"[%d]   Stopped                 %s\n"RESET, hd->job_num, hd->input);
            hd = hd->link;
        }
    }
}

/*  INSERT JOB AT END OF LIST                                           */
void insert_last(Slist **hd, int pid){
    Slist *new = malloc(sizeof(Slist));
    new->pid = pid;
    ++job_counter;              // increment global counter for unique job number
    new->job_num = job_counter; // assign job number
    new->status = STOPPED;      // new jobs always start as STOPPED
    new->input = malloc(strlen(input_string) + 1);  // allocate memory for command string
    strcpy(new->input, input_string);               // copy current command into node
    new->link = NULL;

    if (*hd == NULL){
        // list is empty, new node becomes head
        *hd = new;
    }
    else{
        // traverse to last node and append
        Slist *temp = *hd;
        while (temp->link != NULL)
        {
            temp = temp->link;
        }
        temp->link = new;
    }
}

/*  DELETE LAST JOB FROM LIST                                           */
void delete_last(Slist **hd){
    if (*hd == NULL){
        return; // empty list, nothing to delete
    }

    Slist *temp = (*hd);

    if (temp->link == NULL){
        // only one node in list
        free(temp);
        *hd = NULL;
        job_counter = 0;    // reset job counter when list becomes empty
        return;
    }

    // traverse to second last node
    while (temp->link->link){
        temp = temp->link;
    }
    // free last node and set second last's link to NULL
    free(temp->link);
    temp->link = NULL;
}
