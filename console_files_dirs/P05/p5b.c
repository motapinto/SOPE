// PROGRAMA p5b.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int fd;
    char *text1="CCCCC";
    char *text2="DDDDD";
    fd = open("f1.txt", O_WRONLY|O_SYNC,0600);

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