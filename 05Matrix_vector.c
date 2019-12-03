#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void display_matrix(int **main_matrix, int r, int c)
{
    int i, j;
    for(i=0; i<r; ++i)
    {
        for(j=0; j<c; ++j)
            printf(" %d ", main_matrix[i][j]);
        printf("\n");
    }
    printf("\n");
}

void display_vector(int *vector, int n)
{
    int i;
    for(i=0; i<n; ++i)
        printf("%d\n", vector[i]);
    printf("\n");
}

//Convert 2D array to 1D array
void get1D(int *buff, int **matrix, int rows, int cols)
{
    int i, j, k=0;
    for(i=0; i<rows; ++i)
    {
        for(j=0; j<cols; ++j)
        {
            buff[k++] = matrix[i][j];
        }
    }
}

void get2D(int *arr, int **matrix, int rows, int cols)
{
    int i, j, k=0;
    for(i=0; i<rows; ++i)
    {
        for(j=0; j<cols; ++j)
        {
            matrix[i][j] = arr[k++];
        }
    }
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

    int number_of_processors, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processors);

    MPI_Status status;

    int n, **main_matrix, *vector;
    if(rank == 0)
    {
        scanf("%d", &n);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int i, j;

    int row_counts[number_of_processors];

    //Allocate memory
    vector = malloc(sizeof(int)*n);
    if(rank == 0)
    {
        //generate random matrix & vector
        main_matrix = malloc(sizeof(int*)*n);
        for(i=0; i<n; ++i)
        {
            main_matrix[i] = malloc(sizeof(int)*n);
            for(j=0; j<n; ++j)
            {
                main_matrix[i][j] = random()%10;
            }
            vector[i] = random()%10;
        }

        printf("Matrix : \n");
        display_matrix(main_matrix, n, n);
        
        printf("Vector : \n");
        display_vector(vector, n);
        //get division of data
        if(n >= number_of_processors)
        {
            if(n%number_of_processors == 0)
            {
                for(i=0; i<number_of_processors; ++i)
                    row_counts[i] = n/number_of_processors;
            }

            else
            {
                for(i=0; i<number_of_processors; ++i)
                    row_counts[i] = n/number_of_processors;
                int extra = n%number_of_processors;
                for(i=0; i<number_of_processors && extra; ++i)
                {
                    row_counts[i]++;
                    extra--;
                }
            }
        }

        for(i=0; i<number_of_processors; ++i)
            MPI_Send(&row_counts[i], 1, MPI_INT, i, 1, MPI_COMM_WORLD);

        //printf("Division of row_counts :\n");
        //display_vector(row_counts, number_of_processors);
    }

    MPI_Bcast(vector, n, MPI_INT, 0, MPI_COMM_WORLD);

    int rows;
    MPI_Recv(&rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    //printf("%d : %d\n", rank, rows);

    if(rank == 0)
    {
        int *send_buff = malloc(sizeof(int) * n * n);
        get1D(send_buff, main_matrix, n, n);

        int send_count, temp=0;
        for(i=0; i<number_of_processors; ++i)
        {
            send_count = row_counts[i]*n;
            MPI_Send(&send_buff[temp], send_count, MPI_INT, i, 2, MPI_COMM_WORLD);
            temp += send_count;
        }

        free(send_buff);
    }

    int recv_count = rows * n;
    int *recv_buff = malloc(sizeof(int)*recv_count);
    MPI_Recv(recv_buff, recv_count, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

    //Create a matrix for receiving
    int **matrix = malloc(sizeof(int*)*rows);
    for(i=0; i<rows; ++i)
        matrix[i] = malloc(sizeof(int)*n);

    get2D(recv_buff, matrix, rows, n);
    free(recv_buff);

    //Compute multiplication
    int sum;
    int *sub_result = malloc(sizeof(int)*rows);

    for(i=0; i<rows; ++i)
    {
        sum = 0;
        for(j=0; j<n; ++j)
        {
            sum += matrix[i][j] * vector[j];
        }
        sub_result[i] = sum;
    }

    //Gather results
    int *result = malloc(sizeof(int) * n);
    int displacements[number_of_processors];

    displacements[0] = 0;
    for(i=1; i<number_of_processors; ++i)
        displacements[i] = displacements[i-1] + row_counts[i-1];

    MPI_Gatherv(sub_result, rows, MPI_INT, result, row_counts, displacements, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank == 0)
    {
        printf("Result : \n");
        display_vector(result, n);
    }

	MPI_Finalize();
}