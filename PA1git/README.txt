Project I: my little malloc()
Due: February 23, 2023
_________________________________________________________________________________________________________________________________
Name and NetID:
Kusum Gandham kg699   *submitting assignment
Moussa Myrtil mm3053
_________________________________________________________________________________________________________________________________

Project Goal:
Simulating our own version of malloc and free functions using a defined array of size 4096. The code must be able to catch
errors, allocate memory, free memory, and coalesce two free adjacent nodes to make a bigger free one.
_________________________________________________________________________________________________________________________________

A look at the linked list structure:
In this code we are using a linked list structure to traverse through our memory. We have an integer named trackfree to keep
track of free(1) and allocated(0) nodes. We also have a next and prev to point to the next and previous nodes of the linked
list. It also has an unsigned integer, s, that holds the amount of space requested by malloc. The metaNode holds the metadata
which is this information about the allocated memory. The metaNode is 24 bytes.
_________________________________________________________________________________________________________________________________

mymalloc:
my malloc is the function that will allocate our requested memory in the array using the linked list structure explained
previously. Upon the first call, my malloc will intialize a free node with the size of the array (4096). 
From here malloc, will continue to seperate its allocated memory into seperate nodes. Each time malloc is called, a pointer
will iterate through the linked list to find a free node it can allocated its memory into. Three cases emerge from this search
1) There is not enough space
    This will just return an error message
2)There is just enough space
    This will mark the free node to an allocated node.
3)There is more space
    This will create two chunks out of the one free node. One chunk holds the sizeof(metaNode) plus the size we must allocate
    and the second chunk will be a free node with the remaining size after the allocation.
The pointer returned must be right at the data so we must return the ptr + sizeof(metaNode).
_________________________________________________________________________________________________________________________________

myfree:
myfree is a function that will take in a pointer and free that corresponding memory. To access our meta data on this pointer we
have to subract the pointer and the size of the metaNode. After accesing the metaNode we set the trackfree to 1 indiccating it 
is now free memory. 
However, this is not the end of our free function. We must coalesce adjacent free chunks with each other to make one bigger
chunk of free memory that is available for client use. We do this by using a while loop and pointers to keep track of 
adjacent free chunks and then combining those chunks. This method is called everytime free is called.
There are three error cases in free that we have accounted for.
    1)calling free on something not intialized using malloc
    2)calling free twice on the same pointer
    3)calling free at an address that is not at the start of the memory chunk
_________________________________________________________________________________________________________________________________

memgrind:
There were various stress cases we had to put our code through to test out our malloc and free functions in the code. There are 
three cases that were given to us. We then use time of day to get the times.

1.malloc() and immediately free() a 1 byte chunk 120 TIMES

2.malloc() to request 120 1-byte chunks, storing their pointerarray in an array, then use free to deallocate the chunks

3.repeat 120 times
  has two parts    |    Randomly choose between
  CASE A
      allocating a 1-byte chunk and storing the pointer in an array
  CASE B
      if there exists at least one chunk in the array, deallocate one chunk
   finally after u run 120 times, free all the remaining allocated chunks

We then had to include our own two test cases.

FIRST IMPLEMENTED STRESS TEST
Previously, we have only allocated bytes of the size one for test case four, we randomly chose between allocated and freeing A
random byte value between 2 and 20. This loops until the it alloactes 60 times. This is ran 100 times in the for loop.

SECOND IMPLEMENTED STRESS TEST
We fill in a pointer array with 120 allocations of size 100. Then we free every other block so there would be an alternating
free and allocated chunks. Then we allocate 50 bytes in the memory at these newly freed chunks before freeing all pointers at 
the end.
_________________________________________________________________________________________________________________________________

the results:
    Case 1: 0.00000204 seconds
    Case 2: 0.00002438 seconds
    Case 3: 0.00000946 seconds
    Case 4: 0.00000868 seconds
    Case 5: 0.00000446 seconds

In terms of time of ranking the fastest program timings it is as follows. Note: these are average times.
    1.Case 1
    2.Case 5
    3.Case 4
    4.Case 3
    5.Case 2
These results make sense as case 1 is very straight foward. Immediately freeing right after the allocation of 1 byte does not call
is not a very time demanding code, nor is it memory intensive. Case 2 on the other hand to was a lot more intense as it is 
allocating everything at once meaning the code will iterate through the linked list multiple times.
