#include<stdio.h>
#include<mpi.h>
#include<math.h>
int main(int argc, char *argv[])
{
	int A[3][3]={{5,-1,-1},{-2,5,1},{-1,1,4}};
	int B[3]={3,8,-6};
	float Xnew[3];
	float Xold[3]={0,0,0};
	int rank,size,i,j,n,N=3,localsum,norm,flag=0,cnt=0,a[3],b,chunk;
	float localx,tol=0.001;	
	MPI_Status status;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	
	
	chunk=N/size;
	if(rank==0)
	{
		printf("Matrix :\n");
		for(i=0;i<N;i++)
		{
			printf("\n");
			for(j=0;j<N;j++)
				printf("%d\t",A[i][j]);
		}
		printf("\n");
		printf("Vector : \n");
		for(i=0;i<N;i++)
			printf("%d\n",B[i]);
	}
	
	MPI_Scatter(&A,N,MPI_INT,&a,N,MPI_INT,0,MPI_COMM_WORLD);

	MPI_Scatter(&B,chunk,MPI_INT,&b,chunk,MPI_INT,0,MPI_COMM_WORLD);

	
	do
	{
		norm=0;
		MPI_Bcast(&Xold,N,MPI_INT,0,MPI_COMM_WORLD);
		for(i=0;i<N;i++)
		{
			if(rank!=i)
			{
				localsum=a[i]*Xold[i];
			}
		}
		localx=(b-localsum)/a[rank];

		
		MPI_Allgather(&localx,chunk,MPI_INT,&Xnew,chunk,MPI_INT,MPI_COMM_WORLD);
		

		for(i=0;i<N;i++)
		{
			printf("Xnew[%d] : %f\t",i,Xnew[i]);
		}
		printf("\n");
		
		if(rank == 0)
		{
			for(i=0;i<3;i++)
				norm=norm+(Xnew[i]-Xold[i])*(Xnew[i]-Xold[i]);
			//printf("Norm : %f\n",sqrt(norm));
			if(sqrt(norm) <= tol)
			{
				flag=1;
			}
			else
			{	for(i=0;i<3;i++)
					Xold[i]=Xnew[i];
				cnt++;
			}
		}
		
		MPI_Bcast(&flag,1,MPI_INT,0,MPI_COMM_WORLD);
		
		if(flag==1)
			break;		
	}while(1);
	
	MPI_Finalize();

}
