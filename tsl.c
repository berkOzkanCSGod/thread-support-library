#include "tsl.h"


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

int tsl_init(int salg) {
    runqueue* Q;
    TCB* main_tcb;

    Q = (runqueue*)malloc(sizeof(runqueue));
    main_tcb = (TCB*)malloc(sizeof(TCB));
    main_tcb->tid = 0;
    main_tcb->state = RUNNING;


}

// You will initialize your library in this function.
// An application will call this function exactly once before creating any threads.
// The parameter salg is used to indicate the scheduling algorithm the library
// will use. On success, 0 will be return. On failure, -1 (TSL ERROR) will be
// returned.
// The tsl init() function will initialize your library. As part of this initialization,
// a ready queue structure (runqueue structure) should be created and initialized
// as well. It will keep a list of TCBs corresponding to the threads that are in ready
// state. If you wish you can create and initialize other queues. It is up to you how to
// manage the set of TCBs of the threads that can be in various states. You can keep
// all TCBs in a single data structure or in multiple data structures.
// You will also need to allocate a TCB for the main thread of the application
// (process). Its state will be RUNNING initially. You will also assign a unique thread
// identifier to the main thread. It can be 1)

//define tsl_init