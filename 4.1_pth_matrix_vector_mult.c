#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>


const int MAX_THREADS = 1024;

long thread_count;
int m, n;
int** matrix;
int* vector;
int* mult;
pthread_mutex_t mutex;

void* Matrix_vector_mult(void* rank);

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void Random_matrix_vector(int** matrix, int* vector, int m, int n);
void Print_Mat_vect_mult(int** matrix, int* vector, int* mult, int m, int n);
void Setup_vector(int* vector, int n);

int main(int argc, char* argv[]) {
    long       thread;
    pthread_t* thread_handles;

    Get_args(argc, argv);
    vector = malloc(n * sizeof(int));
    mult = malloc(m * sizeof(int));
    matrix = malloc(m * sizeof(int*));
    for(int i = 0; i < m; i++)
        matrix[i] = malloc(n * sizeof(int));

    thread_handles = malloc(thread_count * sizeof(pthread_t));
    Random_matrix_vector(matrix, vector, m, n);
    Setup_vector(mult, m);

    for(thread = 0; thread < thread_count; thread++)
        pthread_create(thread_handles+thread, NULL, Matrix_vector_mult, (void*) thread);

    for(thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    Print_Mat_vect_mult(matrix, vector, mult, m, n);


    free(thread_handles);
    free(matrix);
    free(vector);
    free(mult);
    return 0;
}

void* Matrix_vector_mult(void* rank) {
    long my_rank = (long) rank;
    int i, j;
    int local_n = n/thread_count;
    int my_first_index = my_rank * local_n;
    int my_last_index = my_first_index + local_n;
    int* local_y = malloc(m * sizeof(int));
    Setup_vector(local_y, m);

    for(i = 0; i < m; i++){
        for(j = my_first_index; j < my_last_index; j++)
            local_y[i] += matrix[i][j] * vector[j];
    }

    pthread_mutex_lock(&mutex);
    for(i = 0; i < m; i++)
        mult[i] += local_y[i];
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void Get_args(int argc, char* argv[]) {
    if (argc != 4)
        Usage(argv[0]);
    thread_count = strtol(argv[1], NULL, 10);
    m = strtol(argv[2], NULL, 10);
    n = strtol(argv[3], NULL, 10);
    if(m < 1 || n < 1)
        Usage(argv[0]);
    if (thread_count <= 0 || thread_count > MAX_THREADS) 
        Usage(argv[0]);
}

void Usage(char* prog_name) {
    fprintf(stderr, "usage: %s <number of threads> <m> <n>\n", prog_name);
    fprintf(stderr, "   m is the number of rows within the matrix and should be >= 1\n");
    fprintf(stderr, "   n is the number of columns within the matrix and should be >= 1\n");
    fprintf(stderr, "   n should be divisible by the number of threads\n");
    exit(0);
}

void Random_matrix_vector(int** matrix, int* vector, int m, int n){
    srand(time(0));
    for(int i = 0; i < m; i++){
        for(int j = 0; j < n; j++){
            matrix[i][j] = rand() % 10;
        }
    }

    for(int i = 0; i < n; i++)
        vector[i] = rand() % 10;
}

void Print_Mat_vect_mult(int** matrix, int* vector, int* mult, int m, int n){
    printf("Matrix:\n");
    for(int i = 0; i < m; i++){
        printf("|");
        for(int j = 0; j < n; j++){
            printf(" %d ", matrix[i][j]);
        }
        printf("|\n");
    }
    printf("Vector:\n");
    printf("[ %d", vector[0]);
    for(int i = 1; i < n; i++)
        printf(", %d", vector[i]);
    printf(" ]^T\n");

    printf("Matrix mult Vector:\n");
    printf("[ %d", mult[0]);
    for(int i = 1; i < m; i++)
        printf(", %d", mult[i]);
    printf(" ]^T\n");
}

void Setup_vector(int* vector, int k){
    for(int i =0; i < k; i++)
        vector[i] = 0;
}