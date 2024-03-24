#include "tsl.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for malloc

void donothing1() {
    printf("donothing1 is running...\n");
}

void donothing2() {
    printf("donothing2 is running...\n");
}

void factorial(int num) {
    printf("Factorial is running...\n");
    long long fact = 1;
    for(int i = 2; i <= num; i++) {
        fact *= i;
    }
    printf("Factorial of %d is %lld\n", num, fact);
}

void fibonacci(int n) {
    printf("Fibonacci is running...\n");
    long long fib[n+1];
    fib[0] = 0;
    fib[1] = 1;
    for(int i = 2; i <= n; i++) {
        fib[i] = fib[i-1] + fib[i-2];
    }
    printf("Fibonacci number %d is %lld\n", n, fib[n]);
}

int main() {
    tsl_init(ALG_FCFS);
    
    // Create a new thread
    int thread1 = tsl_create_thread(&donothing1, NULL);
    int thread2 = tsl_create_thread(&donothing2, NULL);
    int thread3 = tsl_create_thread(&factorial, 5);
    int thread4 = tsl_create_thread(&fibonacci, 10);

    if (thread2 == TSL_ERROR || thread1 == TSL_ERROR || thread3 == TSL_ERROR || thread4 == TSL_ERROR) {
        printf("tsl_create_thread FAIL\n");
    }
    
    // Let the main thread yield to the new thread
    int tid4 = tsl_yield(thread4);

    tsl_join(thread4);
    tsl_join(thread3);
    tsl_join(thread2);
    tsl_join(thread1);

    printf("Main thread is exiting...\n");
    tsl_quit();
    return 0;
}
