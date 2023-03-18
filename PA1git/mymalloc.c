//some notes to self from the pdf write up:
//the simplest structure to use for a metadata is a linked list
//each chunk will have too components
//    metadata/header          |            payload/data
//metadata: information about the object -> size of chunk, whether the chunk is free or allocated
//payload: the actual data itself.


//error cases: {do nothing and return error message/kill program}
//calling free on something not intilaized with malloc
//calling free at an address not at the start of the memomy chunk
//calling free twice on the same pointer


//some notes from lecture (feb 6):
//write the implementation of mymalloc and myfree in this file
//simiulating our own free and malloc functions with out own given array to represent the memory (size 4096)
//helper functions use static functions (private methods)


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "mymalloc.h"


//this is our linked list that we start with as one big chunk of the memory as it has not been split yet
//all memory is intialized with so we must work with 1 when memory is allocated
static metaNode* tracker = NULL;
//static int firstallocation=0;

//making my malloc function reads in size and returns
//toallocate is the bytes being requested
//line is needed to return an error message
void* mymalloc(size_t toallocate, char* file, int line)
{
    //if allocated size is 0 return NULL
    if(toallocate==0)
    {
        printf("ERROR: tried allocating zero bytes");
        return NULL;
    }
    //first we must account for is this is the first malloc call...if it is we must make memory
    //if first allocation is 0 means the lnked list is empty
    //make new big node of free memory with the mem block sizse
    if (tracker==NULL)
    {
        //define this after first allocation
        tracker= (metaNode*) memory;
        tracker->next=NULL;
        tracker->prev = NULL;
        //free memory available
        tracker->trackfree=1;
        tracker->s= 4096-sizeof(metaNode);

    }

    metaNode *ptr=tracker;
    //metaNode *pptr=NULL;
    //now we will keep traversing out tracker(memory) euntil we find a node size that is either
    //exactly the size we we want OR it is bigger than the size we want
    while((ptr != NULL || ptr->trackfree != 1) && ptr->next != NULL)
    {
        //pptr=ptr;
        ptr=ptr->next;
    }
    //exactly the chunk we need 
    //so no need to split
    if (ptr->s<(toallocate+sizeof(metaNode)))
    {
        //marking the pointer allocated
        ptr->trackfree=0;

        //returning the pointer right after the meta data since actual data is stored after the metadata is stored
        return (char*)ptr+sizeof(metaNode);
    }

    //needs to split one big free node into allocated followed by a free
    //"chunkifying the code"
    else 
    {
        //node to take the remaining mem that isnt allocated
        //free chunk
        metaNode *freechunk;
        freechunk = (metaNode*)((char*)ptr + sizeof(metaNode) + toallocate);
		freechunk->prev = ptr;
		freechunk->next = ptr->next;
        //splitting chunk
		freechunk->s = ptr->s - (sizeof(metaNode) + toallocate);
		freechunk->trackfree = 1;

        //currentpointer is the size of allocated
		ptr->s = toallocate;
		ptr->trackfree = 0;
		ptr->next = freechunk;
        //printf("in malloc function\n");
        // the ptr after the metadata info
		return (char*)ptr + sizeof(metaNode);

    }
    printf("ERROR: Not enough memory in file '%s' on line '%d'\n", file, line);
	return 0;
}

//merging free memory
static void coalesce()
{
    metaNode* ptr;
	metaNode* tempp;
	tempp = tracker;
	ptr = tracker;
	while (ptr->next != NULL)
	{
		if (ptr->next->trackfree == 1) 
		{
			tempp->s = tempp->s + ptr->next->s + sizeof(metaNode);
			ptr->next = ptr->next->next;
			break;
		}
		ptr = ptr->next;
	}
    //return;
}


//FREE METHOD
void myfree(void* ptr, char* file, int line)
{
    metaNode* temp = ptr-sizeof(metaNode);
    //printf("size of allocation being freed %ld\n", temp->s);
    if(temp==NULL)
    {
        return;
    }
	if (ptr <= (void*)(memory + 4096) && ptr >= (void*)(memory)) 
	{
        //not a free node that was called
		if (temp->trackfree == 0)
		{
            //set free and merge memory
			temp->trackfree = 1;

			coalesce();
		}
        //ERROR STATEMENTS FOLLOWS
		else
		{
			printf("ERROR: Memory has already been freed in file '%s' on line '%d'\n", file, line);
            return;
		}
	}
	else
	{
		printf("ERROR: Pointer asked for is not valid in file '%s' on line '%d'\n", file, line);
        return;
	}
   // return;
    if(temp->s==0)
    {
        printf("ERROR: Not pointing to the beginning of data in file '%s' on line '%d'\n", file, line);
    }
}
