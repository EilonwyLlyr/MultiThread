#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <String.h>
#include "timer.h"

const int MAX_THREADS = 1024;

long thread_count;
long long n;
long long *a;
long long *b;
long long *k;
pthread_mutex_t mutex;

void* dist_Enum_sort(void* rank);
void* swap(void* rank);

void Get_args(int argc, char* argv[]);
void print_array(long long * x);
void Usage(char* prog_name);
void dist_Enum_sort_serial();

int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;
    double start, finish, elapsed;

    Get_args(argc, argv);

    a = malloc(n * sizeof(long long));
    b = malloc(n * sizeof(long long));
    k = malloc(n * sizeof(long long));
    memset(k, 0, n);

    for(long long i = n, j = 0; j < n; i--, j++)
        a[j] = i;
    
    
    thread_handles = malloc(thread_count * sizeof(pthread_t));

    GET_TIME(start);
    for(thread = 0; thread < thread_count; thread++)
        pthread_create(thread_handles + thread, NULL, dist_Enum_sort, (void*) thread);

    for(thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    for(thread = 0; thread < thread_count; thread++)
        pthread_create(thread_handles + thread, NULL, swap, (void*) thread);

    for(thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Paralle time: %lf\n", elapsed);

    print_array(a);
    print_array(k);
    print_array(b);

    GET_TIME(start);
    dist_Enum_sort_serial();
    GET_TIME(finish);
    elapsed = finish - start;
    printf("Serial time: %lf\n", elapsed);
    free(a);
    free(b);
    free(k);
    free(thread_handles);
    return 0;
}

void* dist_Enum_sort(void* rank){
    long my_rank = (long) rank;
    long long my_n = n/thread_count;
    long long my_first_i = my_n * my_rank;
    long long my_last_i = my_first_i + my_n;
    long long * local_k = malloc(n * sizeof(long long));
    memset(local_k, 0, n);

    for(long long i = 0; i < n; i++){
        for(long long j = my_first_i; j < my_last_i; j++){
            if(a[i] > a[j])
                local_k[i]++;
        }
    }

    pthread_mutex_lock(&mutex);
    for(long long i = 0; i < n; i++)
        k[i] += local_k[i];
    pthread_mutex_unlock(&mutex);
}

void dist_Enum_sort_serial(){
    for(long long i = 0; i < n; i++){
        long long k = 0;
        for(long long j = 0; j < n; j++){
            if(a[i] > a[j])
                k++;
        }
        b[k] = a[i];
    }
}

void* swap(void* rank){
    long my_rank = (long) rank;
    long long my_n = n/thread_count;
    long long my_first_i = my_n * my_rank;
    long long my_last_i = my_first_i + my_n;

    for(long long i = my_first_i; i < my_last_i; i++)
        b[k[i]] = a[i];
}

void Get_args(int argc, char* argv[]){
    if (argc != 3)
        Usage(argv[0]);
    thread_count = strtol(argv[1], NULL, 10);  
    if (thread_count <= 0 || thread_count > MAX_THREADS)
        Usage(argv[0]);
    n = strtoll(argv[2], NULL, 10);
    if (n <= 0)
        Usage(argv[0]);
}

void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
   fprintf(stderr, "   n is the number of terms and should be >= 1\n");
   fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
   exit(0);
}

void print_array(long long* x){
    for(long long i = 0; i < n; i++)
        printf("%lld ", x[i]);
    printf("\n");
}