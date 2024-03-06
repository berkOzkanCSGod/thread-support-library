#define __USE_GNU //added so that gcc uses gnu ucontext.c


#ifndef TSL_H
#define TSL_H

#include <ucontext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define MAX_THREADS 100
#define RUNNING 1
#define READY 0
#define ENDED -1
#define MIN_ID 10
#define MAX_ID 99
#define TSL_STACKSIZE 1024 * 64 //64kb
#define TSL_MAXTHREADS 100
#define TSL_ANY 0

typedef struct TCB TCB;
typedef struct runqueue runqueue;

//globals
extern struct runqueue* Q;
extern struct TCB* main_tcb;
// struct runqueue* Q;
// struct TCB* main_tcb;


//init tsl_init
int tsl_init(int salg);

int tsl_create_thread(void (*tsf)(void *), void *targ);

int tsl_yield(int tid);

int tsl_exit();

int tsl_join(int tid);

int tsl_cancel(int tid);

int tsl_gettid();

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

//return TCB* by finding by id
TCB* find_thread_by_id(int tid);

//return TCB* by finding thread->state == RUNNING
TCB* find_running_thread();


#endif