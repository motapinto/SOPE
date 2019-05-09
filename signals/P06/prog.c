#include <stdio.h>
#include <unistd.h>

int main(int argc, char * argv[]) {

    int loops = 10;
    while (loops) {
        printf("%s\n", argv[1]);
        sleep(5);
        loops--;
    }

    return 0;
}