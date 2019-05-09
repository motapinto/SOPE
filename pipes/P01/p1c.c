#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define READ 0
#define WRITE 1

#define MAX_BUFFER_SIZE 256

int main() {

    int fd[2];
    pid_t pid;

    pipe(fd);
    char word1[MAX_BUFFER_SIZE];
    char word2[MAX_BUFFER_SIZE];

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        /* Child */
        close(fd[WRITE]);
        
        read(fd[READ], word1, MAX_BUFFER_SIZE);
        read(fd[READ], word2, MAX_BUFFER_SIZE);

        int num1, num2;
        num1 = atoi(word1);
        num2 = atoi(word2);

        printf("Sum = %d\n", num1 + num2);
        printf("Dif = %d\n", num1 - num2);
        printf("Product = %d\n", num1 * num2);
        if (num2 == 0) 
            printf("Div = invalid");
        else
            printf("Div = %lf\n", (double)(num1)/num2);

        close(fd[READ]);
    }
    else {
        /* Parent */
        close(fd[READ]);
        scanf("%s %s", word1, word2);
        write(fd[WRITE], word1, MAX_BUFFER_SIZE);
        write(fd[WRITE], word2, MAX_BUFFER_SIZE);
        close(fd[WRITE]);
    }


    return 0;
}