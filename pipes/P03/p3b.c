#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define WRITE 1
#define READ 0


int main(int argc, char * argv[]) {

    if (argc != 2) {
        printf("usage %s filename\n", argv[0]);
        exit(1);
    }

    int fd[2];
    pid_t pid;

    pipe(fd);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid > 0) {
        close(fd[READ]);
        dup2(fd[WRITE], STDOUT_FILENO);

        execlp("cat", "cat", argv[1], NULL);

        close(fd[1]);
        wait(NULL);
    }

    else {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);

        execlp("sort", "sort", NULL);
        close(fd[0]);
    }

    return 0;
}