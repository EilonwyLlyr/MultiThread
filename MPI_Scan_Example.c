#include <mpi.h>
#include <stdio.h>

#define WCOMM MPI_COMM_WORLD
int main(int argc, char **argv){
    int my_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(WCOMM, &my_rank);

    int pdf_i[5] = {1, 2, 3, 4, 5};
    int cdf_i[5];

    MPI_Scan(&pdf_i, &cdf_i, 5, MPI_INT, MPI_SUM, WCOMM);
    for(int i = 0; i< 5;i++)
        printf("process %d: cumulative sum = %d\n", my_rank, cdf_i[i]);

    MPI_Finalize();
}
