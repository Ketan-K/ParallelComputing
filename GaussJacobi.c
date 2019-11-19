#include<stdio.h>
#include<mpi.h>
#define row 3
#define col 3
int main(int argc,char *argv[])
{
	float A[row][col]={5,1,2,1,4,2,2,1,5},B[row]={1,2,3},Xt[col],Xold[row]={1,1,1},Xnew[row],myRowOfA[col],bi,SumAX=0;
	int size,rank,root=0;
	
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	MPI_Scatter(&A,col,MPI_FLOAT,&myRowOfA,col,MPI_FLOAT,root,MPI_COMM_WORLD);
	MPI_Scatter(&B,1,MPI_FLOAT,&bi,1,MPI_FLOAT,root,MPI_COMM_WORLD);
	MPI_Bcast(&Xold,row,MPI_FLOAT,root,MPI_COMM_WORLD);
	/*printf("\nMy Rank : %d",rank);
	for(int i=0;i<col;i++)
	{
		printf("\n\tRank : %d | Col : %d | Data : %.2f ",rank,i,myRowOfA[i]);
	}*/
	SumAX=0;
	for(int i=0;i<col;i++)
		//if(i!=rank)
		SumAX+=myRowOfA[i]*Xold[i];
		//printf("\n\tRank : %d | Sum : %.2f | bi : %.2f ",rank,SumAX,bi);
	Xt[rank]=(bi-SumAX)/myRowOfA[rank];
	printf("\n Xnew[ %d ] : %f ",rank,Xt[rank]);
	MPI_Allgather(&Xt,col,MPI_FLOAT,)

	MPI_Finalize();

}
