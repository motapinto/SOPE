#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define WRITE 1
#define READ 0


int main(int argc, char * argv[]) {

    if (argc != 3) {
        printf("usage %s dir grep_stuff\n", argv[0]);
        exit(1);
    }

    int fd1[2];
    int fd2[2];
    pid_t pid;

    pipe(fd1);
    pipe(fd2);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid > 0) {
        /* Parent */
        close(fd1[READ]);
        close(fd2[READ]);
        close(fd2[WRITE]);

        dup2(fd1[WRITE], STDOUT_FILENO);
        execlp("ls", "ls", argv[1], "-laR", NULL);

        close(fd1[WRITE]);
    }

    else {
        /* Childs */
        close(fd1[WRITE]);
        pid = fork();
        if (pid == -1) {
            perror("fork2");
            exit(1);
        }
        else if (pid > 0) { 
            /* Child 1 */
            close(fd2[READ]);

            dup2(fd1[READ], STDIN_FILENO);
            dup2(fd2[WRITE], STDOUT_FILENO);
            execlp("grep", "grep", argv[2], NULL);

            close(fd2[WRITE]);
            close(fd1[READ]);
        }
        else {
            /* Child 2 */
            close(fd2[WRITE]);
            close(fd1[READ]);

            dup2(fd2[READ], STDIN_FILENO);
            execlp("sort", "sort", NULL);

            close(fd2[READ]);
        }
    }

    return 0;
}