#include <stdio.h>
#include "tested_declarations.h"
#include "rdebug.h"

struct memory_manager_t test;

int main(void) {
    char *ptr1 = heap_realloc(NULL, 10);
    char *ptr2 = heap_realloc(NULL, 10);
    char *ptr3 = heap_realloc(NULL, 10);
    char *ptr4 = heap_realloc(NULL, 10);
    ((void ) ptr1);
    ((void ) ptr2);
    ((void ) ptr3);
    ((void ) ptr4);
    return 0;
}

