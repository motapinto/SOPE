// PROGRAMA p5a.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int fd;
    char *text1="AAAAA";
    char *text2="BBBBB";
    fd = open("f1.txt",O_CREAT|O_EXCL|O_TRUNC|O_WRONLY|O_SYNC,0600);

    if (fd == -1) {
        perror("Open error");
    }

    getchar();
    if (write(fd,text1,5) == -1) perror("Write error");
    getchar();
    if (write(fd,text2,5) == -1) perror("Write error");
    close(fd);
    return 0;
 } 