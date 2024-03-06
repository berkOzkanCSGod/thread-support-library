#ifndef __USE_GNU //added so that gcc uses gnu ucontext.c
#define __USE_GNU
#endif /* __USE_GNU */ 

#include "tsl.h"


typedef struct TCB {
    int tid; //thread id
    unsigned int state; // state of thread
    ucontext_t context; //pointer to context
    char* stack; //pointer to stack
} TCB;

typedef struct runqueue {
    TCB* threads[TSL_MAXTHREADS];
    int head; //index
    int tail; //index
    int size;
} runqueue;

//globals
struct runqueue* Q;
struct TCB* main_tcb;


int tsl_init(int salg) {

    printf("Initializing tsl...\n");

    static int call_count = 0;

    if (call_count) {
        printf(ANSI_COLOR_YELLOW "WARNING: Cannot call tsl_init twice. [int tsl_init(int salg)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }

    Q = (runqueue*)malloc(sizeof(runqueue));

    if (Q == NULL) {
        printf(ANSI_COLOR_RED "ERROR: Queue 'Q' did not initialize. Reason: unknown [int tsl_init(int salg)]\n" ANSI_COLOR_RESET);
    }

    main_tcb = (TCB*)malloc(sizeof(TCB));
    main_tcb->tid = TID_MAIN;
    main_tcb->state = RUNNING;

    call_count = 1;

    printf("Successfully initialized tsl\n");

    return TSL_SUCCESS;
}

void tsl_quit(void) {
    printf("Terminating tsl...\n");
    while (Q->size > 0) {
        tsl_cancel(dequeue(Q)->tid);
    }
    free(Q);
    Q = NULL;
    tsl_cancel(TID_MAIN);
    main_tcb = NULL;
    printf("Terminated tsl\n");
}

int tsl_create_thread(void (*tsf)(void *), void *targ) {
    
    TCB* new_tcb;
    ucontext_t current_context;

    //initializing new_tcb
    new_tcb = (TCB*)malloc(sizeof(TCB));
    new_tcb->state = READY;
    new_tcb->tid = generateid();
    new_tcb->stack = (char *)malloc(TSL_STACKSIZE);
    
    if (new_tcb == NULL) {
        printf( ANSI_COLOR_RED "ERROR: new thread control block could not be initialized. Reason: unknown. [int tsl_create_thread(void (*tsf)(void *), void *targ)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }

    //get current thread's context
    getcontext(&current_context);

    new_tcb->context = current_context;
    new_tcb->context.uc_mcontext.gregs[REG_EIP] = (unsigned long)tsf; 
    new_tcb->context.uc_stack.ss_sp = malloc(TSL_STACKSIZE);
    new_tcb->context.uc_stack.ss_size = TSL_STACKSIZE;
    new_tcb->context.uc_stack.ss_flags = 0;

    char* stack_top = (char*) new_tcb->context.uc_stack.ss_sp + new_tcb->context.uc_stack.ss_size;
    new_tcb->context.uc_mcontext.gregs[REG_ESP] = (unsigned long)stack_top; 

    //pushing tsl and targs onto context stack
    stack_top -= sizeof(void (*) (void *));
    *(void (**) (void *)) stack_top = tsf;
    stack_top -= sizeof(void *);
    *(void **) stack_top = targ;
    new_tcb->context.uc_mcontext.gregs[REG_ESP] = (unsigned long)stack_top; 

    //add new_tcb to queue
    if (Q != NULL) {
        if (!enqueue(Q, new_tcb) == -1){
            printf(ANSI_COLOR_RED "ERROR: could not enqueue. [int tsl_create_thread(void (*tsf)(void *), void *targ)]\n" ANSI_COLOR_RESET);
        }
    } else {
        printf(ANSI_COLOR_RED "ERROR: queue not initialized, likely because tsl_init was not called. [int tsl_create_thread(void (*tsf)(void *), void *targ)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }

    return TSL_SUCCESS;

    /*
        + indicates done
        initialize new TCB for new thread +
            in ready state +
            unique tid +
            TCB will be added to ready queue +
        allocate mem for TCB stack +
            size is: TSL_STACKSIZE +
        *TSL_MAXTHREADS -> max number of threads +
        setting up context
            TCB->context = getcontext(current)+
            EIP point to stub functionz error +
            initialize stack_t of ucontext_t +
            ESP point to top of stack +
        push tsl and targ into the stack +
    */
}

int tsl_yield(int tid) {
    // need to add error checking later //

    // A thread will call tsl yield() to give the cpu to some other thread.

    TCB* current_tcb;
    TCB* next_thread;

    // caller state: RUNNING --> READY
        // need to get TCB with state RUNNING
        // for now just represent it as a dequeue
    current_tcb = dequeue(Q);

    if (current_tcb == NULL) {
        printf(ANSI_COLOR_RED "ERROR: dequeued item is NULL. [int tsl_yield(int tid)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }

    current_tcb->state = READY;

    //adding current thread back into queue
    if (enqueue(Q, current_tcb) == -1) {
        printf(ANSI_COLOR_RED "ERROR: could not enqueue item. [int tsl_yield(int tid)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }

    //save current context
    getcontext(&current_tcb->context);

    //selecting next thread to run
    if (tid > 0) {
        next_thread = find_thread_by_id(tid);
    } else if ( tid == TSL_ANY) {
        // this should change based on the algo.
        next_thread = dequeue(Q);
    }

    if (next_thread == NULL) {
        printf(ANSI_COLOR_RED "ERROR: could not dequeue item. [int tsl_yield(int tid)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }

    // putting next thread onto CPU
    setcontext(&next_thread->context);
    next_thread->state = RUNNING;

    /**
     * caller state: RUNNING --> READY +
     * caller TCB: added to Q +
     * current context saved w/ getcontext() +
     * 
     * if tid > 0 +
     *  run this thread 
     * else if tid == TSL_ANY +
     *  run from schedule 
     * 
     * running thread: setcontext() +
     * 
     * 
    * /

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOT DONE:
        /*
        * Handling getcontext() Return:
        * - When a thread X calls getcontext(), it saves X's context and returns control.
        *   - This is the first return; X continues until it yields to another thread Y.
        * - When another thread (Y or Z) yields back to X using setcontext(), X resumes execution.
        *   - This is the second return.
        */

        /*
        * Actions to Take in Each Case:
        * - First Return:
        *   - Change X's state to READY and perform a context switch to another thread using setcontext().
        * - Second Return:
        *   - Change X's state to RUNNING if needed.
        */
    //////////////////////////////////////////////////////////////////////////////////////////////////////

}

int tsl_exit() {
    // A thread will calltsl exit()to get terminated. 

    TCB* current_thread = find_running_thread();
    if (current_thread == NULL) {
        printf(ANSI_COLOR_YELLOW "WARNING: Could not find current thread to exit. [int tsl_exit()]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }
    current_thread->state = ENDED;
    return TSL_SUCCESS;
}

int tsl_join(int tid) {

    //wait until target (tid) is terminated (state == ENDED)
    TCB* taget_thread = find_thread_by_id(tid);
    if (taget_thread == NULL) {
        printf(ANSI_COLOR_YELLOW "WARNING: Could not find thread with id: %d to exit. [int tsl_join(int tid)]\n" ANSI_COLOR_RESET, tid);
        return TSL_ERROR;
    }

    while(taget_thread->state != ENDED);

    // free context stack
    free(taget_thread->context.uc_stack.ss_sp);
    // free TCB stack
    free(taget_thread->stack);
    // free TCB
    free(taget_thread);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //maybe need to remove from queue
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    taget_thread = NULL;
    
    return tid;
}

int tsl_cancel(int tid) {
    //imediately perform tsl_exit and tsl_join
    TCB* taget_thread;
    
    if (tid != TID_MAIN) {
        taget_thread = find_thread_by_id(tid);
        if (taget_thread == NULL) {
            printf(ANSI_COLOR_RED "ERROR: could not find thread with id: %d. [int tsl_cancel(int tid)]\n" ANSI_COLOR_RESET, tid);
            return TSL_ERROR;
        }
    } else {
        taget_thread = main_tcb;
    }

    // free context stack
    free(taget_thread->context.uc_stack.ss_sp);
    // free TCB stack
    free(taget_thread->stack);
    // free TCB
    free(taget_thread);

    //maybe need to remove from queue
    taget_thread = NULL;

    return tid;

}

int tsl_gettid() {
    return find_running_thread()->tid;
}


void print_stack_memory(const stack_t *stack) {
    printf("Stack Memory:\n");
    unsigned char *ptr = (unsigned char *)stack->ss_sp;
    for (size_t i = 0; i < stack->ss_size; i++) {
        printf("%p: %02x\n", (void *)ptr, *ptr);
        ptr++;
    }
}


int enqueue(runqueue* queue, TCB* tcb) {
    if (queue->size >= TSL_MAXTHREADS) {
        return QUEUE_ERROR;
    } else {
        if (queue->size == 0) {
            queue->threads[0] = tcb;
            queue->tail = 1;
        } else {
            queue->threads[queue->tail] = tcb;
            queue->tail++;
        }

        queue->size++;

        return QUEUE_SUCCESS;
    }
}

TCB* dequeue(runqueue* queue) {

    TCB* returned_tcb;

    if (queue->size == 0) {
        printf(ANSI_COLOR_YELLOW "WARNING: Queue size 0, no action performed.[TCB* dequeue(runqueue* queue)]\n" ANSI_COLOR_RESET);
        return NULL;
    } else if (queue->size < 0) {
        printf(ANSI_COLOR_RED "ERROR: Queue size less than 0, unable to dequeue. [TCB* dequeue(runqueue* queue)]\n" ANSI_COLOR_RESET);
        return NULL;
    } else {

        returned_tcb = queue->threads[0];

        if (returned_tcb == NULL) {
            printf(ANSI_COLOR_RED "ERROR: first item in Q is NULL. [TCB* dequeue(runqueue* queue)]\n" ANSI_COLOR_RESET);
            return NULL;
        }

        for (int i = 0; i < queue->tail; i++) {
            if (i != TSL_MAXTHREADS) {
                queue->threads[i] = queue->threads[i+1];
            }
        }
        
        if (queue->size > 0) {
            queue->size--;
        } else {
            queue->size = 0;
        }

        queue->threads[queue->tail] = NULL;

        if (queue->tail > 0) {
            queue->tail--;
        }

        return returned_tcb; 
    }

    return NULL;
}

void printq(runqueue* queue) {
    if (queue->size > 0) {
        for (int i = 0; i < queue->tail-1; i++) {
            if (i != queue->tail && queue->threads[i] != NULL) {
                printf("%d_", queue->threads[i]->tid);
            }
        }
        printf("%d\n", queue->threads[queue->tail-1]->tid);
    } else {
        printf("Unable to print \'queue,\' because it is empty.\n");
    }
}

int generateid() {
    srand((unsigned int) clock());
    return (int) (clock() + (rand() % (MAX_ID - MIN_ID + 1)) + MIN_ID);
}

//return TCB* by finding by id
TCB* find_thread_by_id(int tid) {
    return NULL;
}

//return TCB* by finding thread->state == RUNNING
TCB* find_running_thread() {
    return NULL;
}