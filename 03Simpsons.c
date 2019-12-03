//Simpson's 3/8th rule

#include<stdio.h>
#include<math.h>
#include<mpi.h>

#define FUNCTION(x) (2 + sin(2*sqrt(x)))

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	MPI_Status status;
	
	//variables
	float a=2, b=10; 
	int numberOfIntervals=9, P, rank;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &P);

	float h = (b-a)/numberOfIntervals;
	
	float x = a + rank*h;
	float f_x = FUNCTION(x);
	float y, y_1, y_2, y_3;
	
	//Reduce according to groups
	if(rank == 0 || rank == P)
	{
		MPI_Reduce(&f_x, &y_1, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	else if(rank%3 == 0)
	{
		f_x *= 2;
		MPI_Reduce(&f_x, &y_2, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	else
	{
		f_x *= 3;
		MPI_Reduce(&f_x, &y_3, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	
	if(rank == 0)
	{
		y = y_1 + y_2 + y_3;
		y = 3*h*y/8;
		
		printf("Y = %f\n", y);
	}
	
	MPI_Finalize();
	
	return 0;
}
