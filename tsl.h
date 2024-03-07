#ifndef TSL_H
#define TSL_H

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




//init tsl_init
int tsl_init(int salg);
int tsl_create_thread(void (*tsf)(void *), void *targ);
int tsl_yield(int tid);
int tsl_exit();
int tsl_join(int tid);
int tsl_cancel(int tid);
int tsl_gettid();
void tsl_quit(void);

#endif