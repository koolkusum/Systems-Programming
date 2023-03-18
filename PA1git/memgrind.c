//memgrind.c
//these are our own test cases to test the malloc and free we have implemented

//run these tests 50 times each
//get average time
//use gettimeofday() to keep track of run time
//has to to do four things
//1.
//  malloc() and immediately free() a 1 byte chunk 120 TIMES
//2.
//  malloc() to request 120 1-byte chunks, storing their pointerarray in an array, then use free to deallocate the chunks
//3.
//repeat 120 times
//  has two parts    |    Randomly choose between
//  CASE A
//      allocating a 1-byte chunk and storing the pointer in an array
//  CASE B
//      if there exists at least one chunk in the array, deallocate one chunk
//finally after u run 120 times, free all the remaining allocated chunks
//4. Your own two stress tests





//this should not take several seconds to run...could be the code is not efficient enough



#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "mymalloc.h"
// malloc() and immediately free() a 1 byte chunk 120 TIMES
float caseone()
{
    struct timeval start, end;

	gettimeofday(&start, NULL);
	
	for (int i = 0; i < 120; i++)
	{
		char* x = (char*) malloc(1);
		free(x);
	}

	gettimeofday(&end, NULL);
	float sum1=(end.tv_sec - start.tv_sec) * 10000000 ;
	float sum2=(end.tv_usec - start.tv_usec) * 0.000001;

	return (float)(sum1 + sum2 );

}
//malloc() to request 120 1-byte chunks, storing their pointerarray in an array, then use free to deallocate the chunks
float casetwo()
{
    struct timeval start, end;

	gettimeofday(&start, NULL);

	char * pointerarray[120];

	for(int i=0;i<120; i++)
	{
		char* ptr= (char*)malloc(1);
		pointerarray[i]=ptr;
		//printf("in case 2 malloc\n");
	}

	for(int i=0; i<120;i++)
	{
		free(pointerarray[i]);
	}

	gettimeofday(&end, NULL);
	float sum1=(end.tv_sec - start.tv_sec) * 10000000 ;
	float sum2=(end.tv_usec - start.tv_usec) * 0.000001;

	return (float)(sum1 + sum2 );
}

//repeat 120 times
//  has two parts    |    Randomly choose between
//  CASE A
//      allocating a 1-byte chunk and storing the pointer in an array
//  CASE B
//      if there exists at least one chunk in the array, deallocate one chunk
//finally after u run 120 times, free all the remaining allocated chunks
float casethree()
{
	struct timeval start, end;

	gettimeofday(&start, NULL);

	char * pointerarray[120];

	int arrayindex=0;
	
	int malloccall=0;

	while(malloccall<120)
	{
		int r = rand()%2;
		//printf("%d\n",r);
		if (r==1)
		{
			char *ptr=(char*)malloc(1);
			pointerarray[arrayindex]=ptr;
			arrayindex++;
			malloccall++;
		}
		else
		{
			if(arrayindex!=0)

			{
				free(pointerarray[arrayindex-1]);
				arrayindex--;

			}
		}
	}
	for (int i = 0; i < arrayindex; i++)
	{
		if (pointerarray[i] != NULL)
		{
			free(pointerarray[i]);
		}
	}

	gettimeofday(&end, NULL);
	float sum1=(end.tv_sec - start.tv_sec) * 10000000 ;
	float sum2=(end.tv_usec - start.tv_usec) * 0.000001;

	return (float)(sum1 + sum2 );
}

//this test case allocates random sizes between 2 and 20 and also randonly frees until it has allocated 60 times
//it then frees at the very end
float casefour()
{
 	struct timeval start, end;

	gettimeofday(&start, NULL);

	char * pointerarray[120];

	int arrayindex=0;
	
	int malloccall=0;

	while(malloccall<60)
	{
		int r = rand()%2;
		//printf("%d\n",r);

		if (r==1)
		{
			int allocatespace= (rand()%20)+2;
			//printf(" size of chunk %d\n",allocatespace);

			char *ptr=(char*)malloc(allocatespace);
			pointerarray[arrayindex]=ptr;
			//printf("malloc: %d\n", arrayindex);

			arrayindex=arrayindex+1;
			malloccall=malloccall+1;
			//printf("malloc call %d\n", malloccall);
		}
		else
		{
			if(arrayindex!=0)

			{
				
				free(pointerarray[arrayindex-1]);
				//printf("free: %d\n", arrayindex-1);
				arrayindex--;

			}

		}
	}
	for (int i = 0; i < arrayindex; i++)
	{
		if (pointerarray[i] != NULL)
		{
			free(pointerarray[i]);
		}
	}

	gettimeofday(&end, NULL);
	float sum1=(end.tv_sec - start.tv_sec) * 10000000 ;
	float sum2=(end.tv_usec - start.tv_usec) * 0.000001;

	return (float)(sum1 + sum2 );

}

//this test case allocates till the pointer array is filled
//before freeing everyother allocation
//then it reallocated between the allocated locations where it has been freed
//after this test we free everything at the end
float casefive()
{
	struct timeval start, end;

	gettimeofday(&start, NULL);


	char*pointerarray[120];

	int arrayindex = 0;
	
	int chunks = 1;
    

	

    while ((pointerarray[arrayindex] = malloc(100)) != NULL ) 
	{
      arrayindex++;
	  chunks++;
      
    }

    for (int i = 0; i < arrayindex; i += 2) 
	{
      free(pointerarray[i]);
      pointerarray[i] = NULL;
    }

    for (arrayindex = 0; arrayindex < chunks-1; arrayindex += 2) 
	{
      pointerarray[arrayindex] = malloc(50);
    }

    for (int i = 0; i < chunks-1; i += 1) 
	{
      free(pointerarray[i]);
      pointerarray[i] = NULL;
    }


	gettimeofday(&end, NULL);
	float sum1=(end.tv_sec - start.tv_sec) * 10000000 ;
	float sum2=(end.tv_usec - start.tv_usec) * 0.000001;

	return (float)(sum1 + sum2 );
}

int main(int argc, char* argv[])
{
    

	//CASE ONE
	float totala = 0.0;
	for (int i = 0; i < 50; i++)
	{
		float a = caseone();
		if (a < 10)
			totala = totala + a;
	}
	float timeA = totala / 50;
	printf("Case 1: ");
    printf("%.8f seconds\n", timeA);

	//CASE TWO	
	float totalb = 0.0;
	for (int i = 0; i < 50; i++)
	{
		float a = casetwo();
		if (a < 10)
			totalb = totalb + a;
	}
	float timeB = totalb / 50;
	printf("Case 2: ");
    printf("%.8f seconds\n", timeB);

	
	//CASE THREE
	float totalc = 0.0;
	for (int i = 0; i < 50; i++)
	{
		float a = casethree();
		if (a < 10)
			totalc = totalc + a;
	}
	float timeC = totalc / 50;
	printf("Case 3: ");
    printf("%.8f seconds\n", timeC);


	//CASE FOUR (PERSONAL TEST)
		float totald = 0.0;
	for (int i = 0; i < 100; i++)
	{
		float a = casefour();
		if (a < 10)
			totald = totald+ a;
	}
	float timeD = totald / 50;
	printf("Case 4: ");
    printf("%.8f seconds\n", timeD);


	//CASE FIVE (PERSONAL TEST)
		float totale = 0.0;
	for (int i = 0; i < 50; i++)
	{
		float a = casefour();
		if (a < 10)
			totale = totale+ a;
	}
	float timee = totale / 50;
	printf("Case 5: ");
    printf("%.8f seconds\n", timee);
}