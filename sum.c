#include<stdio.h>
#include<mpi.h>
int main(int argc,char *argv[])
{
        MPI_Init(&argc,&argv);
	int chunk,s,N,i,size,rank,start,end,lsum=0,localsum=0,tag=0,array[10];
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	N=100;

	
	if(rank==P-1)
	{
		end=N;
	}
	for(i=start;i<end;i++)
	{
		localsum+=i;
	}

	printf("Localsum: %d\n",localsum);
	

	
	MPI_Reduce(&localsum,&lsum,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);

	MPI_Finalize();

}
