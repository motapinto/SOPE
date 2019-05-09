#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include "file.h"

int numSIGUSR1, numSIGUSR2 = 0;
extern char* file_name_glob;
extern FILE *fp_logs;

void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        printf("\nSIGINT Handler Executing\n");
        
        if(fp_logs!=NULL){
            print_event_type("SIGINT---EXIT");
        }

        fflush(stdin);
        fflush(stdout);
        int filedes[2];
        if (pipe(filedes) == -1)
        {
            perror("Pipe");
            exit(1);
        }
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Fork");
            exit(1);
        }
        else if (pid == 0)
        {
            while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR))
            {
            }
            close(filedes[1]);
            close(filedes[0]);
            execlp("rm", "rm", "-rf", "/tmp/*", NULL);
            perror("Execlp");
            exit(1);
        }
        close(filedes[1]);
        close(filedes[0]);
        int status;
        wait(&status);
        if (status == 0)
            printf("Temporary Files Removed... Program will now Exit\n");
    }
    else if (signo == SIGUSR1)
    {
        numSIGUSR1++;
        printf("New Directory Found! %d/%d Directories/Files ATM\n", numSIGUSR1, numSIGUSR2);
        print_event_type("act");
    }
    else if (signo == SIGUSR2)
    {
        numSIGUSR2++;
        if(is_directory(file_name_glob)){
            printf("New File Found! %d/%d Directories/Files ATM\n", numSIGUSR1, numSIGUSR2);

            if(fp_logs!=NULL){
                print_event_type("New file found");
            }
        }

        
    }
}