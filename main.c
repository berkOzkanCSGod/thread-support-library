#include "tsl.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for malloc

// void print_stack_memory(const stack_t *stack) {
//     printf("Stack Memory:\n");
//     unsigned char *ptr = (unsigned char *)stack->ss_sp;
//     for (size_t i = 0; i < stack->ss_size; i++) {
//         printf("%p: %02x\n", (void *)ptr, *ptr);
//         ptr++;
//     }
// }

void donothing() {
    printf("in donothin\n");
}

int main() {

    tsl_create_thread(&donothing, NULL);

    return 0;
}
