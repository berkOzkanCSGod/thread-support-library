#include "tsl.h"
#include <time.h>

int main() {
    
    clock_t init_time, final_time;
    double total_time;

    long int list[89];
    int test[89];
    init_time = clock();

    for (int i = 0; i < 89; i++){
        list[i] = generateid();
        printf("%d\n", list[i]);
    }

    for (int i = 0; i < 89; i++){
        for (int j = 0; j < 89; j++){
            if (i != j && list[i] == list[j]) {
                printf("found duplicate %d-%d\n", list[i], list[j]);
            }
        }
    }

    final_time = clock();

    total_time = ((double) final_time - init_time) / CLOCKS_PER_SEC;
    printf("Time: %f\n", total_time);

    // time_t t;
    // clock_t t2;
    // time(&t);
    // t2 = clock() + generateid();
    // printf("%d\n", t);
    // printf("%d\n", t2);

    return 0;
}