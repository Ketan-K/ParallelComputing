#include<stdio.h>
#include<mpi.h>

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	int temp, n, i, rank, size, start, end, P, N = 100, localsum = 0, tag = 1;
	MPI_Status status;
	
	int numbers[100];
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &P);

	n = N/P;

	start = rank * n;
	if(rank == P-1)
		end = N;
	else
		end = start + n;
	
	for(i=start ; i<end ; i++)
		localsum += i;
	
	if(rank != 0)
	{
		MPI_Send(&localsum, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
	}		
	else
	{
		for(i=1 ; i<P ; i++)
		{
			MPI_Recv(&temp, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
			localsum += temp;
		}
		printf("\nSum=%d\n", localsum);
	}

	MPI_Finalize();
	return 0;
}