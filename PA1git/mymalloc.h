//taking copy and paste from canvas
#include <stddef.h>
#ifndef _MYMALLOC_H
#define _MYMALLOC_H


#define malloc(s)   mymalloc(s, __FILE__, __LINE__)
#define free(p)     myfree(p, __FILE__, __LINE__)
#include <stdio.h> 


//defining our memory with the size 4096
//making the memory not available to client
//memory array is set to all zero when u start
//*hint in lecture*
//checking if client has called for malloc or free the first time 
//but putting "flag" in first byte *end*
#define MEMSIZE 4096
char memory[MEMSIZE];


void *mymalloc(size_t size, char *file, int line);
void myfree(void *ptr, char *file, int line);
 
//shuffle through array, hold the metadata and payload
typedef struct metaNode
{
    //check if the memory is free or not and marks it with 0 or 1 (1 if free, 0 if not free)
    int trackfree;
    //pointer to next chunk
    struct metaNode *next;
    //pointer to prev chunk
    struct metaNode *prev;
    //size we need to allocate (unsigned integer)
    size_t s;
}metaNode;


#endif
