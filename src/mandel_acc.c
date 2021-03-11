#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <time.h>

int X_RESN = 1000; /* x resolution */
int Y_RESN = 1000; /* y resolution */
int MAX_ITER = 200;




// ref: Creating bitmap files from byets data: https://stackoverflow.com/a/47785639
const int BYTES_PER_PIXEL = 3; /// red, green and blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

void generateBitmapImage (unsigned char* image, int height, int width, char* imageFileName)
{
    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    FILE* imageFile = fopen(imageFileName, "wb");

    unsigned char* fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

    int i;
    for (i = 0; i < height; i++) {
        fwrite(image + (i*widthInBytes), BYTES_PER_PIXEL, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
}

unsigned char* createBitmapFileHeader (int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static unsigned char fileHeader[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize      );
    fileHeader[ 3] = (unsigned char)(fileSize >>  8);
    fileHeader[ 4] = (unsigned char)(fileSize >> 16);
    fileHeader[ 5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

unsigned char* createBitmapInfoHeader (int height, int width)
{
    static unsigned char infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)(width      );
    infoHeader[ 5] = (unsigned char)(width >>  8);
    infoHeader[ 6] = (unsigned char)(width >> 16);
    infoHeader[ 7] = (unsigned char)(width >> 24);
    infoHeader[ 8] = (unsigned char)(height      );
    infoHeader[ 9] = (unsigned char)(height >>  8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}




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




// Color conversion functions
// ref: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

static hsv   rgb2hsv(rgb in);
static rgb   hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}

// maps a value from one interval to another
double map(double t, double s0, double e0, double s1, double e1)
{
    return (t - s0)/(e0 - s0)*(e1 - s1) + s1;
}

// Converts a linear double value to a color
rgb colormap1(double t) {
    double u, v;
    hsv c;
    c.h = fmod(fmod(t*1000, 360.0) + 360.0, 360.0);
    c.s = 1.0;
    c.v = 0.5;
    return hsv2rgb(c);
}

rgb colormap2(double t) {
    double u, v;
    hsv c;
    c.h = map(sin(t*10), -1, 1, 0+150, 60+150);
    c.s = 1.0;
    c.v = map(sin(t*1), -1, 1, 0, 1);
    return hsv2rgb(c);
}

int dtoi(double d) {
    int i = d * 256;
    if (i < 0) i = 0;
    if (i > 255) i = 255;
    return i;
}

// converts a rgb color to a long
unsigned long _RGB(rgb c)
{
    return dtoi(c.b) + (dtoi(c.g)<<8) + (dtoi(c.r)<<16);
}

int main(int argc, char *argv[])
{
    int save = 0;
    int s = 0;
    for (int itA = 1; itA < argc; itA++) {
        char *arg = argv[itA];
        if (s == 0) {
            if (strcmp(arg, "-s") == 0) save = 1;
            if (strcmp(arg, "-x") == 0) s = 1;
            if (strcmp(arg, "-y") == 0) s = 2;
            if (strcmp(arg, "-i") == 0) s = 3;
            if (strcmp(arg, "-xy") == 0) s = 4;
        }
        else 
        {
            if (s == 1) X_RESN = atoi(arg);
            if (s == 2) Y_RESN = atoi(arg);
            if (s == 3) MAX_ITER = atoi(arg);
            if (s == 4) X_RESN = Y_RESN = atoi(arg);
            s = 0;
        }
    }
    if (s != 0) {
        fprintf(stderr, "Missing argument value!");
        exit(1);
    }
    // printf("-s %d -x %d -y %d -i %d", save, X_RESN, Y_RESN, MAX_ITER);

    struct timespec vartime = timer_start();

    /* Mandelbrot variables */
    int *ks;
    ks = (int *)malloc((X_RESN*Y_RESN) * sizeof(int));

    double *ds;
    ds = (double *)malloc((X_RESN*Y_RESN) * sizeof(double));

    /* Calculate and draw points */
    #pragma acc parallel loop \
        copyout(ks[0:X_RESN*Y_RESN], ds[0:X_RESN*Y_RESN])
    for (int it = 0; it < X_RESN*Y_RESN; it++)
    {
        int i = it / Y_RESN;
        int j = it % Y_RESN;

        // mandelbrot set is defined in the region of x = [-2, +2] and y = [-2, +2]
        double u = ((double)i - (X_RESN / 2.0)) / (X_RESN / 4.0);
        double v = ((double)j - (Y_RESN / 2.0)) / (Y_RESN / 4.0);

        Compl z, c, t;

        z.real = z.imag = 0.0;
        c.real = v;
        c.imag = u;

        int k = 0;
        double d = 0.0;

        double lengthsq, temp;
        do
        { /* iterate for pixel color */
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

    {
        int height = Y_RESN;
        int width = X_RESN;
        unsigned char *image = malloc(height*width*BYTES_PER_PIXEL*sizeof(char));
        char *imageFileName = (char *)"mandelbrot.bmp";

        int i, j, k;
        double d;
        for (int it_pixel = 0; it_pixel < (X_RESN * Y_RESN); it_pixel++)
        {
            int i = it_pixel / Y_RESN;
            int j = it_pixel % Y_RESN;

            k = ks[it_pixel];
            d = ds[it_pixel];
            // if (k == MAX_ITER)
            {
                rgb c;
                c.r = 1.0;
                c.g = 0.8;
                c.b = 0;
                unsigned long lc = k == MAX_ITER ? _RGB(colormap2(sin(d))) : _RGB(colormap1(k/(double)MAX_ITER));
                image[(i*width + j)*BYTES_PER_PIXEL + 0] = (unsigned char)(lc >>  0); //blue
                image[(i*width + j)*BYTES_PER_PIXEL + 1] = (unsigned char)(lc >>  8); //green
                image[(i*width + j)*BYTES_PER_PIXEL + 2] = (unsigned char)(lc >> 16); //red
            }
        }

        generateBitmapImage((unsigned char *)image, height, width, imageFileName);

        free(image);
    }

    free(ks);
    free(ds);

    long long time_elapsed_nanos = timer_end(vartime);
    double elapsed = time_elapsed_nanos*0.000000001;
    printf("%lf\n", elapsed);

    /* Program Finished */
    return 0;
}
