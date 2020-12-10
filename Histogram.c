#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>

void read_data(int* data_count, double* a, double* b, int my_rank);
void generate_data(double* a, double* b, int* size, int data_count, double* measurements[], int my_rank, int comm_sz, MPI_Comm comm);
void print_data(double a, double b, int size, int data_count, double measurements[], int my_rank, int comm_sz, MPI_Comm comm);

int main(void){
    int data_count, size;
    double a, b;
    double* measurements;
    int comm_sz, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);

    read_data(&data_count, &a, &b, my_rank);

    generate_data(&a, &b, &size, data_count, &measurements, my_rank, comm_sz, comm);

    print_data(a, b, size, data_count, measurements, my_rank, comm_sz, comm);

    free(measurements);
    
    MPI_Finalize();
    return 0;
}

void read_data(int* data_count, double* a, double* b, int my_rank){
    if(my_rank == 0){

        printf("Enter the number of measurements:\n");
        scanf("%d", data_count);
        printf("Enter the lower bound of measurements:\n");
        scanf("%lf", a);
        printf("Enter the upper bound of measurements:\n");
        scanf("%lf", b);
    }
}

void generate_data(double* a, double* b, int* size, int data_count, double* measurements[], int my_rank, int comm_sz, MPI_Comm comm){
    double width = (*b - *a) / comm_sz;
    double* subset;
    if(my_rank == 0){
        time_t t;
        int* bin_size = malloc(comm_sz * sizeof(int));
        double* randData = malloc(data_count * sizeof(double));
        memset(bin_size, 0, comm_sz * sizeof(int));

        srand((unsigned) time(&t));

        for(int i = 0; i < data_count; i++){
            randData[i] = rand()/(double)RAND_MAX * (*b - *a) + *a;
            int bin = (int)((randData[i] - *a)/width);
            bin_size[bin]++;

        }

        *size = bin_size[0];

        for(int i = 1; i < comm_sz; i++)
            MPI_Send(&bin_size[i], 1, MPI_INTEGER, i, 0, comm);
        
        for(int i = 0; i < comm_sz; i++){
            subset = malloc(bin_size[i] * sizeof(double));
            int x = 0;
            for(int j = 0; j < data_count; j++){
                int bin = (int)((randData[j] - *a)/width);
                if(bin == i){
                    subset[x] = randData[j];
                    x++;
                }
            }

            if(i == 0)
                *measurements = subset;
            else
                MPI_Send(subset, bin_size[i], MPI_DOUBLE, i, 0, comm);

        }
        free(randData);
        free(bin_size);
    }else{
        MPI_Recv(size, 1, MPI_INTEGER, 0, 0, comm, MPI_STATUS_IGNORE);
        subset = (double*)malloc(*size * sizeof(double));
        MPI_Recv(subset, *size, MPI_DOUBLE, 0, 0, comm, MPI_STATUS_IGNORE);
        *measurements = subset;
    }

}

void print_data(double a, double b, int size, int data_count, double measurements[], int my_rank, int comm_sz, MPI_Comm comm){
    double* allData = NULL;
    if(my_rank == 0){
        allData = malloc(data_count * sizeof(double));
        int* sizeData = malloc(comm_sz * sizeof(int));
        int i;
        for(i = 0; i < size; i++){
            allData[i] = measurements[i]; 
        }
        sizeData[0] = size;
        for(int j = 1; j < comm_sz; j++){
            int sub;
            MPI_Recv(&sub, 1, MPI_INTEGER, j, 0, comm, MPI_STATUS_IGNORE);
            sizeData[j] = sub;
            double* submes = malloc(sub * sizeof(double));
            MPI_Recv(submes, sub, MPI_DOUBLE, j, 0, comm, MPI_STATUS_IGNORE);
            for(int k = 0; k < sub; k++, i++){
                allData[i] = submes[k];
            }
        }
        printf("\nThe measurements are:\n[%lf", allData[0]);
        for(i = 1; i < data_count; i++)
            printf(",%lf", allData[i]);
        printf("]\n");

        double width = (b - a) / comm_sz;
        printf("\nThe range for each bin is: \n");
        double lower = a;
        double upper = a + width;
        printf("Bin %d: [%lf, %lf)\n", my_rank, lower, upper);
        for(i = 1; i < comm_sz; i++){
            int rank;
            MPI_Recv(&rank, 1, MPI_INTEGER, i, 0, comm, MPI_STATUS_IGNORE);
            lower = a + rank * width;
            upper = a + (rank + 1) * width;
            printf("Bin %d: [%lf, %lf)\n", rank, lower, upper);
        }

        printf("\nThe number of measuremnets in each bin is:\n");
        for(i = 0; i < comm_sz; i++){
            printf("Bin %d: %d\n", i, sizeData[i]);
        }
        free(allData);
        free(sizeData);
    }else{
        MPI_Send(&size, 1, MPI_INTEGER, 0, 0, comm);
        MPI_Send(measurements, size, MPI_DOUBLE, 0, 0, comm);
        MPI_Send(&my_rank, 1, MPI_INTEGER, 0, 0, comm);
    }
}