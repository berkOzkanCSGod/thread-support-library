#ifndef TSL_H
#define TSL_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __USE_GNU //added so that gcc uses gnu ucontext.c
#define __USE_GNU
#endif /* __USE_GNU */ 

#include <ucontext.h>

#define TSL_STACKSIZE 1024 * 32 // 32 kB
#define TSL_MAXTHREADS 256

#define TSL_ANY 0
#define TID_MAIN 1

#define TSL_ERROR  -1 
#define TSL_SUCCESS 0 

#define ALG_FCFS 1
#define ALG_RANDOM 2

typedef struct{
    int tid; //thread id
    unsigned int state; // state of thread
    ucontext_t context; //pointer to context
    char* stack; //pointer to stack
} TCB;

typedef struct {
    TCB* threads[TSL_MAXTHREADS];
    int head; //index
    int tail; //index
    int size;
} runqueue;

// globals
extern runqueue* Q;
extern TCB* main_tcb;
extern int scheduling_algo;

//init tsl_init
int tsl_init(int salg);
int tsl_create_thread(void (*tsf)(void *), void *targ);
int tsl_yield(int tid);
int tsl_exit();
int tsl_join(int tid);
int tsl_cancel(int tid);
int tsl_gettid();
void tsl_quit(void);
void tsl_print_queue(void);

#endif