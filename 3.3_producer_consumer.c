#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

const int MAX_THREADS = 1024;
const int MAX_MESSAGE = 256;

long thread_count;
char** messages;
int* available;
pthread_mutex_t mutex;

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);

void* Producer_consumer(void* rank);

int main(int argc, char* argv[]){
    long thread;
    pthread_t* thread_handles;

    Get_args(argc, argv);
    
    thread_handles = malloc(thread_count * sizeof(pthread_t));
    messages = malloc(thread_count*sizeof(char*));
    available = malloc(thread_count*sizeof(int));
    for(thread = 0; thread < thread_count; thread++)
        pthread_create(thread_handles+thread, NULL, Producer_consumer, (void*) thread);

    for(thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    free(thread_handles);
    free(messages);
    free(available);
    return 0;
}

void* Producer_consumer(void* rank){
    long my_rank = (long) rank;
    long dest = (my_rank + 1)%thread_count;
    long recv = (my_rank - 1 + thread_count)%thread_count;
    char* my_msg = malloc(MAX_MESSAGE * sizeof(char));

    //produce
    sprintf(my_msg, "Hello message to thread %ld from thread %ld\n", dest, my_rank);
    messages[dest] = my_msg;
    available[my_rank] = 1;

    //consume
    while(1){
        pthread_mutex_lock(&mutex);
        if(available[recv] == 1){
            //consume
            printf("%s", messages[my_rank]);
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

void Get_args(int argc, char* argv[]){
    if(argc != 2)
        Usage(argv[0]);
    thread_count = 2*strtol(argv[1], NULL, 10);
    if(thread_count <= 0 || thread_count > MAX_THREADS)
        Usage(argv[0]);
}

void Usage(char* prog_name){
    fprintf(stderr, "usage: %s <number of threads>\n", prog_name);
    fprintf(stderr, "The number of threads created are 2 * <number of threads>");
    exit(0);
}