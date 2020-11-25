// NAME: Ian Conceicao
// EMAIL: IanCon234@gmail.com
// ID: 505153981

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

//Flags
int THREADS_FLAG = 0;
int ITERATIONS_FLAG = 0;
int YIELD_FLAG = 0;
int SYNC_FLAG = 0;

pthread_t *threads;
int iterations = 1;
char syncMode = 'n';
long long counter = 0;


pthread_mutex_t mutex;
int spinLock = 0;

char tag[15] = "add";

void add(long long *pointer, long long value) {
        long long sum = *pointer + value;
        if (YIELD_FLAG)
                sched_yield();
        *pointer = sum;
}

void badSystemCall(char* customMessage){
    fprintf(stderr, "%s: %s \n",customMessage,strerror(errno));
    exit(1);
}


void beforeExit(){
    if(threads)
        free(threads);
}

void* changeCounter(){
    long long i = 1;
    for(;i > -2; i -=2){

        int j = 0;
        for(; j < iterations; j++){

            switch(syncMode){
                case 'n': //None
                    add(&counter, i); //Critical Section
                    break;
                case 'm': //Mutex
                    pthread_mutex_lock(&mutex);
                    add(&counter, i); //Critical Section
                    pthread_mutex_unlock(&mutex);
                    break;
                case 's': //Spin lock
                    while (__sync_lock_test_and_set(&spinLock, 1)){};
					add(&counter, i); //Critical Section
					__sync_lock_release(&spinLock);
                    break;
                case 'c': //Compare and swap
                    ;long long old, new;
                    do{
                        old = counter;
                        new = old + i;
                        if(YIELD_FLAG)
                            sched_yield();
                    } while(__sync_val_compare_and_swap(&counter, old, new) != old);
                    break;
            }
        }
    }
    return NULL;
}

void makeTag(){
   // char tag[15] = "add";
    // ^ declared globally

    if(YIELD_FLAG)
        strcat(tag, "-yield");
    
    switch(syncMode){
        case 'n':
            strcat(tag,"-none");
            break;
        case 'm':
            strcat(tag,"-m");
            break;
        case 's':
            strcat(tag,"-s");
            break;
        case 'c':
        strcat(tag,"-c");
        break;
    }
}

int main(int argc, char** argv){

    int numOfThreads = 1;

    char* correctUsage = "Usage: lab2_add [--threads][--iterations][--yield][--sync]\n";
    static const struct option long_options[] = {
        {"threads",required_argument, NULL, 't'},
        {"iterations",required_argument, NULL, 'i'},
        {"yield",no_argument, NULL, 'y'},
        {"sync",required_argument, NULL, 's'},
        {0,0,0,0}
    };

    char c;
    int optionIndex=0;
    
    while(1){
        c = getopt_long(argc,argv,"",long_options,&optionIndex);
        
        if(c==-1)
            break;
        
        switch(c){
            case 't':
                THREADS_FLAG=1;
                numOfThreads=atoi(optarg);
                break;
            case 'i':
                ITERATIONS_FLAG=1;
                iterations=atoi(optarg);
                break;
            case 'y':
                YIELD_FLAG=1;
                break;
            case 's':
                SYNC_FLAG=1;
                syncMode=optarg[0];
                break;
            default:
                fprintf(stderr, "%s \n", correctUsage);
                exit(1);
        }

    }

    //Check input is valid
    if(iterations<=0){
        fprintf(stderr,"Error: Number of iterations must be greater than 0.\n");
        exit(1);
    }
    if(numOfThreads<=0){
        fprintf(stderr,"Error: Number of threads must be greater than 0.\n");
        exit(1);
    }
    if(SYNC_FLAG && syncMode!='m' && syncMode!='s' && syncMode!='c'){
        fprintf(stderr,"Error: sync must have argument of 'm', 's', or 'c' \n");
        exit(1);
    }

    //Create mutex lock for threads if syncMode is mutex
    if(syncMode == 'm')
        if(pthread_mutex_init(&mutex, NULL))
            badSystemCall("Failed to initialize mutex");
    
    atexit(beforeExit); //Frees the memory even if we exit early

    threads = malloc(numOfThreads * sizeof(pthread_t));

    struct timespec initialTime;
	clock_gettime(CLOCK_MONOTONIC, &initialTime);

    //Start threads

    int i = 0;
    for(;i < numOfThreads; i ++){
        if(pthread_create(&threads[i], NULL, changeCounter,NULL))
            badSystemCall("Failed to make a thread");
    }


    //Wait for all threads to finish that function
    i=0;
    for(;i<numOfThreads;i++){
        if(pthread_join(threads[i],NULL))
            badSystemCall("Failed to join threads");
    }

    struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);

    long long totalRunTime = 1000000000 * (endTime.tv_sec - initialTime.tv_sec) + (endTime.tv_nsec - initialTime.tv_nsec);

    makeTag();
    int totalOperations = numOfThreads * iterations * 2;
    int averageTimePerOperation = totalRunTime / totalOperations;

    fprintf(stdout,"%s,%d,%d,%d,%lld,%d,%lld\n",tag,numOfThreads,iterations,totalOperations,totalRunTime,averageTimePerOperation,counter);

    exit(0);
}