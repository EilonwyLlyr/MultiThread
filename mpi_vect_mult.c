#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_sz,
	MPI_Comm comm);
void Check_for_error(int local_ok, char fname[], char message[],
	MPI_Comm comm);
void Read_data(double local_vec1[], double local_vec2[], double* scalar_p,
	int local_n, int my_rank, int comm_sz, MPI_Comm comm);
void Print_vector(double local_vec[], int local_n, int n, char title[],
	int my_rank, MPI_Comm comm);
double Par_dot_product(double local_vec1[], double local_vec2[],
	int local_n, MPI_Comm comm);
void Par_vector_scalar_mult(double local_vec[], double scalar,
	double local_result[], int local_n);
void Fill_array(double a[], int size, int arrayNum, int* validInput);

int main(void) {
	int n, local_n;
	double *local_vec1 = NULL, *local_vec2 = NULL;
	double scalar;
	double *local_scalar_mult1 = NULL, *local_scalar_mult2 = NULL;
	double dot_product;
	int comm_sz, my_rank;
	MPI_Comm comm = MPI_COMM_WORLD;

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Print input data */
	Read_n(&n, &local_n, my_rank, comm_sz, comm);

	local_vec1 = malloc(local_n * sizeof(double));
	local_vec2 = malloc(local_n * sizeof(double));
	local_scalar_mult1 = malloc(local_n * sizeof(double));
	local_scalar_mult2 = malloc(local_n * sizeof(double));

	Read_data(local_vec1, local_vec2, &scalar, local_n, my_rank, comm_sz, comm);

	/* Print results */
	/* Compute and print dot product */
	dot_product = Par_dot_product(local_vec1, local_vec2, local_n, comm);
	if (my_rank == 0) {
		double sum = dot_product;
		for (int i = 1; i < comm_sz; i++) {
			MPI_Recv(&dot_product, 1, MPI_DOUBLE, i, 0, comm, MPI_STATUS_IGNORE);
			sum += dot_product;
		}
		printf("Dot product is: %lf\n", sum);
	}
	else {
		MPI_Send(&dot_product, 1, MPI_DOUBLE, 0, 0, comm);
	}

	/* Compute scalar multiplication and print out result */
	Par_vector_scalar_mult(local_vec1, scalar, local_scalar_mult1, local_n);
	Par_vector_scalar_mult(local_vec2, scalar, local_scalar_mult2, local_n);

	Print_vector(local_scalar_mult1, local_n, n, "First vector scalar multiplication", my_rank, comm);
	Print_vector(local_scalar_mult2, local_n, n, "Second vector scalar multiplication", my_rank, comm);

	free(local_scalar_mult2);
	free(local_scalar_mult1);
	free(local_vec2);
	free(local_vec1);

	MPI_Finalize();
	return 0;
}

/*-------------------------------------------------------------------*/
void Check_for_error(
	int       local_ok   /* in */,
	char      fname[]    /* in */,
	char      message[]  /* in */,
	MPI_Comm  comm       /* in */) {
	int ok;

	MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
	if (ok == 0) {
		int my_rank;
		MPI_Comm_rank(comm, &my_rank);
		if (my_rank == 0) {
			fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname,
				message);
			fflush(stderr);
		}
		MPI_Finalize();
		exit(-1);
	}
}  /* Check_for_error */


/* Get the input of n: size of the vectors, and then calculate local_n according to comm_sz and n */
/* where local_n is the number of elements each process obtains */
/*-------------------------------------------------------------------*/
void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_sz, MPI_Comm comm) {
	if (my_rank == 0) {
		printf("what is the size of the vectors?\n");
		scanf("%d", n_p);
		*local_n_p = *n_p / comm_sz;
	}
	MPI_Bcast(n_p, 1, MPI_INTEGER, 0, comm);
	MPI_Bcast(local_n_p, 1, MPI_INTEGER, 0, comm);

}  /* Read_n */


/* local_vec1 and local_vec2 are the two local vectors of size local_n which the process pertains */
/* process 0 will take the input of the scalar, the two vectors a and b */
/* process 0 will scatter the two vectors a and b across all processes */
/*-------------------------------------------------------------------*/
void Read_data(double local_vec1[], double local_vec2[], double* scalar_p, int local_n, int my_rank, int comm_sz, MPI_Comm comm) {
	double* a = NULL;
	double* b = NULL;
	int i;

	if (my_rank == 0) {
		printf("What is the scalar?\n");
		scanf("%lf", scalar_p);
	}

	MPI_Bcast(scalar_p, 1, MPI_DOUBLE, 0, comm);

	if (my_rank == 0) {
		a = (double *)malloc(local_n * comm_sz * sizeof(double));

		printf("Enter the %d values for the first vector:\n", local_n * comm_sz);
		for (i = 0; i < local_n * comm_sz; i++)
			scanf("%lf", &a[i]);
		MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm);
		free(a);
	}
	else {
		MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm);
	}

	if (my_rank == 0) {
		b = (double *)malloc(local_n * comm_sz * sizeof(double));
		printf("Enter the %d values for the second vector:\n", local_n * comm_sz);
		for (i = 0; i < local_n * comm_sz; i++)
			scanf("%lf", &b[i]);
		MPI_Scatter(b, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm);
		free(b);
	}
	else {
		MPI_Scatter(b, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm);
	}
}  /* Read_data */

/* The print_vector gathers the local vectors from all processes and print the gathered vector */
/*-------------------------------------------------------------------*/
void Print_vector(double local_vec[], int local_n, int n, char title[], int my_rank, MPI_Comm comm) {
	double* a = NULL;
	int i;

	if (my_rank == 0) {
		a = malloc(n * sizeof(double));
		MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
		printf("%s:\n", title);
		printf("[%lf", a[0]);
		for (i = 1; i < n; i++)
			printf(", %lf", a[i]);
		printf("]\n");
		free(a);
	}
	else {
		MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
	}

}  /* Print_vector */


/* This function computes and returns the partial dot product of local_vec1 and local_vec2 */
/*-------------------------------------------------------------------*/
double Par_dot_product(double local_vec1[], double local_vec2[], int local_n, MPI_Comm comm) {
	double sum = 0;
	for (int i = 0; i < local_n; i++) {
		double prod = local_vec1[i] * local_vec2[i];
		sum += prod;
	}
	return sum;
}  /* Par_dot_product */


/* This function gets the vector which is the scalar times local_vec, and put the vector into local_result */
/*-------------------------------------------------------------------*/
void Par_vector_scalar_mult(double local_vec[], double scalar, double local_result[], int local_n) {
	for (int i = 0; i < local_n; i++) {
		local_result[i] = local_vec[i] * scalar;
	}
}  /* Par_vector_scalar_mult */