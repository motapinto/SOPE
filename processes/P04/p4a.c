#include <stdio.h>
#include <unistd.h>


int main() {
    pid_t pid;
    pid = fork();
    if (pid > 0) {
        sleep(1);
        printf("world !\n");
    }
    else {
        printf("Hello ");
    }
    return 0;
}