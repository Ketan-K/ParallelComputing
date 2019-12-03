//Reduce call 

#include<stdio.h>
#include<mpi.h>
#define N 100

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Status status;
	
	//variables
	int start=0, end=0, numbers[N], localSum=0, i=0, chunk=0, rank=0, P=0, globalSum=0;
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &P);

	chunk = N/P;
	
	for(i=0; i < N; ++i)
		numbers[i] = i+1;
	
	//Set 'start' and 'end' 
	start = rank*chunk;
	if(rank == P-1)
		end = N;
	else
		end = start + chunk;
	
	//Calculate local sum
	for(i=start; i < end; ++i)
		localSum += numbers[i];
	
	//Send data to final processor
	MPI_Reduce(&localSum, &globalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if(rank == 0)
		printf("Sum = %d\n", globalSum);
	
	MPI_Finalize();
	return 0;
}
