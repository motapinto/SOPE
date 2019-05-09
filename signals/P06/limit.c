#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int alarmflag = 0;
void sigalarm_handler(int signo) {
    alarmflag = 1;
    printf("alarm triggered\n");
}

void sigchild_handler(int signo) {
    printf("child survived\n");
    exit(0);
}

int main(int argc, char * argv[]) {

    if (argc < 3) {
        printf("Usage: %s t prog <arg1> <arg2> <...>\n", argv[0]);
        exit(1);
    }

    int time_limit = atoi(argv[1]);
    time_limit = time_limit > 30 ? 30 : time_limit;
    
    pid_t pid = fork();

    if (pid == -1) {
        printf("fork error\n");
        exit(1);
    }
    else if (pid > 0) {
        signal(SIGCHLD, sigchild_handler);
        signal(SIGALRM, sigalarm_handler);
        alarm(time_limit);
        while(!alarmflag) sleep(5);
        kill(pid, SIGKILL);
        printf("child killed\n");        
    }
    else {
        execlp("./prog", argv[2], argv[3], NULL);
        printf("error with execlp\n");
        exit(1);
    }

    return 0;
}