#include "tsl.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for malloc

void donothing1() {
    printf("Holy shit it worked\n");
}

void donothing2() {
    printf("Holy shit it worked PART 2\n");
}

int main() {
    tsl_init(ALG_FCFS);
    
    // Create a new thread
    int thread1 = tsl_create_thread(&donothing1, NULL);
    tsl_print_queue();
    if (thread1 == TSL_ERROR) {
        printf("tsl_create_thread(&donothing1, NULL); FAIL\n");
    } else {
        printf("tsl_create_thread(&donothing1, NULL); SUCCESS\n");
        printf("tid: %d\n", thread1);
        printf("Running thread: %d\n", tsl_gettid());
    }
    int thread2 = tsl_create_thread(&donothing2, NULL);
    tsl_print_queue();
    if (thread2 == TSL_ERROR) {
        printf("tsl_create_thread(&donothing2, NULL); FAIL\n");
    } else {
        printf("tsl_create_thread(&donothing2, NULL); SUCCESS\n");
        printf("tid: %d\n", thread1);
        printf("Running thread: %d\n", tsl_gettid());
    }
    
    // Let the main thread yield to the new thread
    int tid1 = tsl_yield(thread1);
    tsl_join(thread1);
    int tid2 = tsl_yield(thread2);
    tsl_join(thread2);

    tsl_print_queue();
    printf("Done.\n");
    tsl_quit();
    return 0;
}
