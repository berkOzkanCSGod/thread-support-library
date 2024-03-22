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
    int thread2 = tsl_create_thread(&donothing2, NULL);

    if (thread2 == TSL_ERROR || thread1 == TSL_ERROR) {
        printf("tsl_create_thread FAIL\n");
    }
    
    // Let the main thread yield to the new thread
    int tid2 = tsl_yield(thread2);
    int tid1 = tsl_yield(thread1);

    tsl_join(thread2);
    tsl_join(thread1);


    // tsl_print_queue();


    tsl_quit();
    return 0;
}
