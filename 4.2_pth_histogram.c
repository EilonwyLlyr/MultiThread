#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

const int MAX_THREADS = 1024;

long thread_count;
double* measurements;
int* binsize;
long long n;
double a;
double b;
double width;
sem_t sem;

void* Histogram(void* rank);

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void Generate_data(double* measurements);

int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;

    Get_args(argc, argv);
    sem_init(&sem, 0, 1);
    thread_handles = malloc(thread_count * sizeof(pthread_t));
    measurements = malloc(n * sizeof(double));
    binsize = malloc(thread_count * sizeof(int));
    for(int i = 0; i < thread_count; i++)
        binsize[i] = 0;

    Generate_data(measurements);

    printf("All Values: \n[");
        for(int i = 0; i < n; i++)
            printf(" %lf ", measurements[i]);
        printf("]\n");

    for(thread = 0; thread < thread_count; thread++)
        pthread_create(thread_handles + thread, NULL, Histogram, (void*) thread);
    
    for(thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);


    free(thread_handles);
    free(measurements);
    free(binsize);
    return 0;
}

void* Histogram(void* rank){
    long my_rank = (long) rank;
    double* my_bin = malloc(binsize[my_rank] * sizeof(double));
    double lower = a + my_rank * width;
    double upper = a + (my_rank + 1) * width;
    int j = 0;

    for(int i = 0; i < n; i++){
        if(measurements[i] >= lower && measurements[i] < upper){
            my_bin[j] = measurements[i];
            j++;
        }
    }

    sem_wait(&sem);
        printf("\nBin %ld: \n", my_rank);
        printf("Range: [%lf, %lf)\n", lower, upper);
        printf("Number of measurements: %d\n", binsize[my_rank]);
        printf("Values: \n[");
        for(int i = 0; i < binsize[my_rank]; i++)
            printf(" %lf ", my_bin[i]);
        printf("]\n");
    sem_post(&sem);

    return NULL;
}

void Get_args(int argc, char* argv[]){
    if(argc != 5)
        Usage(argv[0]);
    thread_count = strtol(argv[1], NULL, 10);
    if(thread_count <= 0 || thread_count > MAX_THREADS)
        Usage(argv[0]);
    n = strtol(argv[2], NULL, 10);
    a = strtod(argv[3], NULL);
    b = strtod(argv[4], NULL);
    width = (b - a) / thread_count;
}

void Usage(char* prog_name){
    fprintf(stderr, "usage: %s <number of threads> <n> <a> <b>\n", prog_name);
    fprintf(stderr, "       n is the number of terms and should be >= 1\n");
    fprintf(stderr, "       a is the lower bound of the measurements\n");
    fprintf(stderr, "       b is the upper bound of the measurements\n");
    exit(0);
}

void Generate_data(double* measurements){
    srand(time(0));
    for(int i = 0; i < n; i++){
        measurements[i] = rand()/(double)RAND_MAX * (b - a) + a;
        int bin = (int)((measurements[i] - a)/width);
        binsize[bin]++;
    }
}