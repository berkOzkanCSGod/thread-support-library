#include "tsl.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for malloc

void print_stack_memory(const stack_t *stack) {
    printf("Stack Memory:\n");
    unsigned char *ptr = (unsigned char *)stack->ss_sp;
    for (size_t i = 0; i < stack->ss_size; i++) {
        printf("%p: %02x\n", (void *)ptr, *ptr);
        ptr--;
    }
}


int main() {
    // Example ucontext_t structure
    ucontext_t context;
    getcontext(&context);
    stack_t stack;

    // Allocate memory for the stack (replace this with actual stack allocation)
    size_t stack_size = 16;
    stack.ss_sp = malloc(stack_size);
    if (stack.ss_sp == NULL) {
        fprintf(stderr, "Failed to allocate stack memory\n");
        return 1;
    }
    stack.ss_size = stack_size;
    stack.ss_flags = 0;
    void* stack_top = stack.ss_sp + stack.ss_size;
    context.uc_mcontext.gregs[REG_ESP] = (unsigned long)stack_top; 
    context.uc_stack = stack;

    printf("stack_top: %p\n", stack_top);

    // print_stack_memory(&context.uc_stack);

    int a = 420;
    double b = 6.9;

    printf("sizeof(a): %d, sizeof(b): %d\n", sizeof(a), sizeof(b));
    
    printf("before a stack_top: %p\n", stack_top);
    printf("*(int*) stack_top: %d\n", *(int*) stack_top);
    stack_top -= sizeof(a);
    *(int*) stack_top = a;
    printf("after a stack_top: %p\n", stack_top);
    printf("*(int*) stack_top: %d\n", *(int*) stack_top);

    // stack_top -= sizeof(b);
    // printf("after b stack_top: %p\n", stack_top);
    // printf("*(int*) stack_top: %d\n", *(int*) stack_top);


    // *(int *)stack_top = a;
    // *(double *)(stack_top - sizeof(a)) = b;

    context.uc_mcontext.gregs[REG_ESP] = (unsigned long)stack_top; 

    // printf("Going to print now...\n");
    // print_stack_memory(&context.uc_stack);


    free(stack.ss_sp);

    return 0;
}
