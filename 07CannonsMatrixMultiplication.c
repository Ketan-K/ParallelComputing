#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void readMatrixFromFile(int *mat, int n, char *fname)
{
    int i;

    FILE *fp = fopen(fname, "r");

    for(i=0; i<n*n; ++i)
        fscanf(fp, "%d", &mat[i]);
}

void generateRandom(int *mat, int n)
{
    int i;
    for(i=0; i<n*n; ++i)
        mat[i] = rand()%10;
}

int get1DIndex(int row, int col, int n)
{
    return row*n + col;
}

void displayMatrix(int *mat, int n)
{
    int i;
    for(i=0; i<n*n; ++i)
    {
        if(i%n == 0)
            printf("\n");
        printf(" %2d ", mat[i]);
    }
    printf("\n");
}

void MatrixMultiplication(int *A, int *B, int *C, int block_size)
{
    int i, j, k, sum;
    int a, b, c;
    for(i=0; i<block_size; ++i)
    {
        for(j=0; j<block_size; ++j)
        {
            c = get1DIndex(i, j, block_size);
            sum = 0;
            for(k=0; k<block_size; ++k)
            {
                a = get1DIndex(i, k, block_size);
                b = get1DIndex(k, j, block_size);
                sum += A[a]*B[b];
            }
            C[c] += sum;
        }
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int number_of_processors, rank, n;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processors);
    MPI_Status status;

    //Accept dimensions
    if(rank == 0)
    {
        scanf("%d", &n);
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Validation
    int temp = sqrt(number_of_processors);
    int rt_p = (int)temp;
    if(temp != rt_p || n%rt_p != 0)
    {
        if(rank == 0)
            printf("Invalid number of processors.\n");
        MPI_Finalize();
        exit(0);
    }

    //New topology
    MPI_Comm MPI_COMM_2D;
    
    int dims[2], periods[2];
    dims[0] = dims[1] = rt_p;
    periods[0] = periods[1] = 1;

    //Create a cartesian topology
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &MPI_COMM_2D);

    //Get rank & coordinates
    int new_rank, coordinates[2];
    MPI_Comm_rank(MPI_COMM_2D, &new_rank);
    MPI_Cart_coords(MPI_COMM_2D, new_rank, 2, coordinates);

    //printf("%d, %d, %d\n", new_rank, coordinates[0], coordinates[1]);

    //Get matrices    
    int *A, *B, *C;
    int block_size;
    //Rearranged matrix for scatter
    int *A1, *B1, *C1;

    int i, j, k, k1, k2;

    block_size = n/rt_p;

    if(new_rank == 0)
    {
        A = malloc(sizeof(int)*n*n);
        B = malloc(sizeof(int)*n*n);
        C = malloc(sizeof(int)*n*n);

        //readMatrixFromFile(A, n, "/home/siddhant/inputs/A.txt");
        //readMatrixFromFile(B, n, "/home/siddhant/inputs/B.txt");
    
        generateRandom(A, n);
        generateRandom(B, n);

        printf("A :");
        displayMatrix(A, n);
        printf("\nB :");
        displayMatrix(B, n);

        A1 = malloc(sizeof(int)*n*n);
        B1 = malloc(sizeof(int)*n*n);
        C1 = malloc(sizeof(int)*n*n);

        //Rearrangement of matrix
        k=0;
        for(k1=0; k1<n; k1+=block_size)
        {
            for(k2=0 ;k2<n; k2+=block_size)
            {
                for(i=k1; i<k1+block_size; ++i)
                {
                    for(j=k2; j<k2+block_size; ++j)
                    {
                        A1[k] = A[i*n + j];
                        B1[k++] = B[i*n + j];
                    }
                }
            }
        }
        //displayMatrix(A1, n);
    }

    //Sub-matrices
    int *subA, *subB, *subC;

    subA = malloc(sizeof(int)*block_size*block_size);
    subB = malloc(sizeof(int)*block_size*block_size);
    subC = malloc(sizeof(int)*block_size*block_size);
    for(i=0; i<block_size*block_size; ++i)
        subC[i] = 0;

    //Scatter data
    MPI_Scatter(A1, block_size*block_size, MPI_INT, subA, block_size*block_size, MPI_INT, 0, MPI_COMM_2D);
    MPI_Scatter(B1, block_size*block_size, MPI_INT, subB, block_size*block_size, MPI_INT, 0, MPI_COMM_2D);

    int rank_source, rank_dest, top_rank, bottom_rank, right_rank, left_rank;

    //Get 1 shift positions for convinience
    MPI_Cart_shift(MPI_COMM_2D, 0, -1, &bottom_rank, &top_rank);
    MPI_Cart_shift(MPI_COMM_2D, 1, -1, &right_rank, &left_rank);

    //GET INITIAL MATRIX STATE
    //For shifting A
    MPI_Cart_shift(MPI_COMM_2D, 1, -coordinates[0], &rank_source, &rank_dest);
    MPI_Sendrecv_replace(subA, block_size*block_size, MPI_INT, rank_dest, 1, rank_source, 1, MPI_COMM_2D, &status);
    //For shifting B
    MPI_Cart_shift(MPI_COMM_2D, 0, -coordinates[1], &rank_source, &rank_dest);
    MPI_Sendrecv_replace(subB, block_size*block_size, MPI_INT, rank_dest, 1, rank_source, 1, MPI_COMM_2D, &status);

    for(i=0; i<rt_p; ++i)
    {
        MatrixMultiplication(subA, subB, subC, block_size);

        //Shift left by 1 for A and up by 1 for B
        MPI_Sendrecv_replace(subA, block_size*block_size, MPI_INT, left_rank, i+2, right_rank, i+2, MPI_COMM_2D, &status);
        MPI_Sendrecv_replace(subB, block_size*block_size, MPI_INT, top_rank, i+2, bottom_rank, i+2, MPI_COMM_2D, &status);
    }

    MPI_Gather(subC, block_size*block_size, MPI_INT, C1, block_size*block_size, MPI_INT, 0, MPI_COMM_2D);

    if(new_rank == 0)
    {
        //Rearrange matrix
        k=0;
        for(k1=0; k1<n; k1+=block_size)
            for(k2=0 ;k2<n; k2+=block_size)
                for(i=k1; i<k1+block_size; ++i)
                    for(j=k2; j<k2+block_size; ++j)
                        C[i*n + j] = C1[k++];

        displayMatrix(C, n);
    }

    MPI_Finalize();
}