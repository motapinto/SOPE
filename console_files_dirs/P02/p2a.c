#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const int MAX_BUFFER_SIZE = 64;

int main(int argc, char * argv[]) {

    if (argc != 3) {
        printf("Usage: binary <file1> <file2>\n");
        exit(0);
    }

    FILE * file1 = fopen(argv[1], "r");
    FILE * file2 = fopen(argv[2], "w");
    
    char buffer[MAX_BUFFER_SIZE];
    int read_chars;
    while ( (read_chars = fread(buffer, 1, MAX_BUFFER_SIZE, file1)) > 0) {
        fwrite(buffer, 1, read_chars, file2);
    }
    fclose(file1);
    fclose(file2);

    return 0;
}