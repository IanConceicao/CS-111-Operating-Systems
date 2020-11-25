//NAME: Ian Conceicao
//EMAIL: IanCon234@gmail.com
//ID: 505153981

#include "SortedList.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    if(!list  || !element) //Null
        return;
    
    SortedListElement_t *node = list->next;

    while(1){
        if(node == list)
            break;
        if(strcmp(node->key, element->key) > 0)
            break;
        
        node = node->next;  //A critical section but I'm choosing the section below
    }

    if(opt_yield & INSERT_YIELD)
        sched_yield();
    
    //Critical Section
    element->prev = node->prev;
    element->next = node;
    node->prev->next = element;
    node->prev = element;
}


int SortedList_delete( SortedListElement_t *element){
    if(!element || element->prev->next != element || element->next->prev != element)
        return 1;
  
    if(opt_yield & DELETE_YIELD)
        sched_yield();
    
    //Critical Section
    element->next->prev = element->prev;
    element->prev->next = element->next;

    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){

    if(!list ||  !key)
        return NULL;

    SortedListElement_t *node = list->next;

    while(1){
        if(node == list)
            break;
        
        if(!strcmp(node->key,key))
            return node;

        if(opt_yield & LOOKUP_YIELD)
            sched_yield();
        
        //Critical Section
        node = node->next;
    }

    return NULL;
}


int SortedList_length(SortedList_t *list){
    if(!list || !list->next){
        //fprintf(stdout,"Bad list given.\n");
        return -1;
    }

    SortedListElement_t *node = list->next;

    int length=0;
    while(1){
        if(!node->next || !node->prev){
            //fprintf(stdout,"Bad nodes found.\n");
            return -1;
        }

        if(node == list)
            break;
        
        if(opt_yield & LOOKUP_YIELD)
            sched_yield();
        
        //Critical Section
        length++; 
        node = node->next;
    }
    
    return length;
}

