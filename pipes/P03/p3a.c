#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char * argv[]) {

    if (argc != 2) {
        printf("usage %s filename\n", argv[0]);
        exit(1);
    }

    FILE * file;

    int fd[2];
    pid_t pid;

    pipe(fd);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid > 0) {
        close(fd[0]);
        if ((file = fopen(argv[1], "r")) == NULL) {
            perror("open");
            exit(1);
        }

        char c[2];
        while(fgets(c, 2, file) != NULL)
            write(fd[1], c, 1);


        close(fd[1]);
        wait(NULL);
    }

    else {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);

        execlp("sort", "sort", NULL);
        close(fd[0]);
    }

    fclose(file);
    return 0;
}