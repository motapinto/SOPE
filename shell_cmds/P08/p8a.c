#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char * argv[]) {

    if (argc != 3) {
        printf("Usage: ./executable_name n1 n2\n");
        exit(0);
    }

    srand(time(NULL));

    int arg1 = atoi(argv[1]);
    int arg2 = atoi(argv[2]);
    int random_n, iter = 1;

    if (arg2 > arg1-1) {
        printf("Invalid input\n");
        exit(0);
    }

    while ((random_n = rand() % arg1) != arg2) {
        printf("Iteration %d: %d\n", iter, random_n);
        iter++;
    }
    printf("Iteration %d: %d\n", iter, random_n);

    return 0;
}