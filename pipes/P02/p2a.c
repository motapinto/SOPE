#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define READ 0
#define WRITE 1

#define FLOAT_VAL 2
#define INT_VAL 3
#define INVALID_VAL 4

int main() {

    int parent_child[2];
    int child_parent[2];

    pid_t pid;

    pipe(parent_child);
    pipe(child_parent);

    pid = fork();

    int INT = INT_VAL;
    int INVALID = INVALID_VAL;
    int FLOAT = FLOAT_VAL;

    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        /* Child */
        close(child_parent[READ]);
        close(parent_child[WRITE]);

        int n[2];
        read(parent_child[READ], n, 2 * sizeof(int));
        
        int calc = n[0] + n[1];
        write(child_parent[WRITE], &INT, sizeof(int));
        write(child_parent[WRITE], &calc, sizeof(int));

        calc = n[0] - n[1];
        write(child_parent[WRITE], &INT, sizeof(int));
        write(child_parent[WRITE], &calc, sizeof(int));

        calc = n[0] * n[1];
        write(child_parent[WRITE], &INT, sizeof(int));
        write(child_parent[WRITE], &calc, sizeof(int));

        if (n[1] == 0) {
            write(child_parent[WRITE], &INVALID, sizeof(int));
            write(child_parent[WRITE], 0, sizeof(double));
        }
        else {
            double calcd = (double) (n[0])/ n[1];
            write(child_parent[WRITE], &FLOAT, sizeof(int));
            write(child_parent[WRITE], &calcd, sizeof(double));
        }

        close(parent_child[READ]);
        close(child_parent[WRITE]);
    }
    else {
        /* Parent */
        close(parent_child[READ]);
        close(child_parent[WRITE]);
        int n[2];
        scanf("%d %d", &n[0], &n[1]);
        write(parent_child[WRITE], n, 2 * sizeof(int));

        int calc, status;
        read(child_parent[READ], &status, sizeof(int));
        if (status == INVALID) {
            perror("invalid");
            exit(1);
        }
        read(child_parent[READ], &calc, sizeof(int));
        printf("Sum = %d\n", calc);



        read(child_parent[READ], &status, sizeof(int));
        if (status == INVALID) {
            perror("invalid");
            exit(1);
        }
        read(child_parent[READ], &calc, sizeof(int));
        printf("Dif = %d\n", calc);


        
        read(child_parent[READ], &status, sizeof(int));
        if (status == INVALID) {
            perror("invalid");
            exit(1);
        }
        read(child_parent[READ], &calc, sizeof(int));
        printf("Mult = %d\n", calc);


        double calcd;        
        read(child_parent[READ], &status, sizeof(int));
        if (status == INVALID) {
            perror("invalid");
            exit(1);
        }
        read(child_parent[READ], &calcd, sizeof(double));
        printf("Div = %lf\n", calcd);

        close(parent_child[WRITE]);
        close(child_parent[READ]);
    }


    return 0;
}