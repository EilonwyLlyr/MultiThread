#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void Read_order(int* order, int* local_order, int my_rank, int comm_sz, MPI_Comm comm);
void Read_vectors(int order, int local_order, double local_x[], double local_y[], int comm_sz, int my_rank, MPI_Comm comm, MPI_Datatype array_double_type);
void Print_vectors(int order, int local_order, double local_x[], double local_y[], double local_x_y[], int comm_sz, int my_rank, MPI_Comm comm, MPI_Datatype array_double_type);
void Add_vectors(int local_order, double local_x[], double local_y[], double local_x_y[]);

int main(void){
    int order, local_order;
    double *local_x = NULL, *local_y = NULL, *local_x_y = NULL;
    int comm_sz, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);

    Read_order(&order, &local_order, my_rank, comm_sz, comm);
    local_x = malloc(local_order * sizeof(double));
    local_y = malloc(local_order * sizeof(double));
    local_x_y = malloc(local_order * sizeof(double));

    MPI_Datatype array_double_type;
    MPI_Type_contiguous(local_order, MPI_DOUBLE, &array_double_type);
    MPI_Type_commit(&array_double_type);

    Read_vectors(order, local_order, local_x, local_y, comm_sz, my_rank, comm, array_double_type);
    Add_vectors(local_order, local_x, local_y, local_x_y);
    Print_vectors(order, local_order, local_x, local_y, local_x_y, comm_sz, my_rank, comm, array_double_type);
    
    free(local_x);
    free(local_y);
    free(local_x_y);
    MPI_Finalize();
    return 0;
}

void Read_order(int* order, int* local_order, int my_rank, int comm_sz, MPI_Comm comm){
    if(my_rank == 0){
        printf("Enter the order for the two vectors:\n");
        scanf("%d", order);
        *local_order = *order/comm_sz;
    }
    MPI_Bcast(local_order, 1, MPI_INTEGER, 0, comm);
}

void Read_vectors(int order, int local_order, double local_x[], double local_y[], int comm_sz, int my_rank, MPI_Comm comm, MPI_Datatype array_double_type){
    double* x = NULL;
    double* y = NULL;

    if(my_rank == 0){
        x = malloc(order * sizeof(double));
        y = malloc(order * sizeof(double));
    
        printf("Enter %d numbers for vector x\n", order);
        for(int i = 0; i < order; i++)
            scanf("%lf", &x[i]);

        printf("Enter %d numbesr for vector y\n", order);
        for(int i = 0; i < order; i++)
            scanf("%lf", &y[i]);

        MPI_Scatter(x, 1, array_double_type, local_x, 1, array_double_type, 0, comm);
        MPI_Scatter(y, 1, array_double_type, local_y, 1, array_double_type, 0, comm);
        free(x);
        free(y); 
    }else{
        MPI_Scatter(x, 1, array_double_type, local_x, 1, array_double_type, 0, comm);
        MPI_Scatter(y, 1, array_double_type, local_y, 1, array_double_type, 0, comm);
    }
}

void Print_vectors(int order, int local_order, double local_x[], double local_y[], double local_x_y[], int comm_sz, int my_rank, MPI_Comm comm, MPI_Datatype array_double_type){
    double* x = NULL;
    double* y = NULL;
    double* x_y = NULL;
    if(my_rank == 0){
        x = malloc(order * sizeof(double));
        MPI_Gather(local_x, 1, array_double_type, x, 1, array_double_type, 0, comm);
        
        printf("Vector x:\n");
        printf("[%lf", x[0]);
        for(int i = 1; i < order; i++)
            printf(", %lf", x[i]);
        printf("]\n");

        y = malloc(order * sizeof(double));
        MPI_Gather(local_y, 1, array_double_type, y, 1, array_double_type, 0, comm);

        printf("Vector y:\n");
        printf("[%lf", y[0]);
        for(int i = 1; i < order; i++)
            printf(", %lf", y[i]);
        printf("]\n");

        x_y = malloc(order * sizeof(double));
        MPI_Gather(local_x_y, 1, array_double_type, x_y, 1, array_double_type, 0, comm);
        
        printf("Vector x + y:\n");
        printf("[%lf", x_y[0]);
        for(int i = 1; i < order; i++)
            printf(", %lf", x_y[i]);
        printf("]\n");

        free(x);
        free(y);
        free(x_y);
    }else{
        MPI_Gather(local_x, 1, array_double_type, x, 1, array_double_type, 0, comm);
        MPI_Gather(local_y, 1, array_double_type, y, 1, array_double_type, 0, comm);
        MPI_Gather(local_x_y, 1, array_double_type, x_y, 1, array_double_type, 0, comm);
    }
}

void Add_vectors(int local_order, double local_x[], double local_y[], double local_x_y[]){
    for(int i = 0; i < local_order; i++)
        local_x_y[i] = local_x[i] + local_y[i];
}