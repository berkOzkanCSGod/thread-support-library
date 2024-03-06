#ifndef TSL_H
#define TSL_H

#ifndef __USE_GNU //added so that gcc uses gnu ucontext.c
#define __USE_GNU
#endif /* __USE_GNU */ 

#include <ucontext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>


#define RUNNING 1
#define READY 0
#define ENDED -1

#define TSL_STACKSIZE 1024 * 32 // 32 kB
#define TSL_MAXTHREADS 256

#define TSL_ANY 0
#define TID_MAIN 1

#define TSL_ERROR  -1 
#define TSL_SUCCESS 0 

#define QUEUE_ERROR -1
#define QUEUE_SUCCESS 0

#define MIN_ID 1000
#define MAX_ID 9999

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_RESET    "\x1b[0m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"

#define ALG_FCFS 1
#define ALG_RANDOM 2

typedef struct TCB TCB;
typedef struct runqueue runqueue;

// globals
extern struct runqueue* Q;
extern struct TCB* main_tcb;

//init tsl_init
int tsl_init(int salg);
int tsl_create_thread(void (*tsf)(void *), void *targ);
int tsl_yield(int tid);
int tsl_exit();
int tsl_join(int tid);
int tsl_cancel(int tid);
int tsl_gettid();
void tsl_quit(void);

//generates random id using rand
int generateid();

//-1: error
//0: success
int enqueue(runqueue* queue, TCB* tcb);

//NULL: error
//TCB*: success
TCB* dequeue(runqueue* queue);

void printq(runqueue* queue);

//return TCB* by finding by id
TCB* find_thread_by_id(int tid);

//return TCB* by finding thread->state == RUNNING
TCB* find_running_thread();

void print_stack_memory(const stack_t *stack);

#endif