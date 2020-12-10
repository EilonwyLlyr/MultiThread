#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void Generate_data(int comm_sz, int x_i, int* randNums, int my_rank, MPI_Comm comm);
void Print_data(int comm_sz, int x_i, int* randNums, int* prefixSum, int my_rank, MPI_Comm comm);
void prefix_Sum(int count, int randNums[], int prefixSum[]);

int main(void){
    int x_i = 1;
    int randNums, prefixSum;
    int comm_sz, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);

    Generate_data(comm_sz, x_i, &randNums, my_rank, comm);

    MPI_Scan(&randNums, &prefixSum, x_i, MPI_INTEGER, MPI_SUM, comm);

    Print_data(comm_sz, x_i, &randNums, &prefixSum, my_rank, comm);

    MPI_Finalize();
}

void Generate_data(int comm_sz, int x_i, int *randNums, int my_rank, MPI_Comm comm){
    int* a = NULL;
    if(my_rank == 0){
        time_t t;
        a = malloc(comm_sz * sizeof(int));

        srand((unsigned)time(&t));
        for(int i = 0; i < comm_sz; i++){
            a[i] =  (rand() % 9) + 1;
        }

        MPI_Scatter(a, x_i, MPI_INTEGER, randNums, x_i, MPI_INTEGER, 0, comm);
        free(a);
    }else{
        MPI_Scatter(a, x_i, MPI_INTEGER, randNums, x_i, MPI_INTEGER, 0, comm);
    }
}

void Print_data(int comm_sz, int x_i, int* randNums, int* prefixSum, int my_rank, MPI_Comm comm){
    int* a = NULL;
    int* b = NULL;
    if(my_rank == 0){
        a = malloc(comm_sz * sizeof(int));
        MPI_Gather(randNums, x_i, MPI_INTEGER, a, x_i, MPI_INTEGER, 0, comm);
        printf("Random %d elements:\n [%d", comm_sz, a[0]);
        for(int i = 1; i < comm_sz; i++){
            printf(", %d", a[i]);
        }

        printf("]\n");

        b = malloc(comm_sz * sizeof(int));
        MPI_Gather(prefixSum, x_i, MPI_INTEGER, b, x_i, MPI_INTEGER, 0, comm);
        printf("Prefix elements:\n [%d", b[0]);
        for(int i = 1; i < comm_sz; i++){
            printf(", %d", b[i]);
        }
        printf("]\n");
        free(a);
        free(b);
    }else{
        MPI_Gather(randNums, x_i, MPI_INTEGER, a, x_i, MPI_INTEGER, 0, comm);
        MPI_Gather(prefixSum, x_i, MPI_INTEGER, b, x_i, MPI_INTEGER, 0, comm);
    }
}

void prefix_Sum(int count, int randNums[], int prefixSum[]){
    prefixSum[0] = randNums[0];
    for(int i = 1; i < count; i++){
        prefixSum[i] = prefixSum[i - 1] + randNums[i];
    }
}