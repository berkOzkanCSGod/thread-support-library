#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "tsl.h"

// this is a sample application that is using tsl library to 
// create and work with threads. 

#define MAXCOUNT 1000
#define YIELDPERIOD 100

void *foo(void *v)
{
	int count = 1;
    int mytid;
    
    mytid = tsl_gettid();
    printf ("thread %d started running (first time);  at the start function\n", mytid);

	//while (count < MAXCOUNT) {
    while (1) {
        printf ("thread %d is running (count=%d)\n", mytid, count);
		if (count % YIELDPERIOD == 0) {
			tsl_yield (TSL_ANY);
		}
        count++;
        if (count == MAXCOUNT)
                break;
	}
	return (NULL); 
}


int 
main(int argc, char **argv)
{
    int *tids;
    int i;
    int numthreads = 0;

    if (argc != 2) {
        printf ("usage: ./app numofthreads\n");
        exit(1); 
    }
    
    numthreads = atoi (argv[1]);

    tids = (int *) malloc (numthreads * sizeof(int));

	tids[0] = tsl_init (ALG_FCFS);
    // at tid[0] we have the id of the main thread
    
    for (i = 1; i < numthreads; ++i) {
		tids[i] = tsl_create_thread ((void *)&foo, NULL);
		printf ("thead %d created\n", (int) tids[i]);
	}

    for (i = 1; i < numthreads; ++i) {
        printf ("main: waiting for thead %d\n", (int) tids[i]);
        tsl_join (tids[i]);
        printf ("main: thead %d finished\n", (int) tids[i]);

    }
    
    printf ("main thread calling tlib_exit\n");
    tsl_exit();

	return 0;
}
