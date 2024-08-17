#include "mavalloc.h"
#include <time.h>
#include <stdio.h>

int main( int argc, char * argv[] )
{
    clock_t start = clock();

    mavalloc_init(1000000, BEST_FIT);
    void *arr[500];

    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 500; i++) {
            arr[i] = mavalloc_alloc(i+1);
        }

        for (int i = 0; i < 500; i += 2) {
            mavalloc_free(arr[i]);
            arr[i] = NULL;
        }

        for (int i = 0; i < 500; i += 3) {
            mavalloc_free(arr[i]);
            arr[i] = NULL;
        }

        void *ptr1 = mavalloc_alloc(250);
        void *ptr2 = mavalloc_alloc(250);
        void *ptr3 = mavalloc_alloc(250);
        void *ptr4 = mavalloc_alloc(250);
        void *ptr5 = mavalloc_alloc(250);
        void *ptr6 = mavalloc_alloc(250);
        void *ptr7 = mavalloc_alloc(250);
        void *ptr8 = mavalloc_alloc(250);

        mavalloc_free(ptr1);
        mavalloc_free(ptr3);
        mavalloc_free(ptr5);
        mavalloc_free(ptr7);

        void *ptr9 = mavalloc_alloc(300);
        void *ptr10 = mavalloc_alloc(300);
        void *ptr11 = mavalloc_alloc(300);
        void *ptr12 = mavalloc_alloc(300);

        mavalloc_free(ptr2);
        mavalloc_free(ptr4);
        mavalloc_free(ptr6);
        mavalloc_free(ptr8);
        mavalloc_free(ptr9);
        mavalloc_free(ptr10);
        mavalloc_free(ptr11);
        mavalloc_free(ptr12);

        for (int i = 0; i < 500; i++) {
            mavalloc_free(arr[i]);
        }
    }
	
	mavalloc_destroy();

    clock_t end = clock();
    double elapsed = (double) (end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("Time elapsed in ms: %f\n", elapsed);

    return 0;
}
