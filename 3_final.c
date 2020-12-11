#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

const int MAX_THREADS = 1024;

long m;
long long numIn = 0;
long long numTotal;
pthread_mutex_t mutex;

void* MC_Integral_Thread(void* rank);

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);

int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;
    double estimate;

    Get_args(argc, argv);

    thread_handles = malloc(m * sizeof(pthread_t));

    for(thread = 0; thread < m; thread++)
        pthread_create(thread_handles + thread, NULL, MC_Integral_Thread, (void*) thread);

    for(thread = 0; thread < m; thread++)
        pthread_join(thread_handles[thread], NULL);

    printf("%lld %lld\n", numIn, numTotal);
    estimate = numIn/(double)numTotal;
    printf("Estimate: %lf", estimate);

    free(thread_handles);
    return 0;
}

void* MC_Integral_Thread(void* rank){
    long my_rank = (long) rank;
    long long my_toss = numTotal/m;
    long long my_first_i = my_toss * my_rank;
    long long my_last_i = my_first_i + my_toss;
    long long my_hits = 0;
    double x, y, height;
    srand(time(0));
    for(long long i = my_first_i; i < my_last_i; i++){
        x = rand()/(double)RAND_MAX;
        y = rand()/(double)RAND_MAX;
        height = pow(x, 3);

        if(y < height)
            my_hits++;
    }
    pthread_mutex_lock(&mutex);
    numIn += my_hits;
    pthread_mutex_unlock(&mutex);
}

void Get_args(int argc, char* argv[]){
    if(argc != 3)
        Usage(argv[0]);
    m = strtol(argv[1], NULL, 10);
    if(m <= 0 || m > MAX_THREADS)
        Usage(argv[0]);
    numTotal = strtoll(argv[2], NULL, 10);
    if(numTotal <= 0)
        Usage(argv[0]);
}

void Usage(char* prog_name){
    fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
    fprintf(stderr, "   n is the number of throws and should be >= 1\n");
    fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
    exit(0);
}