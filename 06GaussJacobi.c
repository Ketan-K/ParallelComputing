#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

//Assumption : diagonally dominant matrix

int check(float *X_old, float *X_new, int n)
{
    float threshold = 0.00001;
    float norm, sum=0;

    int i;
    for(i=0; i<n; ++i)
    {
        sum += powf((X_new[i] - X_old[i]), 2);
    }
    norm = sqrt(sum);

    if(norm < threshold)
        return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int number_of_processors, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processors);
    MPI_Status status;

    //Matrices
    int A[][3] = {{4, -1, 1}, {4, -8, 1}, {-2, 1, 5}};
    int B[] = {7, -21, 15};
    float X_old[] = {0, 0, 0};
    float X_new[] = {0, 0, 0};

    //Distribute data
    int i, b, *a = malloc(sizeof(int)*number_of_processors);

    MPI_Bcast(X_old, number_of_processors, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Scatter(B, 1, MPI_INT, &b, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(A, number_of_processors, MPI_INT, a, number_of_processors, MPI_INT, 0, MPI_COMM_WORLD);

    int flag=0;
    float temp;
    while(!flag)
    {
        temp = 0;
        for(i=0; i<number_of_processors; ++i)
            if(i != rank)
                temp += a[i]*X_old[i];

        temp = (b - temp)/a[rank];
 
        MPI_Allgather(&temp, 1, MPI_FLOAT, X_new, 1, MPI_FLOAT, MPI_COMM_WORLD);

        if(rank == 0)
            flag = check(X_old, X_new, number_of_processors);

        MPI_Bcast(&flag, 1, MPI_INT, 0, MPI_COMM_WORLD);

        for(i=0; i<number_of_processors; ++i)
            X_old[i] = X_new[i];
    }

    if(rank == 0)
    {
        for(i=0; i<number_of_processors; ++i)
            printf(" %d ", X_new[i]);
        printf("\n");
    }
    
    MPI_Finalize();
}