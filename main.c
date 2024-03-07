#include "tsl.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for malloc

void donothing() {
    printf("Holy shit it worked\n");
    tsl_exit(); // This should be at the end of your thread function
}

int main() {
    tsl_init(ALG_FCFS);
    
    // Create a new thread
    int result = tsl_create_thread(&donothing, NULL);
    if (result == TSL_ERROR) {
        printf("tsl_create_thread(&donothing, NULL); FAIL\n");
    } else {
        printf("tsl_create_thread(&donothing, NULL); SUCCESS\n");
        printf("tid: %d\n", result);
        printf("Running thread: %d\n", tsl_gettid());
    }
    
    // Let the main thread yield to the new thread
    int tid = tsl_yield(result);
    printf("Switched to thread: %d\n", tid);

    // Wait for the new thread to complete
    tsl_join(result);
    printf("Done.\n");

    return 0;
}
