#define __USE_GNU //added so that gcc uses gnu ucontext.c
#include "tsl.h"

struct runqueue* Q;
struct TCB* main_tcb;

//-1: error
//0: success
int enqueue(runqueue* queue, TCB* tcb) {
    if (queue->size >= MAX_THREADS) {
        return -1;
    } else {
        if (queue->size == 0) {
            queue->threads[0] = tcb;
            queue->tail = 1;
        } else {
            queue->threads[queue->tail] = tcb;
            queue->tail++;
        }

        queue->size++;

        return 0;
    }
}

//-1: error
//0: success
int dequeue(runqueue* queue) {
    TCB* returned_tcb;

    if (queue->size == 0) {
        printf("size is 0 so no dequeue, but success.\n");
        return 0;
    } else if (queue->size < 0) {
        printf("size < 0, error.\n");
        return NULL;
    } else {

        returned_tcb = queue->threads[0];

        for (int i = 0; i < queue->tail; i++) {
            if (i != MAX_THREADS) {
                queue->threads[i] = queue->threads[i+1];
            }
        }
        
        if (queue->size > 0) {
            queue->size--;
        }

        queue->threads[queue->tail] = NULL;
        if (queue->tail > 0) {
            queue->tail--;
        }

        return returned_tcb; 
    }
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
        printf("queue is empty\n");
    }
}

int generateid() {
    srand((unsigned int)time(NULL));
    return (long int) (clock() + (rand() % (MAX_ID - MIN_ID + 1)) + MIN_ID);
}

int tsl_init(int salg) {
    Q = (runqueue*)malloc(sizeof(runqueue));

    main_tcb = (TCB*)malloc(sizeof(TCB));
    main_tcb->tid = 0;
    main_tcb->state = RUNNING;


}

void print_stack_memory(const stack_t *stack) {
    printf("Stack Memory:\n");
    unsigned char *ptr = (unsigned char *)stack->ss_sp;
    for (size_t i = 0; i < stack->ss_size; i++) {
        printf("%p: %02x\n", (void *)ptr, *ptr);
        ptr++;
    }
}

int tsl_create_thread(void (*tsf)(void *), void *targ) {
    
    TCB* new_tcb;
    ucontext_t current_context;

    //initializing new_tcb
    new_tcb = (TCB*)malloc(sizeof(TCB));
    new_tcb->state = READY;
    new_tcb->tid = generateid();
    new_tcb->stack = (char *)malloc(TSL_STACKSIZE);
    

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
        enqueue(Q, new_tcb);
    } else {
        printf("Error: queue not initialized, likely because tsl_init was not called.\n");
        exit(1);
    }

    //+ indicates done
    // initialize new TCB for new thread +
    // 	in ready state +
    // 	unique tid +
    // 	TCB will be added to ready queue +
    // allocate mem for TCB stack +
    // 	size is: TSL_STACKSIZE +
    // *TSL_MAXTHREADS -> max number of threads +
    // -----
    // setting up context
    // 	TCB->context = getcontext(current)+
    // 	EIP point to stub functionz error +
    // 	initialize stack_t of ucontext_t +
    // 	ESP point to top of stack +
    // 	...

    // push tsl and targ into the stack+

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

    current_tcb->state = READY;

    //adding current thread back into queue
    enqueue(Q, current_tcb);

    //save current context
    getcontext(&current_tcb->context);

    //selecting next thread to run
    if (tid > 0) {
        /**
         * If the tid paramater of the tsl yield() function is positive, 
         * then the respective thread (if exists) will be selected to run next.
        */
        next_thread = find_thread(tid);
    } else if ( tid == TSL_ANY) {
        // this should change based on the algo.
        next_thread = dequeue(Q);
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

}

int tsl_exit() {
    // A thread will calltsl exit()to get terminated. 
    TCB* current_thread = find_running_thread();
    current_thread->state = ENDED;
}

int tsl_join(int tid) {

    // add error checking later

    //wait until target (tid) is terminated (state == ENDED)
    TCB* taget_thread = find_thread_by_id(tid);

    while(taget_thread->state != ENDED);

    // free context stack
    free(taget_thread->context.uc_stack.ss_sp);
    // free TCB stack
    free(taget_thread->stack);
    // free TCB
    free(taget_thread);

    return (0);
}