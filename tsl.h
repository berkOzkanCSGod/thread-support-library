#include <ucontext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_THREADS 100
#define RUNNING 1
#define READY 0

typedef struct TCB {
    int tid; //thread id
    unsigned int state; // state of thread
    ucontext_t context; //pointer to context
} TCB;

typedef struct runqueue {
    TCB* threads[MAX_THREADS];
    int head; //index
    int tail; //index
    int size;
} runqueue;

//-1: error
//0: success
int enqueue(runqueue* queue, TCB* tcb);

//-1: error
//0: success
int dequeue(runqueue* queue);

void printq(runqueue* queue);

//init tsl_init
int tsl_init(int salg);
