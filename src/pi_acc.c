#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ref: https://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
// call this function to start a nanosecond-resolution timer
struct timespec timer_start() {
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    return start_time;
}

// call this function to end a timer, returning nanoseconds elapsed as a long
long long timer_end(struct timespec start_time){
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long long diffInNanos = (end_time.tv_sec - start_time.tv_sec) * (long long)1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    return diffInNanos;
}

int main(int argc, char *argv[]) { /* acc_calcpi.c  */
    double pi = 0.0f;
    long long N = 1000000000;

    if (argc == 2)
        N = atoll(argv[1]);

    struct timespec vartime = timer_start();

    #pragma acc parallel loop copy(pi) reduction(+:pi)
    for (long long i = 0; i < N + 1; i++) {
        double t = (double)i/N;
        pi += 1.0/(1.0 + t*t);
   	}
    pi -= 0.5*(1.0/(1.0 + 0.0*0.0) + 1.0/(1.0 + 1.0*1.0));
    pi *= 4.0/(N + 1);

    long long time_elapsed_nanos = timer_end(vartime);
    double elapsed = time_elapsed_nanos*0.000000001;
    printf("%lf\n", elapsed);

    // printf("O valor de pi é: %.12f\n",pi);
    return(0);
}
