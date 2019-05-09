#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

const int MAX_BUFFER_SIZE = 64;

int main() {

    int file = open("p4a_output.txt", O_WRONLY | O_EXCL | O_CREAT, 0644);

    int buffer[MAX_BUFFER_SIZE];
    char * separator = " - ";
    char * newline = "\n\n";
    while(1) {
        printf("Student name: ");
        fflush(stdout);
        int nc = read(STDIN_FILENO, buffer, MAX_BUFFER_SIZE);
        if (nc == 1) break;

        write(file, buffer, nc-1);

        printf("Grade: ");
        fflush(stdout);
        nc = read(STDIN_FILENO, buffer, MAX_BUFFER_SIZE);
        if (nc == 1) break;

        write(file, separator, 3);
        write(file, buffer, nc-1);

        write(file, newline, 2);
    }

    close(file);

    return 0;
}