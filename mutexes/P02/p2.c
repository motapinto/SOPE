// PROGRAMA p01.c
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>


#define STDERR 2
#define NUMITER 10000

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
int N = 50000;

void * thrfunc(void * arg)
{
    void * count = malloc(sizeof(int));
    *(int*)count = 0;
    fprintf(stderr, "Starting thread %s\n", (char *) arg);
    
    while(N > 0) {
        write(STDERR,arg,1);
        pthread_mutex_lock(&mut);
        if (N<= 0) {
            pthread_mutex_unlock(&mut);
            break;
        }
        N--; 
        pthread_mutex_unlock(&mut);
        (*(int*)count)++;
    }
    return count;
}

int main()
{
    pthread_t ta, tb;
    pthread_create(&ta, NULL, thrfunc, "1");
    pthread_create(&tb, NULL, thrfunc, "2");

    void * count1, * count2;
    pthread_join(ta, &count1);
    pthread_join(tb, &count2);

    write(STDERR_FILENO, "\n", 1);

    fprintf(stderr, "Thread 1 wrote %d chars\n", *((int *)count1));
    fprintf(stderr, "Thread 2 wrote %d chars\n", *((int *)count2));

    free(count1);
    free(count2);
    return 0;
} 