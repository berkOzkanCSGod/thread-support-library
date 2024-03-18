#ifndef __USE_GNU //added so that gcc uses gnu ucontext.c
#define __USE_GNU
#endif /* __USE_GNU */ 

#include "tsl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define RUNNING 1
#define READY 0
#define ENDED -1

#define QUEUE_ERROR -1
#define QUEUE_SUCCESS 0

#define MIN_ID 1000
#define MAX_ID 9999

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_RESET    "\x1b[0m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"

#define DEBUG_MODE 1

typedef struct TCB TCB;
typedef struct runqueue runqueue;

// globals
struct runqueue* Q;
struct TCB* main_tcb;
int scheduling_algo;

//generates random id using rand
int generateid();

//-1: error
//0: success
int enqueue(runqueue* queue, TCB* tcb);

//NULL: error
//TCB*: success
TCB* dequeue(runqueue* queue);
int dequeue_by_tid(runqueue *queue, int tid);

void printq(runqueue* queue);

//return TCB* by finding by id
TCB* find_thread_by_id(int tid);

//return TCB* by finding thread->state == RUNNING
TCB* find_running_thread();
TCB* find_ended_thread();

void print_stack_memory(const stack_t *stack);

void stub (void (*tsf) (void*), void *targ);

TCB* select_next_thread();

void print_ucontext(ucontext_t *context);

void print_tcb(TCB* tcb);

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

int tsl_init(int salg) {

    DEBUG_MODE ? printf("Initializing tsl...\n") : 0;

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
    if (enqueue(Q, main_tcb) == -1){
        printf(ANSI_COLOR_RED "ERROR: could not enqueue. [int tsl_init(int salg)]\n" ANSI_COLOR_RESET);
    }    

    DEBUG_MODE ? printq(Q) : 0;

    call_count = 1;

    scheduling_algo = salg;

    DEBUG_MODE ? printf("Successfully initialized tsl\n\n") : 0;

    return TSL_SUCCESS;
}

void tsl_quit(void) {
    DEBUG_MODE ? printf("Terminating tsl...\n") : 0;
    while (Q->size > 0) {
        tsl_cancel(dequeue(Q)->tid);
    }
    free(Q);
    Q = NULL;
    tsl_cancel(TID_MAIN);
    main_tcb = NULL;
    DEBUG_MODE ? printf("Terminated tsl\n") : 0;
    exit(0);
}

int tsl_create_thread(void (*tsf)(void *), void *targ) {
    
    TCB* new_tcb;
    ucontext_t current_context;

    //initializing new_tcb
    new_tcb = (TCB*)malloc(sizeof(TCB));
    if (new_tcb == NULL) {
        printf( ANSI_COLOR_RED "ERROR: new thread control block could not be initialized. Reason: unknown. [int tsl_create_thread(void (*tsf)(void *), void *targ)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }
    new_tcb->state = READY;
    new_tcb->tid = generateid();
    new_tcb->stack = (char *)malloc(TSL_STACKSIZE);

    if (new_tcb->stack == NULL) {
        printf( ANSI_COLOR_RED "ERROR: new_tcb->stack could not be initialized. Reason: unknown. [int tsl_create_thread(void (*tsf)(void *), void *targ)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }

    DEBUG_MODE ? printf("in create thread, initialization worked successfully.\n") : 0;

    //get current thread's context
    getcontext(&current_context);

    DEBUG_MODE ? print_ucontext(&current_context) : 0;

    new_tcb->context = current_context;
    new_tcb->context.uc_mcontext.gregs[REG_EIP] = (unsigned long)tsf; 
    new_tcb->context.uc_stack.ss_sp = malloc(TSL_STACKSIZE);
    new_tcb->context.uc_stack.ss_size = TSL_STACKSIZE;
    new_tcb->context.uc_stack.ss_flags = 0;

    DEBUG_MODE ? print_ucontext(&current_context) : 0;
    DEBUG_MODE ? print_ucontext(&new_tcb->context) : 0;

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
        if (enqueue(Q, new_tcb) == -1){ //CHANGED THIS LINE!! REMOVED "!" in the start of the condition
            printf(ANSI_COLOR_RED "ERROR: could not enqueue. [int tsl_create_thread(void (*tsf)(void *), void *targ)]\n" ANSI_COLOR_RESET);
        }
    } else {
        printf(ANSI_COLOR_RED "ERROR: queue not initialized, likely because tsl_init was not called. [int tsl_create_thread(void (*tsf)(void *), void *targ)]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }

    DEBUG_MODE ? printf("End of create thread\n") : 0;
    DEBUG_MODE ? printq(Q) : 0;

    return new_tcb->tid;

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
    printf("tsl_yield called with tid: %d\n", tid);

    TCB* current_tcb;
    TCB* next_thread;
    printf("Before finding running thread:\n");
    printq(Q);
    current_tcb = find_running_thread();
    if (current_tcb == NULL) {
        printf("No running thread found.\n");
        current_tcb = find_ended_thread();
        if(current_tcb == NULL) {
            printf("No ended thread found.\n");
            return TSL_ERROR;
        }
    }
    if(current_tcb->state == ENDED){
        printf("Current thread ended\n");
        printf("Before current thread dequeued\n");
        printq(Q);
        // Dequeue the current thread
        if (dequeue_by_tid(Q, current_tcb->tid) == -1) {
            printf("ERROR: Could not dequeue current thread.\n");
            return TSL_ERROR;
        }
        printf("After current thread dequeued\n");
        printq(Q);
        if (enqueue(Q, current_tcb) == -1) {
            printf("ERROR: Could not enqueue current thread.\n");
            return TSL_ERROR;
        }
        printf("Current thread enqueued\n");
        printf("After current thread is set to ENDED and enqueued\n");
        printq(Q);
        printf("Selecting next thread to run\n");
        if (tid > 0) {
            next_thread = find_thread_by_id(tid);
            if (next_thread == NULL) {
                printf("ERROR: No thread found with tid: %d\n", tid);
                return TSL_ERROR;
            }
        } else if ( tid == TSL_ANY) {
            next_thread = select_next_thread();
        }

        if (next_thread == NULL) {
            printf("ERROR: Could not select next thread.\n");
            return TSL_ERROR;
        }

        printf("Next thread to run id: %d\n", next_thread->tid);

        next_thread->state = RUNNING;
        printf("Next thread state set to RUNNING\nThe next thread is: \n");
        print_tcb(next_thread);

        setcontext(&next_thread->context);
        printf("Returning from tsl_yield with tid: %d\n", current_tcb->tid);
        return current_tcb->tid;
    }
    printf("Current running thread id: %d\n", current_tcb->tid);
    printf("Before current thread dequeued\n");
    printq(Q);
    // Dequeue the current thread
    if (dequeue_by_tid(Q, current_tcb->tid) == -1) {
        printf("ERROR: Could not dequeue current thread.\n");
        return TSL_ERROR;
    }
    printf("After current thread dequeued\n");
    printq(Q);
    current_tcb->state = READY;
    printf("Current thread state set to READY\n");

    if (enqueue(Q, current_tcb) == -1) {
        printf("ERROR: Could not enqueue current thread.\n");
        return TSL_ERROR;
    }
    printf("Current thread enqueued\n");
    printf("After current thread is set to READY and enqueued\n");
    printq(Q);

    if (getcontext(&current_tcb->context) == -1) {
        printf("ERROR: Could not get context.\n");
        return TSL_ERROR;
    }
    printf("Context of current thread saved\n");

    if (current_tcb->state == READY) {
        printf("Selecting next thread to run\n");

        if (tid > 0) {
            next_thread = find_thread_by_id(tid);
            if (next_thread == NULL) {
                printf("ERROR: No thread found with tid: %d\n", tid);
                return TSL_ERROR;
            }
        } else if ( tid == TSL_ANY) {
            next_thread = select_next_thread();
        }

        if (next_thread == NULL) {
            printf("ERROR: Could not select next thread.\n");
            return TSL_ERROR;
        }

        printf("Next thread to run id: %d\n", next_thread->tid);

        next_thread->state = RUNNING;
        printf("Next thread state set to RUNNING\nThe next thread is: \n");
        print_tcb(next_thread);

        setcontext(&next_thread->context);
    } else {
        printf("Thread resumed, changing state to RUNNING\n");
        current_tcb->state = RUNNING;
    }

    printf("Returning from tsl_yield with tid: %d\n", current_tcb->tid);
    return current_tcb->tid;
}

int tsl_exit() {
    // A thread will calltsl exit()to get terminated. 

    TCB* current_thread = find_running_thread();
    if (current_thread == NULL) {
        printf(ANSI_COLOR_YELLOW "WARNING: Could not find current thread to exit. [int tsl_exit()]\n" ANSI_COLOR_RESET);
        return TSL_ERROR;
    }
    current_thread->state = ENDED;

    DEBUG_MODE ? printf("exit request tid: %d\n\n", current_thread->tid) : 0;
    int ready_threads = 0;
    for (int i = 0; i < Q->size; i++) {
        if (Q->threads[i]->state == READY) {
            ready_threads++;
        }
    }
    if(ready_threads > 0){
        tsl_yield(TSL_ANY);
        return TSL_SUCCESS;
    } else {
        printf("No other threads to run, exiting\n");
        for(int i = 0; i < Q->size; i++){
            if(Q->threads[i]->tid == current_thread->tid){
                if (dequeue_by_tid(Q, current_thread->tid) == -1) {
                    printf(ANSI_COLOR_RED "ERROR: Could not dequeue thread with id: %d. [int tsl_join(int tid)]\n" ANSI_COLOR_RESET, current_thread->tid);
                    return TSL_ERROR;
                }
            }
        }
        printf("Thread with id: %d has been dequeued\n", current_thread->tid);
        tsl_print_queue();
        exit(TSL_SUCCESS);
    }
}
void tsl_print_queue(void) {
    printf("Queue Information:\n");
    printf("\tsize: %d\n", Q->size);
    printf("\thead: %d\n", Q->head);
    printf("\ttail: %d\n", Q->tail);
    printf("\tthreads: ");
    printq(Q);
    printf("\n");
}

int tsl_join(int tid) {

    //wait until target (tid) is terminated (state == ENDED)
    TCB* taget_thread = find_thread_by_id(tid);
    if (taget_thread == NULL) {
        printf(ANSI_COLOR_YELLOW "WARNING: Could not find thread with id: %d to exit. [int tsl_join(int tid)]\n" ANSI_COLOR_RESET, tid);
        return TSL_ERROR;
    }
    DEBUG_MODE ? printf("in tsl_join for tid: %d\n", tid) : 0;
    while(taget_thread->state != ENDED);
    
    for(int i = 0; i < Q->size; i++){
        if(Q->threads[i]->tid == tid){
            if (dequeue_by_tid(Q, tid) == -1) {
                printf(ANSI_COLOR_RED "ERROR: Could not dequeue thread with id: %d. [int tsl_join(int tid)]\n" ANSI_COLOR_RESET, tid);
                return TSL_ERROR;
            }
        }
    }
    printf("Thread with id: %d has been dequeued\n", tid);
    // free context stack
    free(taget_thread->context.uc_stack.ss_sp);
    // free TCB stack
    free(taget_thread->stack);
    // free TCB
    free(taget_thread);

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
        } else {
            DEBUG_MODE ? print_tcb(taget_thread) : 0;
        }
    } else {
        taget_thread = main_tcb;
        DEBUG_MODE ? printf("Main selected for cancel\n") : 0;
    }
    
    for(int i = 0; i < Q->size; i++){
        if(Q->threads[i]->tid == tid){
            if (dequeue_by_tid(Q, tid) == -1) {
                printf(ANSI_COLOR_RED "ERROR: Could not dequeue thread with id: %d. [int tsl_join(int tid)]\n" ANSI_COLOR_RESET, tid);
                return TSL_ERROR;
            }
        }
    }
    printf("Thread with id: %d has been dequeued\n", tid);
    // free context stack
    free(taget_thread->context.uc_stack.ss_sp);
    // free TCB stack
    free(taget_thread->stack);
    // free TCB
    free(taget_thread);

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
int dequeue_by_tid(runqueue *queue, int tid) {
    int i, index = -1;

    // Find the TCB with the given tid
    for (i = 0; i < queue->size; i++) {
        int currentIndex = (queue->head + i) % TSL_MAXTHREADS;
        if (queue->threads[currentIndex]->tid == tid) {
            index = currentIndex;
            break;
        }
    }

    // If not found, return error
    if (index == -1) {
        return -1;
    }

    // Shift all elements after the found TCB to fill the gap
    for (i = index; i != queue->tail; i = (i + 1) % TSL_MAXTHREADS) {
        queue->threads[i] = queue->threads[(i + 1) % TSL_MAXTHREADS];
    }

    // Decrease the tail index
        queue->tail = (queue->tail - 1 + TSL_MAXTHREADS) % TSL_MAXTHREADS;

    // Decrease the size of the queue
    queue->size--;

    return 0;  // Success
}
void printq(runqueue* queue) {
    if (queue->size > 0) {
        for (int i = 0; i < queue->tail-1; i++) {
            if (i != queue->tail && queue->threads[i] != NULL) {
                printf("%d, ", queue->threads[i]->tid);
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
    int index = 0;
    TCB* current = Q->threads[Q->head];
    while(index < Q->size){
        if(current->tid == tid){
            return current;
        }
        index++;
        current = Q->threads[index];
    }
    return NULL;
}

//return TCB* by finding thread->state == RUNNING
TCB* find_running_thread() {
    int index = 0;
    TCB* current = Q->threads[Q->head];
    while(index < Q->size){
        if(current->state == RUNNING){
            return current;
        }
        index++;
        current = Q->threads[index];
    }
    return NULL;
}
TCB* find_ended_thread() {
    int index = 0;
    TCB* current = Q->threads[Q->head];
    while(index < Q->size){
        if(current->state == ENDED){
            return current;
        }
        index++;
        current = Q->threads[index];
    }
    return NULL;
}

TCB* select_next_thread() {
    if (scheduling_algo == 1) {
        //dequeue
        TCB *next_thread = Q->threads[0];
        printf("SelectNextThread used FCFS: Selected thread:\n");
        print_tcb(next_thread);
        return next_thread;
    } else if (scheduling_algo == 2) {
        srand(time(NULL));
        int index = rand() % TSL_MAXTHREADS;
        while(Q->threads[index]->state != READY){
            index = rand() % TSL_MAXTHREADS;
        }
        TCB *next_thread = Q->threads[index];
        printf("SelectNextThread used Random Selection: Selected thread:\n");
        print_tcb(next_thread);
        return next_thread;
    }
}

void print_ucontext(ucontext_t *context) {
    printf("Context Information:\n");
    printf("\tuc_stack: {ss_sp: %p, ss_flags: %d, ss_size: %zu}\n",
           (void *)context->uc_stack.ss_sp, context->uc_stack.ss_flags,
           context->uc_stack.ss_size);
    printf("\tuc_mcontext: %p\n\n", (void *)&context->uc_mcontext);
}

void print_tcb(TCB* tcb) {
    if (tcb == NULL) {
        printf(ANSI_COLOR_RED "ERROR: unable to printf tcb beacuse it is NULL\n\n" ANSI_COLOR_RESET);
        exit(1);
    }
    printf("\n\n");
    printf("tid: %d\n", tcb->tid);
    printf("state: %s %d\n", tcb->state ? "RUNNING" : "READY", tcb->state);
    print_ucontext(&tcb->context);
    printf("\n\n");

}