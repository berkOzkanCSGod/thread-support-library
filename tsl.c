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
    if (queue->size == 0) {
        printf("size is 0 so no dequeue, but success.\n");
        return 0;
    } else if (queue->size < 0) {
        printf("size < 0, error.\n");
        return -1;
    } else {
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

        return 0; 
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
    //? indicates not sure
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
    // 	ESP point to top of stack ?
    // 	...

    // push tsl and targ into the stack+

}

