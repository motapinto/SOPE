#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MAX_BUFFER_SIZE = 64;

int main() {

    char cmd[MAX_BUFFER_SIZE];
    char delimiter[2] = " ";
    char * decomp;

    fgets(cmd, MAX_BUFFER_SIZE, stdin);
    decomp = strtok(cmd, delimiter);

    while (decomp != NULL) {
        printf("%s\n", decomp);

        decomp = strtok(NULL, delimiter);
    }


    return 0;
}