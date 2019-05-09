#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h> 

int increment = 1;

void sigterm_handler(int signo) {
    printf("sigterm caught\n");
    exit(0);
}

void sigusr_handler(int signo) {
    if (signo == SIGUSR1) {
        increment = 1;
    }
    else if (signo == SIGUSR2) {
        increment = -1;
    }
}

int main() {    

    int v = 0;
    srand(time(NULL));

    struct sigaction action;
    action.sa_handler = sigterm_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGTERM, &action, NULL)) {
        printf("error installing sigterm handler\n");
        exit(1);
    }

    action.sa_handler = sigusr_handler;
    if (sigaction(SIGUSR1, &action, NULL)) {
        printf("error installing sigusr1 handler\n");
        exit(1);
    }
    if (sigaction(SIGUSR2, &action, NULL)) {
        printf("error installing sigusr2 handler\n");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        printf("fork error\n");
        exit(1);
    }
    else if (pid > 0) {
        // Send random signals to child
        while(1) {
            int random = rand() % 2;
            kill(pid, random == 0 ? SIGUSR1 : SIGUSR2);
            sleep(10);
        }
    }
    else {
        int counter = 0;

        // Print values
        while(1) {
            v += increment;
            counter++;
            sleep(1);
            printf("Value %d: %d\n", counter, v);
            if (counter >= 50) {
                kill(getppid(), SIGTERM);
                raise(SIGTERM);
            }
        }

    }

    printf("SHOULD NOT GET HERE!!!\n");
    return 0;
}