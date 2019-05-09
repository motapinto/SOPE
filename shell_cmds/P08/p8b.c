#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>

int main(int argc, char * argv[]) {

    long ticks_sec = sysconf(_SC_CLK_TCK);
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);


    if (argc != 3) {
        printf("Usage: ./executable_name n1 n2\n");
        exit(0);
    }

    srand(time(NULL));

    int arg1 = atoi(argv[1]);
    int arg2 = atoi(argv[2]);
    int random_n, iter = 1;
    struct tms times_var;

    if (arg2 > arg1-1) {
        printf("Invalid input\n");
        exit(0);
    }

    while ((random_n = rand() % arg1) != arg2) {
        times(&times_var);
        gettimeofday(&tv2, NULL);
        struct timeval tvdiff = { tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec };
        if (tvdiff.tv_usec < 0) { tvdiff.tv_usec += 1000000; tvdiff.tv_sec -= 1; }

        printf("Iter %d: %LF %Lf %LF\n", iter, (long double) tvdiff.tv_sec, (long double) times_var.tms_utime / ticks_sec, (long double) times_var.tms_stime / ticks_sec);

        iter++;
    }
    gettimeofday(&tv2, NULL);
    struct timeval tvdiff = { tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec };
    if (tvdiff.tv_usec < 0) { tvdiff.tv_usec += 1000000; tvdiff.tv_sec -= 1; }
    printf("Iter %d: %LF %Lf %LF\n", iter, (long double) tvdiff.tv_sec, (long double) times_var.tms_utime / ticks_sec, (long double) times_var.tms_stime / ticks_sec);

    return 0;
}