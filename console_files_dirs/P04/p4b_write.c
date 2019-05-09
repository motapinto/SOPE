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

    int file = open("p4b_output.txt", O_WRONLY | O_EXCL | O_CREAT, 0644);

    struct to_be_stored struc;
    char buffer[MAX_BUFFER_SIZE];
    while(1) {
        printf("Student name: "); fflush(stdout);
        int nc = read(STDIN_FILENO, buffer, MAX_BUFFER_SIZE);
        strncpy(struc.name, buffer, nc-1);

        printf("Classification: "); fflush(stdout);
        read(STDIN_FILENO,  buffer, MAX_BUFFER_SIZE);
        struc.classification = atoi(buffer);

        write(file, &struc, sizeof(struct to_be_stored));
    }

    close(file);

    return 0;
}