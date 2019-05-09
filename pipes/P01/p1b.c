#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define READ 0
#define WRITE 1

struct numbers {
    int n1, n2;
};

int main() {

    int fd[2];
    pid_t pid;

    pipe(fd);

    pid = fork();
    struct numbers nums;
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        /* Child */
        close(fd[WRITE]);
        read(fd[READ], &nums, sizeof(struct numbers));

        printf("Sum = %d\n", nums.n1 + nums.n2);
        printf("Dif = %d\n", nums.n1 - nums.n2);
        printf("Product = %d\n", nums.n1 * nums.n2);
        if (nums.n2 == 0) 
            printf("Div = invalid");
        else
            printf("Div = %lf\n", (double)(nums.n1)/nums.n2);

        close(fd[READ]);
    }
    else {
        /* Parent */
        close(fd[READ]);
        scanf("%d %d", &nums.n1, &nums.n2);
        write(fd[WRITE], &nums, sizeof(struct numbers));
        close(fd[WRITE]);
    }


    return 0;
}