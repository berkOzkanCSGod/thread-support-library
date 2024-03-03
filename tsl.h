#define __USE_GNU
#include <ucontext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define MAX_THREADS 100
#define RUNNING 1
#define READY 0
#define MIN_ID 1000
#define MAX_ID 9999
#define TSL_STACKSIZE 1024 * 64 //64kb
#define TSL_MAXTHREADS 100

typedef struct TCB TCB;
typedef struct runqueue runqueue;

//globals
struct runqueue* Q;
struct TCB* main_tcb;


//init tsl_init
int tsl_init(int salg);

int tsl_create_thread(void (*tsf)(void *), void *targ);


typedef struct TCB {
    int tid; //thread id
    unsigned int state; // state of thread
    ucontext_t context; //pointer to context
    char* stack; //pointer to stack
} TCB;

typedef struct runqueue {
    TCB* threads[MAX_THREADS];
    int head; //index
    int tail; //index
    int size;
} runqueue;

//generates random id using rand
int generateid();

//-1: error
//0: success
int enqueue(runqueue* queue, TCB* tcb);

//-1: error
//0: success
int dequeue(runqueue* queue);

void printq(runqueue* queue);