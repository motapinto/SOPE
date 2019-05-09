#include <stdio.h>
#include <unistd.h>


int main() {
    pid_t pid;
    pid = fork();
    if (pid > 0) {
        printf("Hello ");
    }
    else {
        sleep(1);
        printf("world !\n");
    }
    return 0;
}