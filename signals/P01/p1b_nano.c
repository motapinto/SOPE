// PROGRAMA p01a.c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
void sigint_handler(int signo)
{
 printf("In SIGINT handler ...\n");
}
int main(void)
{
 if (signal(SIGINT,sigint_handler) < 0)
 {
 fprintf(stderr,"Unable to install SIGINT handler\n");
 exit(1);
 }
 printf("Sleeping for 30 seconds ...\n");
 int counter = 30;
 struct timespec req, rem;

 int ret_val = 1;
 req.tv_sec = counter;
 while (ret_val) {
    ret_val = nanosleep(&req, &rem);
    printf("%ld\n", rem.tv_sec);
    if (errno == EINTR) {
        req.tv_sec = rem.tv_sec;
        req.tv_nsec = rem.tv_nsec;
    }
 }
 printf("Waking up ...\n");
 exit(0);
} 