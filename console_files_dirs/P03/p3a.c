#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


const int MAX_BUFFER_SIZE = 64;

int main(int argc, char * argv[]) {

    if (argc != 2) {
        printf("Usage: binary <file1>\n");
        exit(0);
    }

    int file1 = open(argv[1], O_RDONLY);
    if (file1 == -1) {
        perror(argv[1]);
        return 2;
    }
    
    char buffer[MAX_BUFFER_SIZE];
    int read_chars;
    while ( (read_chars = read(file1, buffer, MAX_BUFFER_SIZE)) > 0) {
        write(STDOUT_FILENO, buffer, read_chars);
    }
    close(file1);

    return 0;
}