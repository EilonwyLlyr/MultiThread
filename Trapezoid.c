#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void Read_a_b_n(double* a_p, double* b_p, int* n_p, int* local_n_p, double* h_p, int my_rank, int comm_sz, MPI_Comm comm);
double fofx(double x);
double integral(double a, int n, double h, int my_rank, MPI_Comm comm);

int main(void){
    double a, b, h, area;
    int n, local_n;
    int comm_sz, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    //Read data: lowerbound, upperbound, number of subintervals
    Read_a_b_n(&a, &b, &n, &local_n, &h, my_rank, comm_sz, comm);
    
    //Compute estimate area of integral
    area = integral(a, local_n, h, my_rank, comm);

    //process 0 will display area
    if(my_rank == 0)
        printf("The estimate area of x^2 from %lf to %lf is: %lf\n", a, b, area);

    MPI_Finalize();
    return 0;
}

void Read_a_b_n(double* a_p, double* b_p, int* n_p, int* local_n_p, double* h_p, int my_rank, int comm_sz, MPI_Comm comm){
    if(my_rank == 0){
        printf("The function is: x*x\n");

        printf("What is the lower bound: \n");
        scanf("%lf", a_p);

        printf("What is the upper bound: \n");
        scanf("%lf", b_p);

        printf("What is the number of subintervals: \n");
        scanf("%d", n_p);
        *local_n_p = *n_p / comm_sz;

        *h_p = (*b_p - *a_p) / *n_p; 
    }
    MPI_Bcast(a_p, 1, MPI_DOUBLE, 0, comm);
    MPI_Bcast(b_p, 1, MPI_DOUBLE, 0, comm);
    MPI_Bcast(local_n_p, 1, MPI_INTEGER, 0, comm);
    MPI_Bcast(h_p, 1, MPI_DOUBLE, 0, comm);
}

double fofx(double x){
    return x*x;
}

double integral(double a, int n, double h, int my_rank, MPI_Comm comm){
    double local_area = 0;
    double area;
    double local_a = a + my_rank * n *h;
    for(int i = 0; i < n; i ++){
        local_area += (h/2) * ( fofx(local_a +  i * h) + fofx(local_a + (i + 1) * h));
    }

    printf("Estimate area for proccessor %d is: %lf\n", my_rank, local_area);

    MPI_Reduce(&local_area, &area, 1, MPI_DOUBLE, MPI_SUM, 0, comm);

    return area;
}