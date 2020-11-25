#include "SortedList.h"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include<signal.h> 

int opt_yield = 0;
//Flags
int THREADS_FLAG = 0;
int ITERATIONS_FLAG = 0;
int YIELD_FLAG = 0;
int SYNC_FLAG = 0;

SortedList_t* list;
SortedListElement_t* elements; //List of elements for threads to create data structure from
int sizeOfList = 0;

pthread_t *threads;
int iterations = 1;
int numOfThreads = 1;
int length;


pthread_mutex_t mutex;
int spinLock = 0;

char syncMode = 'n';
char* yieldInput ="";

char tag[20] = "list";

void badSystemCall(char* customMessage){
    fprintf(stderr, "%s: %s \n",customMessage,strerror(errno));
    exit(1);
}

void listError(char* customMessage){
    fprintf(stderr, "List Error: %s\n",customMessage);
    exit(3);
}

void badYieldInput(){
    fprintf(stderr,"Error: Bad yield input. \nCorrect yield input: --yield=[idl], i for insert, d for delete, l for lookups \n");
    exit(1);
}


void makeTag(){
   // char tag[15] = "add";
    // ^ declared globally

    if(!YIELD_FLAG)
        strcat(tag, "-none");
    else{
        strcat(tag, "-");
        strcat(tag,yieldInput);
    }
    
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
    }
}


void beforeExit(){
    if(list)
         free(list);
    if(elements)
        free(elements);
    if(threads)
        free(threads);
}

void setElems(){

    elements = malloc(sizeOfList*sizeof(SortedListElement_t));
    srand(time(NULL));   // Initialization, should only be called once.
    char randomletter;
    int i = 0;
    for(; i < sizeOfList;i++){
            randomletter = 'A' + (rand() % 26);

            char* inputKey = malloc(2);
            inputKey[0] = randomletter;
            inputKey[1] = '\0';
            
            elements[i].key = inputKey;
    }
}

void catch(){
    fprintf(stderr, "Segmentation fault caught, exitting \n");
    exit(2);
}


void* doWork(void* input){
    int threadNum = *(int *)input;

    //Create data structure
    int i = threadNum;
    for(;i < sizeOfList; i += numOfThreads){
        switch(syncMode){
            case 'n':
                SortedList_insert(list, &elements[i]);
                break;
            case 'm':
                pthread_mutex_lock(&mutex);
				SortedList_insert(list, &elements[i]);
				pthread_mutex_unlock(&mutex);
				break;
            case 's':
                while(__sync_lock_test_and_set(&spinLock, 1)){};
				SortedList_insert(list, &elements[i]);
				__sync_lock_release(&spinLock);
                break;
        }
    }

    //Get the length

    switch(syncMode){
            case 'n':
                length = SortedList_length(list);
                break;
            case 'm':
                pthread_mutex_lock(&mutex);
                length = SortedList_length(list);
				pthread_mutex_unlock(&mutex);
				break;
            case 's':
                while(__sync_lock_test_and_set(&spinLock, 1)){};
                length = SortedList_length(list);
				__sync_lock_release(&spinLock);
                break;
    }

    if(length == -1)
        listError("List corrupted on insertion of elements, failed to get length, exitting.");
    
    //Look up and delete old

    SortedListElement_t* node;

    i = threadNum;
    int delete = 0;
    for(; i < sizeOfList; i += numOfThreads){
        switch(syncMode){
            case 'n':
                node = SortedList_lookup(list, elements[i].key);
                delete = SortedList_delete(node);
                break;
            case 'm':
                pthread_mutex_lock(&mutex);
                node = SortedList_lookup(list, elements[i].key);
                delete = SortedList_delete(node);
                pthread_mutex_unlock(&mutex);
                break;
            case 's':
                while (__sync_lock_test_and_set(&spinLock, 1)){};
                node = SortedList_lookup(list, elements[i].key);
                delete = SortedList_delete(node);
				__sync_lock_release(&spinLock);
                break;
        }
        if(!node)
            listError("List corrupted! Element cannot be found");
        if(delete)
            listError("Failed deleting a node.");
    }
    return NULL;
}

int main(int argc, char** argv){
	signal(SIGSEGV, catch);


    char* correctUsage = "Usage: lab2_ [--threads][--iterations][--yield][--sync]\n";
    static const struct option long_options[] = {
        {"threads",required_argument, NULL, 't'},
        {"iterations",required_argument, NULL, 'i'},
        {"yield",required_argument, NULL, 'y'},
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
                yieldInput=optarg;
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
    if(SYNC_FLAG && syncMode!='m' && syncMode!='s'){
        fprintf(stderr,"Error: sync must have argument of 'm', or 's' \n");
        exit(1);
    }
    if(strlen(yieldInput) > 3){
        badYieldInput();
    }
    //Set opt_yield accordingly
    unsigned int k = 0;
    for(; k < strlen(yieldInput); k++){
        switch(yieldInput[k]){
            case 'i':
                opt_yield = opt_yield | INSERT_YIELD;
                break;
            case 'd':
                opt_yield = opt_yield | DELETE_YIELD;
                break;
            case 'l':
                opt_yield = opt_yield | LOOKUP_YIELD;
                break;
            default:
                badYieldInput();
                break;
        }
    }

    //Create empty circular list
    list = malloc(sizeof(SortedList_t));
    list->key = NULL;
    list->prev = list;
    list->next = list;

    sizeOfList = numOfThreads * iterations;

    //Create mutex lock for threads if syncMode is mutex
    if(syncMode == 'm')
        if(pthread_mutex_init(&mutex, NULL))
            badSystemCall("Failed to initialize mutex");

    setElems();
    atexit(beforeExit); //Frees the memory even if we exit early

    threads = malloc(numOfThreads * sizeof(pthread_t));
    int* threadNumb = malloc(numOfThreads*sizeof(int));

    struct timespec initialTime;
	clock_gettime(CLOCK_MONOTONIC, &initialTime);

    //Start threads

    int i = 0;
    for(;i < numOfThreads; i ++){
        threadNumb[i]=i;
        if(pthread_create(&threads[i], NULL, doWork, &threadNumb[i]))
            badSystemCall("Failed to make a thread");
    }

    //Wait for all threads to finish that function
    i=0;
    for(;i<numOfThreads;i++){
        if(pthread_join(threads[i],NULL))
            badSystemCall("Failed to join threads");
    }

    if(!length)
        listError("Length of list at the end was not 0.");

    struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);

    long long totalRunTime = 1000000000 * (endTime.tv_sec - initialTime.tv_sec) + (endTime.tv_nsec - initialTime.tv_nsec);

    int totalOperations = numOfThreads * iterations * 3;
    int averageTimePerOperation = totalRunTime / totalOperations;
    
    makeTag();
    fprintf(stdout,"%s,%d,%d,1,%d,%lld,%d\n",tag,numOfThreads,iterations,totalOperations,totalRunTime,averageTimePerOperation);

    exit(0);
}