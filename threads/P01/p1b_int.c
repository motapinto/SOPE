// PROGRAMA p01.c
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define STDERR 2
#define NUMITER 10000
void * thrfunc(void * arg)
{
    int i;
    fprintf(stderr, "Starting thread %s\n", (char *) arg);
    for (i = 1; i <= NUMITER; i++) write(STDERR, arg,1);
    return NULL;
}

int main()
{
    pthread_t ta, tb;

    int c1 = 0x31, c2 = 0x32;
    pthread_create(&ta, NULL, thrfunc, &c1);
    pthread_create(&tb, NULL, thrfunc, &c2);
    pthread_join(ta, NULL);
    pthread_join(tb, NULL);
    return 0;
} 