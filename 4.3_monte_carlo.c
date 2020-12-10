#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

const int MAX_THREADS = 1024;

long thread_count;
long long number_in_cirlce = 0;
long long total_tosses;
pthread_mutex_t mutex;

void* Monte_carlo(void* rank);

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);


int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;
    double pi_estimate;

    Get_args(argc, argv);
    thread_handles = malloc(thread_count * sizeof(pthread_t));

    if(pthread_mutex_init(&mutex, NULL) != 0){
        printf("\nmutex init failed\n");
        exit(1);
    }

    for(thread = 0; thread < thread_count; thread++)
        pthread_create(thread_handles+thread, NULL, Monte_carlo, (void*) thread);
    
    for(thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    pi_estimate = 4 * number_in_cirlce / (double)total_tosses;
    printf("Monte Carlo estimate: %lf", pi_estimate);

    free(thread_handles);
    return 0;
}

void* Monte_carlo(void* rank){
    long my_rank = (long) rank;
    long long my_tosses = total_tosses/thread_count;
    long long my_first_i = my_tosses * my_rank;
    long long my_last_i = my_first_i + my_tosses;
    long long my_hits = 0;
    double x, y;
    double distance_square;
    srand(time(0));
    for(int i = my_first_i; i < my_last_i; i++){
        x = (double)rand()/RAND_MAX*2.0 - 1.0;
        y = (double)rand()/RAND_MAX*2.0 - 1.0;
        distance_square = x*x + y*y;
        if(distance_square <=1)
            my_hits++;
    }
    pthread_mutex_lock(&mutex);
    number_in_cirlce += my_hits;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void Get_args(int argc, char* argv[]){
    if(argc != 3)
        Usage(argv[0]);
    thread_count = strtol(argv[1], NULL, 10);
    if(thread_count <= 0 || thread_count > MAX_THREADS)
        Usage(argv[0]);
    total_tosses = strtoll(argv[2], NULL, 10);
    if(total_tosses <= 0)
        Usage(argv[0]);
}

void Usage(char* prog_name){
    fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
    fprintf(stderr, "   n is the number of throws and should be >= 1\n");
    fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
    exit(0);
}