
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define FLOAT_VAL 2
#define INT_VAL 3
#define INVALID_VAL 4

void readint(int fd, int  * res);

int main() {

    int INT = INT_VAL;
    int INVALID = INVALID_VAL;
    int FLOAT = FLOAT_VAL;

    int fd_r, fd_w;

    if (mkfifo("/tmp/fifo_req", 0660) < 0 ) {
        if(errno==EEXIST) printf("FIFO '/tmp/fifo_req' already exists\n");
        else printf("Couldnt create FIFO\n");
        exit(1);
    }

    if (mkfifo("/tmp/fifo_ans", 0660) < 0 ) {
        if(errno==EEXIST) printf("FIFO '/tmp/fifo_req' already exists\n");
        else printf("Couldnt create FIFO\n");
        exit(1);
    }

    if ((fd_w = open("/tmp/fifo_ans", O_WRONLY)) == -1) {
        perror("open");
        exit(1);
    }

    if ((fd_r = open("/tmp/fifo_req", O_RDONLY)) == -1) {
        perror("open");
        exit(1);
    }

    int n[2];
    while(1) {
        readint(fd_r, &n[0]);
        readint(fd_r, &n[1]);

        if (n[0] == 0 && n[1] == 0) break;

        int calc = n[0] + n[1];
        write(fd_w, &INT, sizeof(int));
        write(fd_w, &calc, sizeof(int));

        calc = n[0] - n[1];
        write(fd_w, &INT, sizeof(int));
        write(fd_w, &calc, sizeof(int));

        calc = n[0] * n[1];
        write(fd_w, &INT, sizeof(int));
        write(fd_w, &calc, sizeof(int));

        if (n[1] == 0) {
            write(fd_w, &INVALID, sizeof(int));
            write(fd_w, 0, sizeof(double));
        }
        else {
            double calcd = (double) (n[0])/ n[1];
            write(fd_w, &FLOAT, sizeof(int));
            write(fd_w, &calcd, sizeof(double));
        }

    }

    close(fd_r);
    close(fd_w);
    unlink("/tmp/fifo_ans");
    unlink("/tmp/fifo_req");

    return 0;
}


void readint(int fd, int * res)
{
 int n;
 do {
    n = read(fd,res,sizeof(int));
 } while (n==0);
 if (n == -1 ) {
     perror("read");
     exit(1);
 }
} 