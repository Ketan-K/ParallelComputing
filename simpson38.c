#include<stdio.h>
#include<mpi.h>
#include<math.h>
float f(float x)
{
	return 2+(sin(2*sqrt(x)));
}
int main(int argc,char *argv[])
{
        MPI_Init(&argc,&argv);
	int n,s,N,i,size,rank;
	float x,fx,lsum=0,a=0,b=1,h,res,val=0;
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
//	printf("Rank : %d\n Size : %d\n",rank,size);
	h=(b-a)/size;
	x=a+(rank*h);
	fx=f(x);
	if(rank==0)
	{
		fx+=f(b);
	}
	else
	{
		if(rank%3==0)
			fx*=2.0;
		else 
			fx*=3.0;
	}
//        printf("\n Xi : %f => f(%f) : %f ",x,x,f(x));
	MPI_Reduce(&fx,&val,1,MPI_FLOAT,MPI_SUM,0,MPI_COMM_WORLD);
	if(rank==0)
	{
	res=val*3.0*h/8.0;
	printf("Value : %f \n",res);
	}
	MPI_Finalize();

}
