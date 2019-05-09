#include <stdio.h>
#include <unistd.h>

int main() {

    pid_t pid;

    for (int i = 0; i < 3; i ++) {

        pid = fork();
        if (pid == 0) {
            switch(i) {
                case 0:
                    printf("Hello ");
                    break;
                case 1:
                    printf("my ");
                    break;
                case 2:
                    printf("friends!\n");
                    break;
                default:
                    break;
            }
            break;
        }
    }

    return 0;
}