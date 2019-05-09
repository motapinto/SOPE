
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FLOAT_VAL 2
#define INT_VAL 3
#define INVALID_VAL 4

void readint(int fd, int  * res);
void readdouble(int fd, double * res);

int main() {
    int fd_r, fd_w;
    do {
        fd_r=open("/tmp/fifo_ans", O_RDONLY);
        if (fd_r==-1) sleep(1);
    } while (fd_r==-1);
    
    do {
        fd_w=open("/tmp/fifo_req", O_WRONLY);
        if (fd_r==-1) sleep(1);
    } while (fd_r==-1);

    int n[2];
    scanf("%d %d", &n[0], &n[1]);
    write(fd_w, n, 2 * sizeof(int));

    if (n[0] == 0 && n[1] == 0) {
        goto end;
    }

    int calc, status;
    readint(fd_r, &status);
    if (status == INVALID_VAL) {
        perror("invalid");
        exit(1);
    }
    readint(fd_r, &calc);
    printf("Sum = %d\n", calc);

    readint(fd_r, &status);
    if (status == INVALID_VAL) {
        perror("invalid");
        exit(1);
    }
    readint(fd_r, &calc);
    printf("Dif = %d\n", calc);


    
    readint(fd_r, &status);
    if (status == INVALID_VAL) {
        perror("invalid");
        exit(1);
    }
    readint(fd_r, &calc);
    printf("Mult = %d\n", calc);


    double calcd;
    readint(fd_r, &status);
    if (status == INVALID_VAL) {
        perror("invalid");
        exit(1);
    }
    readdouble(fd_r, &calcd);
    printf("Div = %lf\n", calcd);
    
    end:

    close(fd_r);
    close(fd_w);

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

void readdouble(int fd, double * res)
{
 int n;
 do {
    n = read(fd,res,sizeof(double));
 } while (n==0);
 if (n == -1 ) {
     perror("read");
     exit(1);
 }
} 