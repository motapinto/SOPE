#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


const int MAX_BUFFER_SIZE = 64;

int main(int argc, char * argv[]) {

    if (argc != 2 && argc != 3) {
        printf("Usage: binary <file1>\nor\nUsage: binary <file1> <file2>\n");
        exit(0);
    }

    int file1 = open(argv[1], O_RDONLY);
    if (file1 == -1) {
        perror(argv[1]);
        return 2;
    }
    
    int file2;
    if (argc == 3) {
        file2 = open(argv[2], O_WRONLY | O_EXCL | O_CREAT, 0644);
        if (file2 == -1) {
            perror(argv[2]);
            close(file1);
            return 2;
        }
        dup2(file2, STDOUT_FILENO);
    }
    
    char buffer[MAX_BUFFER_SIZE];
    int read_chars;
    while ( (read_chars = read(file1, buffer, MAX_BUFFER_SIZE)) > 0) {
        write(STDOUT_FILENO, buffer, read_chars);
    }
    close(file1);
    if (argc == 3) close(file2);

    return 0;
}