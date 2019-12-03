#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void readMatrixFromFile(double *mat, int *dims, char *fname)
{
    int i;

    FILE *fp = fopen(fname, "r");

    for(i=0; i<dims[0]*dims[1]; ++i)
        fscanf(fp, "%lf", &mat[i]);
}

void generateRandom(double *mat, int *dims)
{
    int i;
    for(i=0; i<dims[0]*dims[1]; ++i)
        mat[i] = rand()%50 + 1;
}

void displayMatrix(double *mat, int *dims)
{
    int i;
    for(i=0; i<dims[0]*dims[1]; ++i)
    {
        if(i%dims[1] == 0)
            printf("\n");
        printf(" %lf ", mat[i]);
    }
    printf("\n");
}

int get1DIndex(int row, int col, int *dims)
{
    return row*dims[1] + col;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int number_of_processors, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processors);
    MPI_Status status;

    double *A, *subA, *temp;
    int dims[2], subDims[2];

    if(rank == 0)
    {
        //Assumed to be an augmented matrix
        scanf("%d %d", &dims[0], &dims[1]);
        A = malloc(sizeof(double)*dims[0]*dims[1]);
        //readMatrixFromFile(A, n, "/home/siddhant/inputs/A.txt");
        generateRandom(A, dims);
        displayMatrix(A, dims);

        subDims[0] = dims[0]/number_of_processors;
        subDims[1] = dims[1];
    }

    //Broadcast dimensions for matrix
    MPI_Bcast(subDims, 2, MPI_INT, 0, MPI_COMM_WORLD);

    //Get sub-matrix
    subA = malloc(sizeof(double)*subDims[1]);
    temp = malloc(sizeof(double)*subDims[1]);
    //Scatter rows to each processor
    MPI_Scatter(A, subDims[0]*subDims[1], MPI_DOUBLE, subA, subDims[0]*subDims[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int start_row, end_row;
    start_row = rank*subDims[0];
    end_row = start_row + subDims[0] - 1;

    int i, j;
    for(i=0; i<number_of_processors; ++i)
    {
        //Division
        if(rank == i)
        {
            for(j=i+1; j<subDims[1]; ++j)
                temp[j] = subA[j] /= subA[i];
            temp[i] = subA[i] = 1;
        }
        else
        {}
        MPI_Bcast(temp, subDims[1], MPI_DOUBLE, i, MPI_COMM_WORLD);

        //Elimination
        if(rank > i)
        {
            for(j=rank+1; j<subDims[1]; ++j)
                subA[j] -= temp[j]*subA[rank];
            subA[i] = 0;
        }
    }

    MPI_Gather(subA, subDims[1], MPI_DOUBLE, A, subDims[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if(rank == 0)
        displayMatrix(A, dims);

    MPI_Finalize();
}