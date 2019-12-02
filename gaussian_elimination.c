#include<stdio.h>
#include<mpi.h>
void division(double arr1[],int rank)
{
	int i;
	double pivot=arr1[rank];
	for(i=0;i<4;i++)
	{
		arr1[i]=arr1[i]/pivot;
	}
	printf("\n");
}

int main(int argc,char *agrv[])
{
	int size,rank,i,j,tag,itr,sum;
	double arr1[4],arr2[4],a[4][4]={{8,2,3,4},{7,4,8,5},{8,4,7,5},{5,2,1,6}},B[4][4],x[4];
	double vec[4]={10,20,30,40};
	MPI_Init(&argc,&agrv);
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Scatter(a,4,MPI_DOUBLE,arr1,4,MPI_DOUBLE,0,MPI_COMM_WORLD);

	for(itr = 0;itr<size;itr++)
	{
		if(rank==itr)
		{
			division(arr1,rank);
			for(i=itr+1;i<size;i++)
			{
				MPI_Send(&arr1,4,MPI_DOUBLE,i,1,MPI_COMM_WORLD);
			}
		}
		if(rank>itr)
		{
			MPI_Recv(&arr2,4,MPI_DOUBLE,itr,1,MPI_COMM_WORLD,&status);
			for(i=0;i<4;i++)
			{
				arr1[i]=arr1[i]-arr1[i]*arr2[i];
			}
		}
	}	
	
	MPI_Gather(&arr1,4,MPI_DOUBLE,&B,4,MPI_DOUBLE,0,MPI_COMM_WORLD);	
	if(rank==0)
	{
		for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
			{
				printf("%f\t", B[i][j]);
			}
			printf("\n");
		}
	}

	for(i=size-1;i>-1;i--)
	{
		sum=0;
		if(i==size-1)
		{
			x[i]=vec[i];
		}
		else
		{
			for(j=i+1;j<size;j++)
				sum=sum+arr2[i]*x[i];
			x[i]=vec[i]-sum;
		}
		MPI_Bcast(&x,size,MPI_FLOAT,i,MPI_COMM_WORLD);
	}
	
	if(rank==0)
	{
		for(i=0;i<4;i++)
		{
			printf("X%d=> %f\n",i,x[i]);
		}
	}
	MPI_Finalize();
}
