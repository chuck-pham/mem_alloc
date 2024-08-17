#include "mavalloc.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char * argv[] )
{
    clock_t start = clock();

    void *arr[500];

    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 500; i++) {
            arr[i] = malloc(i+1);
        }

        for (int i = 0; i < 500; i += 2) {
            free(arr[i]);
            arr[i] = NULL;
        }

        for (int i = 0; i < 500; i += 3) {
            free(arr[i]);
            arr[i] = NULL;
        }

        void *ptr1 = malloc(250);
        void *ptr2 = malloc(250);
        void *ptr3 = malloc(250);
        void *ptr4 = malloc(250);
        void *ptr5 = malloc(250);
        void *ptr6 = malloc(250);
        void *ptr7 = malloc(250);
        void *ptr8 = malloc(250);

        free(ptr1);
        free(ptr3);
        free(ptr5);
        free(ptr7);

        void *ptr9 = malloc(300);
        void *ptr10 = malloc(300);
        void *ptr11 = malloc(300);
        void *ptr12 = malloc(300);

        free(ptr2);
        free(ptr4);
        free(ptr6);
        free(ptr8);
        free(ptr9);
        free(ptr10);
        free(ptr11);
        free(ptr12);

        for (int i = 0; i < 500; i++) {
            free(arr[i]);
        }
    }

    clock_t end = clock();
    double elapsed = (double) (end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("Time elapsed in ms: %f\n", elapsed);

    return 0;
}
