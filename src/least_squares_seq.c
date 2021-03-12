
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <omp.h>

int main(int argc, char **argv) {

    double *x, *y, time_start, time_end;
    double SUMx, SUMy, SUMxy, SUMxx, SUMres, res, slope, y_intercept, y_estimate;
    int i, j, n;
    /*int new_sleep (int seconds);*/
    FILE *infile;

    infile = fopen("xydata", "r");
    if (infile == NULL)
        printf("error opening file\n");

    fscanf(infile, "%d", &n);
    x = (double *)malloc(n * sizeof(double));
    y = (double *)malloc(n * sizeof(double));
    for (i = 0; i < n; i++)
        fscanf(infile, "%lf %lf", &x[i], &y[i]);

    time_start = omp_get_wtime();

    SUMx = 0;
    SUMy = 0;
    SUMxy = 0;
    SUMxx = 0;
    
    for (j = 0; j < n; j++) {
        SUMx = SUMx + x[j];
        SUMy = SUMy + y[j];
        SUMxy = SUMxy + x[j] * y[j];
        SUMxx = SUMxx + x[j] * x[j];
    }

    slope = (SUMx * SUMy - n * SUMxy) / (SUMx * SUMx - n * SUMxx);
    y_intercept = (SUMy - slope * SUMx) / n;

    SUMres = 0;
    for (i = 0; i < n; i++) {
        y_estimate = slope * x[i] + y_intercept;
        res = y[i] - y_estimate;
        SUMres = SUMres + res * res;
    }

    time_end = omp_get_wtime();

    printf("Equation: y = %6.2lfx + %6.2lf\t", slope, y_intercept);
    printf("Residual sum = %6.2lf\t", SUMres);
    printf("Original version. \tn = %d. \tTime elapsed proccesses: %1.3f\n", n, time_end - time_start);
    
    return 0;
}
