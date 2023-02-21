// static task assignment
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
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
    int myid, numprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    struct complex c;
    float real_min = -2.5;
    float real_max = 1.5;
    float imag_min = -2.0;
    float imag_max = 2.0;
    float disp_width = 1200;
    float disp_height = 1200;
    float scale_real = (real_max - real_min) / disp_width;
    float scale_imag = (imag_max - imag_min) / disp_height;

    int pixels[(int)(disp_width * disp_height)];

    int elements_per_proc = (int)((disp_height * disp_width) / numprocs);

    // Create a buffer that will hold a subset of the pixels array
    int *sub_pixels = malloc(sizeof(int) * elements_per_proc);

    // Scatter the pixels elements to all processes
    MPI_Scatter(pixels, elements_per_proc, MPI_INT, sub_pixels,
                elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    int pixel;
    for (pixel = 0; pixel < elements_per_proc; pixel++)
    {
        c.real = real_min + ((float)((int)(pixel + myid * elements_per_proc) % (int)disp_width) * scale_real);
        c.imag = imag_min + ((float)((int)(pixel + myid * elements_per_proc) / (int)disp_width) * scale_imag);
        int color = cal_pixel(c);
        sub_pixels[pixel] = color;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(sub_pixels, elements_per_proc, MPI_INT, pixels, elements_per_proc, MPI_INT, 0,
               MPI_COMM_WORLD);

    if (myid == 0)
    {
        FILE *fptr;
        fptr = fopen("mandelbrot.ppm", "w"); // "w" defines "writing mode"
        /* write to file */
        // this is the header of the ppm file
        fprintf(fptr, "P6\n 1200 1200 255\n"); // width = 800, height = 800

        pixel = 0;
        for (pixel = 0; pixel < disp_height * disp_width; pixel++)
        {
            fprintf(fptr, "%c%c%c", pixels[pixel], pixels[pixel], pixels[pixel]); // rgb
        }
        fclose(fptr);
    }

    MPI_Finalize();

    if (myid == 0)
    {
        clock_t t2 = clock();
        printf("Elapsed time = % 5.2f seconds\n", (float)(t2 - t1) / CLOCKS_PER_SEC);
    }

    return 0;
}
