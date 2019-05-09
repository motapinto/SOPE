#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


const int MAX_BUFFER_SIZE = 64;

int main(int argc, char * argv[]) {

    if (argc != 3) {
        printf("Usage: binary <file1> <file2>\n");
        exit(0);
    }

    int file1 = open(argv[1], O_RDONLY);
    if (file1 == -1) {
        perror(argv[1]);
        return 2;
    }

    int file2 = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (file2 == -1) {
        perror(argv[2]);
        close(file1);
        return 3;
    }
    
    char buffer[MAX_BUFFER_SIZE];
    int read_chars;
    while ( (read_chars = read(file1, buffer, MAX_BUFFER_SIZE)) > 0) {
        write(file2, buffer, read_chars);
    }
    close(file1);
    close(file2);

    return 0;
}