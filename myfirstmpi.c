#include<stdio.h>
#include<mpi.h>
int main(int argc,char *argv[])
{
        MPI_Init(&argc,&argv);
	int n,s,N,i,size,rank,start,end,lsum=0,tag=0;
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	N=100;
//	printf("Rank : %d\n Size : %d\n",rank,size);
	n=N/size;
	start=rank*n;
	if( rank==size-1)
		end=N;
	else
	end=start+n;
	for(i=start;i<end;i++)
	{
		lsum+=i;
	}
	//printf("LocalSum : %d ",lsum);
	if(rank!=0)
	MPI_Send(&lsum,1,MPI_INT,0,tag,MPI_COMM_WORLD);
	else
	{
	for(i=1;i<size;++i)
	{
		MPI_Recv(&s,1,MPI_INT,i,tag,MPI_COMM_WORLD,&status);
		lsum+=s;
	}
        printf("\tFinal Sum : %d\n ",lsum);
	}
	MPI_Finalize();

}
