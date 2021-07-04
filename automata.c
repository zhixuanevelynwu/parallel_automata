/* 
    A parallel implementation of automata(game of life)
    Author: Zhixuan Wu
    Date: 06/04/2021
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <omp.h>

//gcc -Wall -std=c99 -fopenmp automata.c
int main(int argc, char *argv[])
{

    double t1, t2;

    if (argc < 5)
    {
        printf("wrong usage\n");
        exit(0);
    }

    int X;            /* # GENERATION */
    int Y;            /* SIZE OF MAP */
    int thread_count; /* # THREADS */

    char filename[128]; /* INPUT FILENAME */
    FILE *fp1, *fp2;

    X = atoi(argv[1]);
    Y = atoi(argv[2]);
    thread_count = atoi(argv[3]);

    int map[Y][Y];     /* INPUT MAP */
    int map_out[Y][Y]; /* OUTPUT MAP */

    fp1 = fopen(argv[4], "r"); // read input file
    strcpy(filename, argv[4]);
    strcat(filename, ".out");
    printf("%s\n", filename);
    fp2 = fopen(filename, "w"); // create and write to filename.out file
    // error checking
    if ((fp1 == NULL) || (fp2 == NULL))
    {
        perror("fopen");
        exit(1);
    }

    /* GET 2D ARRAY FROM INPUT FILE */
    int c = 0;
    int i = 0, j = 0;
    while (!feof(fp1))
    {
        fscanf(fp1, "%d", &c);
        map[j][i++] = c;
        if (i == Y)
        {
            j++;
            i = 0;
        }
    }

    /* ACTUAL CELLULAR AUTOMATON */
    t1 = omp_get_wtime();

    int gen;
    for (gen = 0; gen < X; gen++)
    { // LOOP FOR GENERATIONS

        // LOOP THORUGH THE MAP AND CALCULATE EACH CELL
        //# pragma omp for schedule(dynamic)
        //printf("n_thread: %d \n", omp_get_num_threads());
        //t1 = omp_get_wtime();
        for (i = 0; i < Y; i++)
#pragma omp parallel num_threads(thread_count)
        {
//# pragma omp for schedule(static, 10)
#pragma omp for schedule(static)
            for (j = 0; j < Y; j++)
            {
                //printf("%d> i = %d; j = %d \n", omp_get_thread_num(), i, j);
                /*  check neighbors of each cell
                        general case(not corner ones)
                            ->  they have 8 neighbors
                                map[i-1][j-1]  map[i-1][j]  map[i-1][j+1]
                                map[i][j-1]    [*ITSELF*]   map[i][j+1]
                                map[i+1][j-1]  map[i+1][j]  map[i+1][j+1]
                                ->  count which ones of these are alive
                                    if count == 2, no change
                                    if count == 3, the cell becomes alive in the next gen regardless of its present state
                                    if count != 2 && count != 3, it dies
                */
                int cell;
                if (gen % 2 == 0)
                    cell = map[i][j];
                else
                    cell = map_out[i][j];

                int count = 0;

                /* COUNT # ALIVE NEIGHBOURS */
                if (i - 1 >= 0)
                {
                    if (gen % 2 == 0)
                    {
                        count += map[i - 1][j];
                        if (j - 1 >= 0)
                            count += map[i - 1][j - 1];
                        if (j + 1 < Y)
                            count += map[i - 1][j + 1];
                    }
                    else
                    {
                        count += map_out[i - 1][j];
                        if (j - 1 >= 0)
                            count += map_out[i - 1][j - 1];
                        if (j + 1 < Y)
                            count += map_out[i - 1][j + 1];
                    }
                }

                if (j - 1 >= 0)
                {
                    if (gen % 2 == 0)
                        count += map[i][j - 1];
                    else
                        count += map_out[i][j - 1];
                }

                if (j + 1 < Y)
                {
                    if (gen % 2 == 0)
                        count += map[i][j + 1];
                    else
                        count += map_out[i][j + 1];
                }

                if (i + 1 < Y)
                {
                    if (gen % 2 == 0)
                    {
                        count += map[i + 1][j];
                        if (j - 1 >= 0)
                            count += map[i + 1][j - 1];
                        if (j + 1 < Y)
                            count += map[i + 1][j + 1];
                    }
                    else
                    {
                        count += map_out[i + 1][j];
                        if (j - 1 >= 0)
                            count += map_out[i + 1][j - 1];
                        if (j + 1 < Y)
                            count += map_out[i + 1][j + 1];
                    }
                }

                /* DETERMINE CELL'S STATE FOR THE NEXT GEN */
                if (count == 3)
                {
                    cell = 1;
                }

                else if (count != 3 && count != 2)
                {
                    // dies
                    cell = 0;
                }

                if (gen % 2 == 0)
                    map_out[i][j] = cell;
                else
                    map[i][j] = cell;

                //sleep(1);
            }
        }
    }

    t2 = omp_get_wtime();
    printf("CPU time used: %.2f - %.2f = %.2f sec \n", t2, t1, t2 - t1);

    if (X % 2 != 0)
    {
        for (i = 0; i < Y; i++)
        {
            for (j = 0; j < Y; j++)
            {
                fprintf(fp2, "%d ", map_out[i][j]);
            }
            fprintf(fp2, "\n");
        }
    }
    else
    {
        for (i = 0; i < Y; i++)
        {
            for (j = 0; j < Y; j++)
            {
                fprintf(fp2, "%d ", map[i][j]);
            }
            fprintf(fp2, "\n");
        }
    }

    fclose(fp1);
    fclose(fp2);

    return EXIT_SUCCESS;
}