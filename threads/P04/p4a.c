// PROGRAMA p04.c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 10

void * thr_func(void * arg) {

    sleep(1);
    fprintf(stderr, "Hello from thread %d\n", *(int*)arg);

    return arg;
}

int main() {

    pthread_t threads[NUM_THREADS];
    int args[NUM_THREADS];

    for (int t = 0; t < NUM_THREADS; t++) {
        args[t] = t;
        pthread_create(&threads[t], NULL, thr_func, &args[t]);
    }

    void * ret;
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], &ret);
        fprintf(stderr, "Thread %d finished\n", *(int*)ret);
    }

    return 0;
}