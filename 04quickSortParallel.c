#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void display_array(int *array, int length)
{
    int i;
    for(i=0; i < length; ++i)
        printf(" %d ", array[i]);
    printf("\n");
}

int partition(int *arr, int lb, int ub)
{
    int down, up;
    int pivot, temp;

    pivot = arr[lb];
    down = lb;
    up = ub;

    while(down < up)
    {
        while(down <= ub && arr[down] <= pivot) down++;
        while(arr[up] > pivot) up--;

        if(down < up)
        {
            temp = arr[down];
            arr[down] = arr[up];
            arr[up] = temp;
        }
    }

    arr[lb] = arr[up];
    arr[up] = pivot;

    return up;
}

void quickSort(int *arr, int lb, int ub)
{
    int pos;

    if(lb < ub)
    {
        pos = partition(arr, lb, ub);
        quickSort(arr, lb, pos-1);
        quickSort(arr, pos+1, ub);
    }
}

int parallel_partition(int *arr, int length, int pivot)
{
    int ub = length;
    int down, up;
    int temp;

    down = 0;
    up = length-1;

    while(down < up)
    {
        while(down <= ub && arr[down] < pivot) down++;
        while(arr[up] >= pivot && up >= 0) up--;

        if(down < up)
        {
            temp = arr[down];
            arr[down] = arr[up];
            arr[up] = temp;
        }
    }

    if(arr[up] >= pivot)
        return up-1;
    
    return up;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    MPI_Status status;

    int i, array_length, number_of_processors, rank;
    
    array_length = 16;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int main_array[16] = {9, 11, 1, 8, 10, 5, 3, 16, 2, 14, 15, 4, 6, 13, 7, 12};

    /*
    if(rank == 0)
    {
        for(i=0; i<array_length; ++i)
            main_array[i] = random()%100;
    }
    */
    
    int *sub_array, sub_length;
    sub_length = array_length/number_of_processors;
    sub_array = malloc(sizeof(int)*sub_length);

    int pivot;
    
    if(rank == 0)
        pivot = main_array[0];

    //Distribute data among processors
    MPI_Scatter(main_array, sub_length, MPI_INT, sub_array, sub_length, MPI_INT, 0, MPI_COMM_WORLD);

    //STAGE 1
    //Broadcast pivot element
    MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Sort based on pivot
    int mid = parallel_partition(sub_array, sub_length, pivot);    
    //printf("Processor %d, mid = %d\n", rank, mid);
    //display_array(sub_array, sub_length);

    int recv_count, *recv_array;
    int less_count, greater_count, new_length;

    less_count = mid+1;
    greater_count = sub_length - less_count;

    //Send receive based on processor ranks & merge final arrays
    if(rank < number_of_processors/2)
    {
        MPI_Recv(&recv_count, 1, MPI_INT, rank+number_of_processors/2, 1, MPI_COMM_WORLD, &status);
        new_length = recv_count + less_count;
        if(recv_count > 0)
        {
            recv_array = malloc(sizeof(int)*recv_count);
            MPI_Recv(recv_array, recv_count, MPI_INT, rank+number_of_processors/2, 2, MPI_COMM_WORLD, &status);
            //display_array(recv_array, recv_count);
        }

        MPI_Send(&greater_count, 1, MPI_INT, rank+number_of_processors/2, 1, MPI_COMM_WORLD);
        if(greater_count > 0)
            MPI_Send(&sub_array[mid+1], greater_count, MPI_INT, rank+number_of_processors/2, 2, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Send(&less_count, 1, MPI_INT, rank-number_of_processors/2, 1, MPI_COMM_WORLD);
        if(less_count > 0)
            MPI_Send(sub_array, less_count, MPI_INT, rank-number_of_processors/2, 2, MPI_COMM_WORLD);
        
        MPI_Recv(&recv_count, 1, MPI_INT, rank-number_of_processors/2, 1, MPI_COMM_WORLD, &status);
        new_length = recv_count + greater_count;
        if(recv_count > 0)
        {
            recv_array = malloc(sizeof(int)*recv_count);
            MPI_Recv(recv_array, recv_count, MPI_INT, rank-number_of_processors/2, 2, MPI_COMM_WORLD, &status);
            //display_array(recv_array, recv_count);
        }
    }

    //Create a new array
    int *new_array = malloc(sizeof(int)*new_length);
    
    if(rank < number_of_processors/2)
    {
        int j=0;
        for(i=0; i<less_count; ++i)
            new_array[j++] = sub_array[i];
        
        for(i=0; i<recv_count; ++i)
            new_array[j++] = recv_array[i];
    }
    else
    {
        int j=0;
        for(i=less_count; i<sub_length; ++i)
            new_array[j++] = sub_array[i];
        
        for(i=0; i<recv_count; ++i)
            new_array[j++] = recv_array[i];
    }
    //display_array(new_array, new_length);

    //free old memory
    sub_length = 0;
    free(sub_array);
    recv_count = 0;
    free(recv_array);

    
    //STAGE 2
    if(rank == 0)
    {
        pivot = new_array[0];
        MPI_Send(&pivot, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
    }
    else if(rank == 1)
    {
        MPI_Recv(&pivot, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    }

    else if(rank == 2)
    {
        pivot = new_array[0];
        MPI_Send(&pivot, 1, MPI_INT, 3, 1, MPI_COMM_WORLD);
    }
    else if(rank == 3)
    {
        MPI_Recv(&pivot, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, &status);
    }

    mid = parallel_partition(new_array, new_length, pivot);

    less_count = mid+1;
    greater_count = new_length - less_count;

    //Send receive based on processor ranks & merge final arrays
    if(rank == 0)
    {
        MPI_Recv(&recv_count, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
        sub_length = recv_count + less_count;
        if(recv_count > 0)
        {
            recv_array = malloc(sizeof(int)*recv_count);
            MPI_Recv(recv_array, recv_count, MPI_INT, 1, 2, MPI_COMM_WORLD, &status);
        }

        MPI_Send(&greater_count, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
        if(greater_count > 0)
            MPI_Send(&new_array[mid+1], greater_count, MPI_INT, 1, 3, MPI_COMM_WORLD);
    }
    else if(rank == 1)
    {
        MPI_Send(&less_count, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        if(less_count > 0)
            MPI_Send(new_array, less_count, MPI_INT, 0, 2, MPI_COMM_WORLD);
        
        MPI_Recv(&recv_count, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
        sub_length = recv_count + greater_count;
        if(recv_count > 0)
        {
            recv_array = malloc(sizeof(int)*recv_count);
            MPI_Recv(recv_array, recv_count, MPI_INT, 0, 3, MPI_COMM_WORLD, &status);
        }
    }
    
    else if(rank == 2)
    {
        MPI_Recv(&recv_count, 1, MPI_INT, 3, 1, MPI_COMM_WORLD, &status);
        sub_length = recv_count + less_count;
        if(recv_count > 0)
        {
            recv_array = malloc(sizeof(int)*recv_count);
            MPI_Recv(recv_array, recv_count, MPI_INT, 3, 2, MPI_COMM_WORLD, &status);
        }

        MPI_Send(&greater_count, 1, MPI_INT, 3, 2, MPI_COMM_WORLD);
        if(greater_count > 0)
            MPI_Send(&new_array[mid+1], greater_count, MPI_INT, 3, 3, MPI_COMM_WORLD);
    }
    else if(rank == 3)
    {
        MPI_Send(&less_count, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
        if(less_count > 0)
            MPI_Send(new_array, less_count, MPI_INT, 2, 2, MPI_COMM_WORLD);
        
        MPI_Recv(&recv_count, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);
        sub_length = recv_count + greater_count;
        if(recv_count > 0)
        {
            recv_array = malloc(sizeof(int)*recv_count);
            MPI_Recv(recv_array, recv_count, MPI_INT, 2, 3, MPI_COMM_WORLD, &status);
        }
    }

    //Create a new array
    sub_array = malloc(sizeof(int)*sub_length);
    
    if(rank%2 == 0)
    {
        int j=0;
        for(i=0; i<less_count; ++i)
            sub_array[j++] = new_array[i];
        for(i=0; i<recv_count; ++i)
            sub_array[j++] = recv_array[i];
    } 
    else
    {
        int j=0;
        for(i=less_count; i<new_length; ++i)
            sub_array[j++] = new_array[i];
        for(i=0; i<recv_count; ++i)
            sub_array[j++] = recv_array[i];
    }

    //printf("Processor %d\n", rank);
    //display_array(sub_array, sub_length);

    //SERIAL quick sort
    quickSort(sub_array, 0, sub_length-1);

    //GATHER recv counts
    int recv_counts[number_of_processors], displacements[number_of_processors];

    MPI_Allgather(&sub_length, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    displacements[0] = 0;
    for(i=1; i<number_of_processors; ++i)
    {
        displacements[i] = displacements[i-1] + recv_counts[i-1];
    }

    int temp_array[array_length];
    MPI_Gatherv(sub_array, sub_length, MPI_INT, temp_array, recv_counts, displacements, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(rank == 0)
    {
        display_array(main_array, array_length);
        display_array(temp_array, array_length);
    }
    MPI_Finalize();
}

/**Not taken care of
 * 1. Load imbalance
 *
*/