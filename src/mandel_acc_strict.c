#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define X_RESN 1000 /* x resolution */
#define Y_RESN 1000 /* y resolution */
#define MAX_ITER 200




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




typedef struct complextype {
    double real, imag;
} Compl;




int main(int argc, char *argv[])
{
    struct timespec vartime = timer_start();

    /* Mandelbrot variables */
    int *ks;
    ks = (int *)malloc((X_RESN*Y_RESN) * sizeof(int));

    double *ds;
    ds = (double *)malloc((X_RESN*Y_RESN) * sizeof(double));

    /* Calculate and draw points */
    int i, j, k;
    double d;
    Compl z, c, t;
    #pragma acc parallel loop \
        copyout(ks[0:X_RESN*Y_RESN], ds[0:X_RESN*Y_RESN]) \
        private(k, d, c, z, t)
    for (int it = 0; it < X_RESN*Y_RESN; it++)
    {
        i = it / Y_RESN;
        j = it % Y_RESN;

        // mandelbrot set is defined in the region of x = [-2, +2] and y = [-2, +2]
        double u = ((double)i - (X_RESN / 2.0)) / (X_RESN / 4.0);
        double v = ((double)j - (Y_RESN / 2.0)) / (Y_RESN / 4.0);

        z.real = z.imag = 0.0;
        c.real = v;
        c.imag = u;

        k = 0;
        d = 0.0;

        double lengthsq;
        do
        {
            t = z;
            z.imag = 2.0 * t.real * t.imag + c.imag;
            z.real = t.real * t.real - t.imag * t.imag + c.real;
            lengthsq = z.real * z.real + z.imag * z.imag;
            d += pow(pow(z.imag - t.imag, 2.0) + pow(z.real - t.real, 2.0), 0.5);
            k++;
        } while (lengthsq < 4.0 && k < MAX_ITER);

        ks[it] = k;
        ds[it] = d;
    }

    free(ks);
    free(ds);

    long long time_elapsed_nanos = timer_end(vartime);
    double elapsed = time_elapsed_nanos*0.000000001;
    printf("%lf\n", elapsed);

    /* Program Finished */
    return 0;
}
