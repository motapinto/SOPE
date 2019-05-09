#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

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

    while(1) {
        v += increment;
        sleep(1);
        printf("%d\n", v);
    }
    return 0;
}