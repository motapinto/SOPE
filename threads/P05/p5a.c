
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 4


void * thr_sum(void * arg) {
    int n1 = ((int *) arg)[0];
    int n2 = ((int*) arg)[1];
    printf("%d + %d = %d\n", n1, n2, n1+n2);
    return NULL;
}
void * thr_dif(void * arg) {
    int n1 = ((int *) arg)[0];
    int n2 = ((int*) arg)[1];
    printf("%d - %d = %d\n", n1, n2, n1-n2);
    return NULL;
}
void * thr_mult(void * arg) {
    int n1 = ((int *) arg)[0];
    int n2 = ((int*) arg)[1];
    printf("%d * %d = %d\n", n1, n2, n1*n2);
    return NULL;
}
void * thr_quo(void * arg) {
    int n1 = ((int *) arg)[0];
    int n2 = ((int*) arg)[1];
    printf("%d / %d = %d\n", n1, n2, n1/n2);
    return NULL;
}

int main() {

    void *(*THREAD_FUNC[NUM_THREADS])(void*) = {
        thr_sum,
        thr_dif,
        thr_mult,
        thr_quo
    };

    int nums[2];
    scanf("%d %d", &nums[0], &nums[1]);
    
    pthread_t threads[NUM_THREADS];
    int iter[NUM_THREADS];
    for (int t = 0; t < NUM_THREADS; t++) {
        iter[t] = t;
        pthread_create(&threads[iter[t]], NULL, THREAD_FUNC[iter[t]], (void*)nums);
    }


    pthread_exit(0);
}