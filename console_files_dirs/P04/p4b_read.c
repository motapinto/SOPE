#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
const int MAX_BUFFER_SIZE = 50;

struct to_be_stored{
    char name[50];
    int classification;
};


int main() {

    int file = open("p4b_output.txt", O_RDONLY);

    struct to_be_stored struc;

    read(file, &struc, sizeof(struct to_be_stored));

    printf("%s - %d\n", struc.name, struc.classification);
    fflush(stdout);
    close(file);

    return 0;
}