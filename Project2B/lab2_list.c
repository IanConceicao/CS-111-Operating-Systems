// NAME: Ian Conceicao
// EMAIL: IanCon234@gmail.com
// ID: 505153981


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
int LISTS_FLAG = 0;

SortedList_t** lists;
SortedListElement_t* elements; //List of elements for threads to create data structure from
int sizeOfList = 0;

pthread_t *threads;
int iterations = 1;
int numOfThreads = 1;
int length;
int numOfLists = 1;

long long totalLockTime=0;

pthread_mutex_t* mutexes;
int* spinLocks;


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
    if(lists)
         free(lists);
    if(elements)
        free(elements);
    if(threads)
        free(threads);
}


void setUpLocks(){

    //Make one type of lock per list
    if(syncMode=='s')
        spinLocks = malloc(sizeof(int)*numOfLists);
   
    if(syncMode=='m')
        mutexes =  malloc(sizeof(pthread_mutex_t)*numOfLists);

    int i = 0;
    for(;i<numOfLists;i++){
        if (syncMode == 's')
            spinLocks[i]=0;
        if(syncMode == 'm')
            if(pthread_mutex_init(&mutexes[i], NULL))
                badSystemCall("Failed to initialize mutex");
    }
}

void setUpList(){

    sizeOfList = numOfThreads * iterations; //Total number of elems needed

    //Create empty circular lists
    lists = malloc(sizeof(SortedList_t*)*numOfLists); //list now serves as an array to actual lists
    
    int i = 0;
    for(; i < numOfLists; i++){
        lists[i] = malloc(sizeof(SortedList_t));
        lists[i]->key = NULL;
        lists[i]->prev = lists[i];
        lists[i]->next = lists[i];
    }
}


int hash(const char* A){
    return (A[0] % numOfLists);
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


void setTime(struct timespec *startTime){
    if (clock_gettime(CLOCK_MONOTONIC, startTime) == -1)
        badSystemCall("Error setting timer to measure the lock waiting times");
}

void endTime(struct timespec *startTime, struct timespec *endTime){
    if (clock_gettime(CLOCK_MONOTONIC, endTime) == -1)
        badSystemCall("Error setting timer to measure the lock waiting times");
    totalLockTime += 1000000000 * (endTime->tv_sec - startTime->tv_sec) + (endTime->tv_nsec - startTime->tv_nsec);
    //Can change shared variable because this function is only called when holding a lock^
}


int getLength(){ //Method is protected, only call in critical sections
    int i = 0;
    int sum = 0;
    int individual = 0;
    for(; i<numOfLists;i++){
        individual = SortedList_length(lists[i]);
        if(individual==-1)
            return -1;
        sum += individual;
    }
    return sum;
}

void* doWork(void* input){
    int threadNum = *(int *)input;

    struct timespec start;
    struct timespec end;
    
    //Create data structure
    int i = threadNum;
    for(;i < sizeOfList; i += numOfThreads){
        int hashResult = hash(elements[i].key);
        switch(syncMode){
            case 'n':
                SortedList_insert(lists[hashResult], &elements[i]);
                break;
            case 'm':
                setTime(&start);
                pthread_mutex_lock(&mutexes[hashResult]);
                endTime(&start, &end);
                SortedList_insert(lists[hashResult], &elements[i]);
				pthread_mutex_unlock(&mutexes[hashResult]);
				break;
            case 's':
                setTime(&start);
                while(__sync_lock_test_and_set(&spinLocks[hashResult], 1)){};
                endTime(&start, &end);
                SortedList_insert(lists[hashResult], &elements[i]);
				__sync_lock_release(&spinLocks[hashResult]);
                break;
        }
    }

    i = 0;
    int sum = 0;
    int individual = 0;
    //Get the length
    switch(syncMode){
            case 'n':
                for(;i<numOfLists;i++){
                    individual = SortedList_length(lists[i]);
                    if(individual==-1)
                        listError("List corrupted on insertion of elements, failed to get length, exitting.");
                    sum+=individual;
                }
                break;
            case 'm':

                for(;i<numOfLists;i++){
                    setTime(&start);
                    pthread_mutex_lock(&mutexes[i]);
                    endTime(&start, &end);

                    individual = SortedList_length(lists[i]);
                    if(individual==-1)
                        listError("List corrupted on insertion of elements, failed to get length, exitting.");
                    sum+=individual;

                    pthread_mutex_unlock(&mutexes[i]);
                }
				break;
            case 's':
                for(;i<numOfLists;i++){
                    setTime(&start);
                    while(__sync_lock_test_and_set(&spinLocks[i], 1)){};
                    endTime(&start, &end);
                   
                    individual = SortedList_length(lists[i]);
                    if(individual==-1)
                        listError("List corrupted on insertion of elements, failed to get length, exitting.");
                    sum+=individual;

                    __sync_lock_release(&spinLocks[i]);
                
                }
                break;
    }

    
    //Look up and delete old

    SortedListElement_t* node;

    i = threadNum;
    int delete = 0;
    for(; i < sizeOfList; i += numOfThreads){
        int hashResult = hash(elements[i].key);
        switch(syncMode){
            case 'n':
                node = SortedList_lookup(lists[hashResult], elements[i].key);
                delete = SortedList_delete(node);
                break;
            case 'm':
                setTime(&start);
                pthread_mutex_lock(&mutexes[hashResult]);
                endTime(&start, &end);
                node = SortedList_lookup(lists[hashResult], elements[i].key);
                delete = SortedList_delete(node);
                pthread_mutex_unlock(&mutexes[hashResult]);
                break;
            case 's':
                setTime(&start);
                while (__sync_lock_test_and_set(&spinLocks[hashResult], 1)){};
                endTime(&start, &end);
                node = SortedList_lookup(lists[hashResult], elements[i].key);
                delete = SortedList_delete(node);
				__sync_lock_release(&spinLocks[hashResult]);
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


    char* correctUsage = "Usage: lab2_ [--threads][--iterations][--yield][--sync][--lists]\n";
    static const struct option long_options[] = {
        {"threads",required_argument, NULL, 't'},
        {"iterations",required_argument, NULL, 'i'},
        {"yield",required_argument, NULL, 'y'},
        {"sync",required_argument, NULL, 's'},
        {"lists",required_argument,NULL, 'l'},
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
            case 'l':
                LISTS_FLAG=1;
                numOfLists=atoi(optarg);
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
    if(numOfLists<=0){
         fprintf(stderr,"Error: Number of lists must be greater than 0.\n");
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

    setUpLocks();
    setUpList();


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

    int individual = 0;
    int sum = 0;
     for(;i<numOfLists;i++){
        individual = SortedList_length(lists[i]);
        if(individual==-1)
            listError("List corrupted on deleting elements exitting.");
        sum+=individual;
    }

    if(sum != 0)
        listError("Length of list at the end was not 0.");

    struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);

    long long totalRunTime = 1000000000 * (endTime.tv_sec - initialTime.tv_sec) + (endTime.tv_nsec - initialTime.tv_nsec);

    int numOfOperations = numOfThreads * iterations * 3;
    long averageLockTime = totalLockTime/ numOfOperations;
    int averageOperationTime = totalRunTime / numOfOperations;
    
    makeTag();
	printf("%s,%d,%d,%d,%d,%lld,%d,%ld\n", tag, numOfThreads, iterations, numOfLists, numOfOperations, totalRunTime, averageOperationTime, averageLockTime);

    exit(0);
}