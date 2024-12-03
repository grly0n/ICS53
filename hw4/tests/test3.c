#include "icsmm.h"
#include <stdio.h>

void press_to_cont() {
    printf("Press enter to continue");
    while (getchar() != '\n');
    printf("\n");
}

int main(int argc, char *argv[]) {
    ics_mem_init();
    printf("Initialized heap\n");
    press_to_cont();
    
    printf("=== Not enough freelist blocks: allocation 1 ===\n");
    void *ptr0 = ics_malloc(98);
    void *ptr1 = ics_malloc(634);
    void *ptr2 = ics_malloc(151);
    void *ptr3 = ics_malloc(784);
    void *ptr4 = ics_malloc(234);
    void *ptr5 = ics_malloc(712);
    void *ptr6 = ics_malloc(156);
    void *ptr7 = ics_malloc(892);
    ics_payload_print(ptr7);
    press_to_cont();

    printf("=== Not enough freelist blocks: free operations ===\n");
    ics_free(ptr0);
    ics_free(ptr2);
    ics_free(ptr4);
    ics_free(ptr6);
    ics_freelist_print();
    press_to_cont();

    printf("=== Not enough freelist blocks: allocation 2 ===\n");
    void *ptr8 = ics_malloc(256);
    ics_freelist_print();
    /*
    printf("=== Multiple splinters: allocation 1 ===\n");
    void *ptr0 = ics_malloc(69);
    void *ptr1 = ics_malloc(72);
    void *ptr2 = ics_malloc(30);
    void *ptr3 = ics_malloc(2345);
    void *ptr4 = ics_malloc(150);
    void *ptr5 = ics_malloc(167);
    void *ptr6 = ics_malloc(256);
    void *ptr7 = ics_malloc(100);
    press_to_cont();
    
    printf("=== Multiple splinters: free operations ===\n");
    ics_free(ptr0);
    ics_free(ptr2);
    ics_free(ptr4);
    ics_free(ptr6);
    ics_freelist_print_compact();
    press_to_cont();
    
    printf("=== Multiple splinters: allocation 2 ===\n");
    void *ptr8 = ics_malloc(680);
    void *ptr9 = ics_malloc(80);
    void *ptr10 = ics_malloc(16);
    void *ptr11 = ics_malloc(250);
    ics_freelist_print_compact();
    */
    ics_mem_fini();
}
