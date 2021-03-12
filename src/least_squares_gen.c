#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX_ABS_EPSILON 0.0
#define MAX_ABS_COEFF 100.0

double random_in_range(double max_abs) {
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return scale * ( 2*max_abs ) - max_abs;  /* [min, max] */
}

double f(double x, double a, double b) {
    return a*x + b + random_in_range(MAX_ABS_EPSILON);
}

int main(int argc, char *argv[]) {
    double *y, x;
    double a, b;
    int n, i;
    FILE *out_file;
    
	if (argc < 2) {
        printf("Entre com o valor de n.\n");
        return 1;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }

    out_file = fopen("xydata", "w");
    if (out_file == NULL) {
        printf("error opening file\n");
        return 1;
    }
    
    a = random_in_range(MAX_ABS_COEFF);
    b = random_in_range(MAX_ABS_COEFF);
    y = (double *) malloc(n*sizeof(double)); 

	for (i = 0; i < n; i++) {
        x = i * 0.001;
        y[i] = f(x, a, b);
    }

    printf("Foram geradas %d linhas com a função %lfx + %lf\n", n, a, b);
    fprintf(out_file, "%d\n", n);

    for (i = 0; i < n; i ++) {
        fprintf(out_file, "%lf %lf\n", (double) i*0.001, y[i]);
    }
    
	return(0);
}