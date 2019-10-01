#include<mpi.h>
#include<stdio.h>

int main(int argc,char **argv)
{

	float mat[4][4]={4,2,1,5,4,1,2,5,8,5,1,2,8,5,1,2},vec[4]={1,1,1,1},recv_buf[4];

	
	MPI_Init(&argc,&argv);
	int size,rank;

	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	

	
	MPI_Bcast(&vec,4,MPI_FLOAT,0,MPI_COMM_WORLD);
	MPI_Scatter(&mat,4,MPI_FLOAT,&recv_buf,4,MPI_FLOAT,0,MPI_COMM_WORLD);
	
	

	float mult=0,temp[4],final_res[4];

	for(int i=0;i<4;i++)
		mult+=recv_buf[i]*vec[i];

	MPI_Gather(&mult,1,MPI_FLOAT,&final_res,1,MPI_FLOAT,0,MPI_COMM_WORLD);

	if(rank==0){
		
		printf("Multiplication is \n");
		for(int j=0;j<4;j++){
			printf("%f\n",final_res[j]);
		}
		
	}

	MPI_Finalize();
	
}
