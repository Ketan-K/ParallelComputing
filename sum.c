#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
int main(int argc,char *argv[])
{
        MPI_Init(&argc,&argv);
	int chunk,s,N,i,size,rank,start,end,lsum[10],tag=0,array[10];
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	N=100;
//	printf("Rank : %d\n Size : %d\n",rank,size);
	chunk=N/size;
	if(rank==0)
	{
		for(i=0;i<10;i++)
			{
				array[i]=rand()%10+1;
			}
	}
	int sndcnt=10/size;
	
	MPI_Scatter(&array,sndcnt,MPI_INT,&lsum,sndcnt,MPI_INT,0,MPI_COMM_WORLD);
		for(i=0;i<sndcnt;i++)
        printf("\tFinal Sum : %d\n ",lsum[i]);

	MPI_Finalize();

}
