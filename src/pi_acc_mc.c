#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
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
    long long N =  1000;
    int count_points = 64000; // 64 points = 128 doubles = 1kb

    if (argc == 3) {
        N = atoll(argv[1]);
        count_points = atoll(argv[2]);
    }

    struct timespec vartime = timer_start();

    double *xs = (double *)malloc(count_points*sizeof(double));
    double *ys = (double *)malloc(count_points*sizeof(double));
    long long sum_all = 0;

    for (int itS = 0; itS < N; itS++) {
        for (int itP = 0; itP < count_points; itP++) {
            xs[itP] = (double)rand()/RAND_MAX;
            ys[itP] = (double)rand()/RAND_MAX;
        }
        long long sum = 0;
        #pragma acc parallel loop \
            copyin(xs[0:count_points], ys[0:count_points]) \
            copy(sum) \
            reduction(+:sum)
        for (int i = 0; i < count_points; i++) {
            if (xs[i]*xs[i] + ys[i]*ys[i] < 1.0)
                sum++;
        }
        sum_all += sum;
    }

    pi = 4.0*sum_all/count_points/N;

    long long time_elapsed_nanos = timer_end(vartime);
    double elapsed = time_elapsed_nanos*0.000000001;
    // printf("%lf\n", pi);
    printf("%lf\n", elapsed);

    // printf("O valor de pi é: %.12f\n",pi);
    return(0);
}
