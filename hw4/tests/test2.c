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
    /*
    printf("=== Splinter test ===\n");
    void *ptr0 = ics_malloc(40);
    void *ptr1 = ics_malloc(200);
    void *ptr2 = ics_malloc(300);
    void *ptr3 = ics_malloc(3000);
    ics_payload_print(ptr0);
    ics_payload_print(ptr1);
    ics_payload_print(ptr2);
    ics_payload_print(ptr3);
    press_to_cont();
    
    printf("=== Free ptr1 ===\n");
    ics_free(ptr1);
    ics_freelist_print();
    press_to_cont();

    printf("=== Allocate ptr4 ===\n");
    void *ptr4 = ics_malloc(401);
    ics_payload_print(ptr4);
    ics_freelist_print();
    */
    printf("=== First three mallocs ===\n");
    void *ptr0 = ics_malloc(1000);
    void *ptr1 = ics_malloc(500);
    void *ptr2 = ics_malloc(300);
    //ics_payload_print(ptr0);
    //ics_payload_print(ptr1);
    //ics_payload_print(ptr2);
    press_to_cont();

    printf("=== Free ptr0 ===\n");
    ics_free(ptr0);
    ics_freelist_print_compact();
    press_to_cont();

    printf("=== Second two mallocs ===\n");
    void *ptr3 = ics_malloc(345);
    void *ptr4 = ics_malloc(123);
    //ics_payload_print(ptr3);
    //ics_payload_print(ptr4);
    press_to_cont();

    printf("=== Free ptr2 ===\n");
    ics_free(ptr2);
    ics_freelist_print_compact();
    press_to_cont();

    printf("=== Third two mallocs ===\n");
    void *ptr5 = ics_malloc(508);
    void *ptr6 = ics_malloc(712);
    //ics_payload_print(ptr5);
    //ics_payload_print(ptr6);
    press_to_cont();

    printf("=== Free ptr4 ===\n");
    ics_free(ptr4);
    ics_freelist_print_compact();
    press_to_cont();

    printf("=== Malloc ptr7 ===\n");
    void *ptr7 = ics_malloc(440);
    ics_freelist_print_compact();
    press_to_cont();
    
    printf("=== Malloc ptr8 ===\n");
    void *ptr8 = ics_malloc(260);
    ics_freelist_print_compact();
    press_to_cont();

    printf("=== Malloc ptr9 ===\n");
    void *ptr9 = ics_malloc(300);
    ics_freelist_print_compact();
    press_to_cont();


    ics_mem_fini();
}
