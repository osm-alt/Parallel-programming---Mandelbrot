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

    int data_tag = 1;
    int terminator_tag = 2;
    int result_tag = 3;

    if (myid == 0)
    {
        int count = 0;
        int row = 0;
        int k = 0;

        FILE *fptr;
        fptr = fopen("mandelbrot_dynamic.ppm", "w"); // "w" defines "writing mode"
                                                     /* write to file */
        // this is the header of the ppm file
        fprintf(fptr, "P6\n 1200 1200 255\n"); // width = 800, height = 800

        for (k = 1; k < numprocs; k++)
        {
            MPI_Send(&row, 1, MPI_INT, k, data_tag, MPI_COMM_WORLD);
            row++;
            count++;
        }

        int row_colors[(int)disp_width];

        MPI_Status status;
        do
        {
            MPI_Recv(row_colors, disp_width, MPI_INT, MPI_ANY_SOURCE, result_tag, MPI_COMM_WORLD, &status);
            count--;
            if (row < disp_height)
            {
                MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, data_tag, MPI_COMM_WORLD);
                row++;
                count++;
            }
            else
            {
                MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, terminator_tag, MPI_COMM_WORLD);
            }

            int pixel = 0;
            for (pixel = 0; pixel < disp_width; pixel++)
            {
                fprintf(fptr, "%c%c%c", row_colors[pixel], row_colors[pixel], row_colors[pixel]); // rgb
            }

        } while (count > 0);
        fclose(fptr);
    }
    else
    {
        int y = 0;
        int x = 0;
        MPI_Status status;
        int row_colors[(int)disp_width];
        MPI_Recv(&y, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        while (status.MPI_TAG == data_tag)
        {
            c.imag = imag_min + ((float)y * scale_imag);
            for (x = 0; x < disp_width; x++)
            {
                c.real = real_min + ((float)x * scale_real);
                row_colors[x] = cal_pixel(c);
            }
            MPI_Send(row_colors, disp_width, MPI_INT, 0, result_tag, MPI_COMM_WORLD);
            MPI_Recv(&y, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
    }

    MPI_Finalize();
    if (myid == 0)
    {
        clock_t t2 = clock();
        printf("Elapsed time = % 5.2f seconds\n", (float)(t2 - t1) / CLOCKS_PER_SEC);
    }

    return 0;
}