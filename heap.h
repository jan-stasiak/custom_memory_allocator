#ifndef PROJEKT_2_HEAP_H
#define PROJEKT_2_HEAP_H

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

#define FENCE '#'
#define FENCE_SIZE 16
#define CHUNK_SIZE sizeof(struct memory_chunk_t)
#define PAGE 4096

enum pointer_type_t {
    pointer_null,
    pointer_heap_corrupted,
    pointer_control_block,
    pointer_inside_fences,
    pointer_inside_data_block,
    pointer_unallocated,
    pointer_valid
};


struct memory_manager_t {
    void *memory_start;
    size_t memory_size;
    struct memory_chunk_t *first_memory_chunk;
    intptr_t start_size;
};

struct memory_chunk_t {
    struct memory_chunk_t *prev;
    struct memory_chunk_t *next;
    size_t size;
    int free;
    double hash;
};

double chunk_hash(struct memory_chunk_t *chunk);

int heap_setup(void);

void heap_clean(void);

void *heap_malloc(size_t size);

void *heap_calloc(size_t number, size_t size);

void *heap_realloc(void *memblock, size_t count);

void heap_free(void *memblock);

size_t heap_get_largest_used_block_size(void);

enum pointer_type_t get_pointer_type(const void *const pointer);

int heap_validate(void);

#endif //PROJEKT_2_HEAP_H
