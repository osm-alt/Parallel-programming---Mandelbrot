#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct complex
{
    float real;
    float imag;
};

int cal_pixel(struct complex c)
{
    int count, max_iter;
    struct complex z;
    float temp, lengthsq;
    max_iter = 256;
    z.real = 0;
    z.imag = 0;
    count = 0; // number of iterations
    do
    {
        temp = z.real * z.real - z.imag * z.imag + c.real;
        z.imag = 2 * z.real * z.imag + c.imag;
        z.real = temp;
        lengthsq = z.real * z.real + z.imag * z.imag;
        count++;
    } while ((lengthsq < 4.0) && (count < max_iter));
    return count;
}

int main(int argc, char *argv[])
{
    clock_t t1 = clock();
    struct complex c;
    float real_min = -2.5;
    float real_max = 1.5;
    float imag_min = -2.0;
    float imag_max = 2.0;
    float disp_width = 1200;
    float disp_height = 1200;
    float scale_real = (real_max - real_min) / disp_width;
    float scale_imag = (imag_max - imag_min) / disp_height;

    FILE *fptr;
    fptr = fopen("mandelbrot_sequential.ppm", "w"); // "w" defines "writing mode"
    /* write to file */
    // this is the header of the ppm file
    fprintf(fptr, "P6\n 1200 1200 255\n"); // width = 800, height = 800

    int x;
    int y;
    for (y = 0; y < disp_height; y++)
    {
        for (x = 0; x < disp_width; x++)
        {
            c.real = real_min + ((float)x * scale_real);
            c.imag = imag_min + ((float)y * scale_imag);
            int color = cal_pixel(c);
            fprintf(fptr, "%c%c%c", color, color, color); // rgb
        }
    }

    fclose(fptr);

    clock_t t2 = clock();
    printf("Elapsed time = % 5.2f seconds\n", (float)(t2 - t1) / CLOCKS_PER_SEC);

    return 0;
}
