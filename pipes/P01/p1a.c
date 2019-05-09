#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define READ 0
#define WRITE 1

int main() {

    int fd[2];
    pid_t pid;

    pipe(fd);

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        /* Child */
        close(fd[WRITE]);
        int n[2];
        read(fd[READ], n, 2 * sizeof(int));

        printf("Sum = %d\n", n[0] + n[1]);
        printf("Dif = %d\n", n[0] - n[1]);
        printf("Product = %d\n", n[0] * n[1]);
        if (n[1] == 0) 
            printf("Div = invalid");
        else
            printf("Div = %lf\n", (double)(n[0])/n[1]);

        close(fd[READ]);
    }
    else {
        /* Parent */
        close(fd[READ]);
        int n[2];
        scanf("%d %d", &n[0], &n[1]);
        write(fd[WRITE], n, 2 * sizeof(int));
        close(fd[WRITE]);
    }


    return 0;
}