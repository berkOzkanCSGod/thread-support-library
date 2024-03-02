#include <stdio.h>
#include <string.h>
#include "../tsl.h"


int main() {

    TCB* t1 = (TCB*)malloc(sizeof(TCB));
    if (t1 == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for TCB t1\n");
        return 1; 
    }
    t1->tid = 1;

    TCB* t2 = (TCB*)malloc(sizeof(TCB));
    if (t2 == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for TCB t2\n");
        return 1; 
    }
    t2->tid = 2;

    TCB* t3 = (TCB*)malloc(sizeof(TCB));
    if (t3 == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for TCB t3\n");
        return 1; 
    }
    t3->tid = 3;
    
    TCB* t4 = (TCB*)malloc(sizeof(TCB));
    if (t4 == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for TCB t4\n");
        return 1; 
    }
    t4->tid = 4;


    runqueue* Q = (runqueue*)malloc(sizeof(runqueue));
    
    Q->head = 0;
    Q->size = 0;
    Q->tail = 0;

    enqueue(Q, t1);
    enqueue(Q, t2);
    enqueue(Q, t3);
    enqueue(Q, t4);

    printq(Q);

    dequeue(Q);
    printq(Q);
    dequeue(Q);
    printq(Q);
    dequeue(Q);
    printq(Q);
    dequeue(Q);

    printq(Q);


    free(t1);
    free(t2);
    free(t3);
    free(t4);

    free(Q);

    return 0;
}