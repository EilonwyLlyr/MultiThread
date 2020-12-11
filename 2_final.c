#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <String.h>
#include <pthread.h>

const int MAX_THREADS = 1024;

long m;
long long n;
double x;
double sum = 0;

pthread_mutex_t mutex;

void* Thread_sum(void* rank);

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
double factorial(long long i);


int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;
    
    Get_args(argc, argv);
    thread_handles = malloc(m * sizeof(pthread_t));

    for(thread = 0; thread < m; thread++)
        pthread_create(thread_handles + thread, NULL, Thread_sum, (void*) thread);

    for(thread = 0; thread < m; thread++)
        pthread_join(thread_handles[thread], NULL);

    printf("sin %lf: %lf", x, sum);
    free(thread_handles);
    return 0;
}

void* Thread_sum(void* rank){
    long my_rank = (long) rank;
    long long my_n = n/m;
    long long my_first_i = my_n * my_rank;
    long long my_last_i = my_first_i + my_n;
    double factor;
    double local_sum = 0;

    if(my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;

    for(long long i = my_first_i; i < my_last_i; i++, factor = -factor){
        local_sum += (factor/factorial(2*i + 1)) * pow(x, 2*i + 1);
    }

    pthread_mutex_lock(&mutex);
    sum += local_sum;
    pthread_mutex_unlock(&mutex);
}

double factorial(long long x){
    long long fac = 1;
    for(int i = 1; i <= x; i++)
        fac *= i;
    return fac;
}

void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads> <n> <x>\n", prog_name);
   fprintf(stderr, "   n is the number of terms and should be >= 1\n");
   fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
   exit(0);
}

void Get_args(int argc, char* argv[]){
    if (argc != 4)
        Usage(argv[0]);
    m = strtol(argv[1], NULL, 10);  
    if (m <= 0 || m > MAX_THREADS)
        Usage(argv[0]);
    n = strtoll(argv[2], NULL, 10);
    if (n <= 0)
        Usage(argv[0]);
    x = strtod(argv[3], NULL);
}