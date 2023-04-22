#include <stdio.h>
#include <math.h>
#include "heap.h"
#include <stddef.h>
#include <string.h>
#include "tested_declarations.h"
#include "rdebug.h"

struct memory_manager_t memory_manager;

int heap_setup(void) {
    memory_manager.memory_start = custom_sbrk(0);
    if (memory_manager.memory_start == (void *) -1) {
        return -1;
    }
    memory_manager.memory_size = 0;
    memory_manager.first_memory_chunk = NULL;

    return 0;
}

void heap_clean(void) {
    custom_sbrk(-((intptr_t) memory_manager.memory_size));
    memory_manager.memory_size = 0;
    memory_manager.first_memory_chunk = NULL;
    memory_manager.memory_start = NULL;
}

double generate_chunk_hash(struct memory_chunk_t *chunk) {
    if (chunk == NULL) {
        return -1;
    }

    double temp_hash = 0;
    for (int i = 0; i < (int) (CHUNK_SIZE - sizeof(double)); ++i) {
        temp_hash += *((uint8_t *) chunk + i);
    }
    return temp_hash;
}

void *heap_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    if (memory_manager.first_memory_chunk == NULL) {
        if (size >= memory_manager.memory_size) {
            memory_manager.start_size = (long int) custom_sbrk(0);
            void *temp = custom_sbrk(size + 2 * FENCE_SIZE + CHUNK_SIZE);
            if (temp == (void *) -1) {
                return NULL;
            }
            memory_manager.memory_size +=
                    size + 2 * FENCE_SIZE + CHUNK_SIZE - memory_manager.memory_size;
        }
        memory_manager.first_memory_chunk = memory_manager.memory_start;
        memory_manager.first_memory_chunk->size = size;
        memory_manager.first_memory_chunk->free = 0;
        memory_manager.first_memory_chunk->prev = NULL;
        memory_manager.first_memory_chunk->next = NULL;
        memory_manager.first_memory_chunk->hash = generate_chunk_hash(memory_manager.first_memory_chunk);
        memset((uint8_t *) memory_manager.memory_start + CHUNK_SIZE, FENCE, FENCE_SIZE);
        memset((uint8_t *) memory_manager.memory_start + CHUNK_SIZE + size + FENCE_SIZE, FENCE,
               FENCE_SIZE);
        return (uint8_t *) memory_manager.memory_start + CHUNK_SIZE + FENCE_SIZE;
    } else {
        struct memory_chunk_t *chunk;
        chunk = memory_manager.first_memory_chunk;

        while (1) {
            if (chunk->free == 1 && chunk->size - (2 * FENCE_SIZE) >= size) {
                chunk->free = 0;
                chunk->size = size;
                chunk->hash = generate_chunk_hash(chunk);
                memset((uint8_t *) chunk + CHUNK_SIZE, FENCE, FENCE_SIZE);
                memset((uint8_t *) chunk + CHUNK_SIZE + size + FENCE_SIZE, FENCE,
                       FENCE_SIZE);
                return (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE;
            }
            if (chunk->next == NULL) {
                break;
            }
            chunk = chunk->next;
        }
        int heap_size =
                (uint8_t *) chunk + CHUNK_SIZE + chunk->size + (2 * FENCE_SIZE) -
                (uint8_t *) memory_manager.memory_start;
        if (memory_manager.memory_size - heap_size < size + CHUNK_SIZE + (2 * FENCE_SIZE)) {
            void *temp = custom_sbrk(
                    (long) size - heap_size + memory_manager.memory_size + (2 * FENCE_SIZE) + CHUNK_SIZE);
            if (temp == (void *) -1) {
                return NULL;
            }
            memory_manager.memory_size += size - heap_size + memory_manager.memory_size + (2 * FENCE_SIZE) +
                                          CHUNK_SIZE;
        }
        chunk->next = (struct memory_chunk_t *) ((uint8_t *) chunk + CHUNK_SIZE + chunk->size +
                                                 FENCE_SIZE * 2);
        chunk->next->size = size;
        chunk->next->prev = chunk;
        chunk->next->next = NULL;
        chunk->next->free = 0;
        chunk->next->hash = generate_chunk_hash(chunk->next);
        chunk->hash = generate_chunk_hash(chunk);
        memset((uint8_t *) chunk->next + CHUNK_SIZE, FENCE, FENCE_SIZE);
        memset((uint8_t *) chunk->next + CHUNK_SIZE + chunk->next->size + FENCE_SIZE, FENCE,
               FENCE_SIZE);
        return (uint8_t *) chunk->next + CHUNK_SIZE + FENCE_SIZE;
    }

    return NULL;
}

void *heap_calloc(size_t number, size_t size) {
    if (size == 0 || number == 0) {
        return NULL;
    }
    size = size * number;
    if (memory_manager.first_memory_chunk == NULL) {
        if (size >= memory_manager.memory_size) {
            memory_manager.start_size = (long int) custom_sbrk(0);
            void *temp = custom_sbrk(size + 2 * FENCE_SIZE + CHUNK_SIZE);
            if (temp == (void *) -1) {
                return NULL;
            }
            memory_manager.memory_size +=
                    size + 2 * FENCE_SIZE + sizeof(struct memory_chunk_t) - memory_manager.memory_size;
        }
        memory_manager.first_memory_chunk = memory_manager.memory_start;
        memory_manager.first_memory_chunk->size = size;
        memory_manager.first_memory_chunk->free = 0;
        memory_manager.first_memory_chunk->prev = NULL;
        memory_manager.first_memory_chunk->next = NULL;
        memory_manager.first_memory_chunk->hash = generate_chunk_hash(memory_manager.first_memory_chunk);
        memset((uint8_t *) memory_manager.memory_start + CHUNK_SIZE, FENCE, FENCE_SIZE);
        memset((uint8_t *) memory_manager.memory_start + CHUNK_SIZE + FENCE_SIZE, 0, size);
        memset((uint8_t *) memory_manager.memory_start + CHUNK_SIZE + size + FENCE_SIZE, FENCE,
               FENCE_SIZE);
        return (uint8_t *) memory_manager.memory_start + CHUNK_SIZE + FENCE_SIZE;
    } else {
        struct memory_chunk_t *chunk;
        chunk = memory_manager.first_memory_chunk;

        while (1) {
            if (chunk->free == 1 && chunk->size - (2 * FENCE_SIZE) >= size) {
                chunk->free = 0;
                chunk->size = size;
                memset((uint8_t *) chunk + CHUNK_SIZE, FENCE, FENCE_SIZE);
                memset((uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE, 0, size);
                memset((uint8_t *) chunk + CHUNK_SIZE + size + FENCE_SIZE, FENCE,
                       FENCE_SIZE);
                chunk->hash = generate_chunk_hash(chunk);
                return (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE;
            }
            if (chunk->next == NULL) {
                break;
            }
            chunk = chunk->next;
        }
        int heap_size =
                (uint8_t *) chunk + CHUNK_SIZE + chunk->size + (2 * FENCE_SIZE) -
                (uint8_t *) memory_manager.memory_start;
        if (memory_manager.memory_size - heap_size < size + CHUNK_SIZE + (2 * FENCE_SIZE)) {
            void *temp = custom_sbrk(
                    (long) size - heap_size + memory_manager.memory_size + (2 * FENCE_SIZE) + CHUNK_SIZE);
            if (temp == (void *) -1) {
                return NULL;
            }
            memory_manager.memory_size += size - heap_size + memory_manager.memory_size + (2 * FENCE_SIZE) +
                                          CHUNK_SIZE;
        }
        chunk->next = (struct memory_chunk_t *) ((uint8_t *) chunk + CHUNK_SIZE + chunk->size +
                                                 FENCE_SIZE * 2);
        chunk->next->size = size;
        chunk->next->prev = chunk;
        chunk->next->next = NULL;
        chunk->next->free = 0;
        chunk->next->hash = generate_chunk_hash(chunk->next);
        chunk->hash = generate_chunk_hash(chunk);
        memset((uint8_t *) chunk->next + CHUNK_SIZE, FENCE, FENCE_SIZE);
        memset((uint8_t *) chunk->next + CHUNK_SIZE + FENCE_SIZE, 0, size);
        memset((uint8_t *) chunk->next + CHUNK_SIZE + chunk->next->size + FENCE_SIZE, FENCE,
               FENCE_SIZE);
        return (uint8_t *) chunk->next + CHUNK_SIZE + FENCE_SIZE;
    }

    return NULL;
    return NULL;
}

void *heap_realloc(void *memblock, size_t count) {
    if (heap_validate()) {
        return NULL;
    }
    if (memblock == NULL) {
        return heap_malloc(count);
    }
    struct memory_chunk_t *chunk = (struct memory_chunk_t *) ((uint8_t *) memblock - FENCE_SIZE - CHUNK_SIZE);
    if (get_pointer_type(memblock) != pointer_valid) {
        return NULL;
    } else if (count == chunk->size) {
        return memblock;
    } else if (count == 0) {
        heap_free(memblock);
        return NULL;
    } else if (count <= chunk->size) {
        chunk->size = count;
        memset((uint8_t *) memblock + chunk->size, FENCE, FENCE_SIZE);
        chunk->hash = generate_chunk_hash(chunk);
        return memblock;
    } else if (count > chunk->size) {
        if (chunk->next != NULL) {
            int how_much_stay =
                    ((uint8_t *) chunk->next) - ((uint8_t *) chunk) - (2 * FENCE_SIZE) - CHUNK_SIZE - chunk->size;
            if (count - chunk->size <= (size_t) how_much_stay && chunk->next->free == 1) {
                chunk->size = count;
                memset((uint8_t *) memblock + chunk->size, FENCE, FENCE_SIZE);
                chunk->hash = generate_chunk_hash(chunk);
                return memblock;
            } else if (count - chunk->size == how_much_stay + chunk->next->size && chunk->next->free == 1) {

                chunk->size = count;
                memset((uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE + chunk->size, FENCE, FENCE_SIZE);
                chunk->next = chunk->next->next;
                chunk->next->prev = chunk;
                chunk->hash = generate_chunk_hash(chunk);
                chunk->next->hash = generate_chunk_hash(chunk->next);
                return memblock;
            } else if (count - chunk->size < how_much_stay + chunk->next->size && chunk->next->free == 1) {
                int move_size = count - chunk->size - how_much_stay;
                memmove((uint8_t *) chunk->next + move_size, (uint8_t *) chunk->next, CHUNK_SIZE);
                memset((uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE + count, FENCE, FENCE_SIZE);
                chunk->size = count;
                chunk->next = (struct memory_chunk_t *) ((uint8_t *) chunk->next + move_size);
                chunk->next->size -= move_size;
                chunk->hash = generate_chunk_hash(chunk);
                chunk->next->hash = generate_chunk_hash(chunk->next);
                return memblock;
            } else if ((count - chunk->size > how_much_stay + chunk->next->size && chunk->next->free == 1) ||
                       (chunk->next->free == 0)) {
                uint8_t *new_chunk = heap_malloc(count);
                if (new_chunk == NULL) {
                    return NULL;
                }
                memcpy(new_chunk, memblock, chunk->size);
                heap_free((uint8_t *) chunk + FENCE_SIZE + CHUNK_SIZE);
                return new_chunk;
            }
        } else {
            if (count <= chunk->size) {
                chunk->size = count;
                memset((uint8_t *) memblock + chunk->size, FENCE, FENCE_SIZE);
                chunk->hash = generate_chunk_hash(chunk);
                return memblock;
            } else {

                int heap_size =
                        (uint8_t *) chunk + CHUNK_SIZE + chunk->size + (2 * FENCE_SIZE) -
                        (uint8_t *) memory_manager.memory_start;
                if (memory_manager.memory_size - heap_size < count + CHUNK_SIZE + (2 * FENCE_SIZE)) {
                    void *temp = custom_sbrk(
                            (long) count - chunk->size);
                    if (temp == (void *) -1) {
                        return NULL;
                    }
                    memory_manager.memory_size += count - chunk->size;
                }
                memset((uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE + count, FENCE,
                       FENCE_SIZE);
                chunk->size = count;
                chunk->hash = generate_chunk_hash(chunk);

                return memblock;

            }
        }

    }
    return NULL;
}


void heap_free(void *memblock) {
    if (!memblock || heap_validate() == 1) return;

    struct memory_chunk_t *chunk = memory_manager.first_memory_chunk;

    while (1) {
        if (chunk == NULL) {
            break;
        }

        if ((uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE == (uint8_t *) memblock) {
            chunk->free = 1;
            if (chunk->next) {
                chunk->size = (int) ((uint8_t *) chunk->next - (uint8_t *) chunk - CHUNK_SIZE);
            }
            while (1) {
                if (chunk->next == NULL || chunk->next->free == 0) {
                    break;
                }
                chunk->size = chunk->size + chunk->next->size + (FENCE_SIZE * 2);
                chunk->next = chunk->next->next;
                chunk->next->prev = chunk;
                chunk->hash = generate_chunk_hash(chunk);
            }
            while (1) {
                if (chunk->prev == NULL || chunk->prev->free == 0) {
                    break;
                }
                chunk->prev->size = chunk->prev->size + chunk->size + (FENCE_SIZE * 2);
                if (chunk->next != NULL) {
                    chunk->prev->next = chunk->next;
                    chunk->prev->hash = generate_chunk_hash(chunk->prev);
                    chunk->next->prev = chunk->prev;
                    chunk->next->hash = generate_chunk_hash(chunk->next);
                } else {
                    chunk->prev->next = NULL;
                    chunk->prev->hash = generate_chunk_hash(chunk->prev);
                }
                chunk = chunk->prev;
            }
            if (memory_manager.first_memory_chunk->next == NULL) {
                memory_manager.first_memory_chunk = NULL;
                struct memory_chunk_t *temp = (struct memory_chunk_t *) memblock;
                temp = NULL;
                ((void) temp);
            }
            chunk = memory_manager.first_memory_chunk;
            if (chunk != NULL) {
                while (1) {
                    if (chunk->next == NULL) {
                        break;
                    }
                    chunk = chunk->next;
                }
                while (1) {
                    if (chunk->prev == NULL || chunk->free == 0) {
                        break;
                    }
                    if (chunk->free == 1) {
                        chunk = chunk->prev;
                        chunk->next = NULL;
                        chunk->hash = generate_chunk_hash(chunk);
                    }
                }
            }
            break;
        } else {
            chunk = chunk->next;
        }
    }
    chunk = memory_manager.first_memory_chunk;
    while (1) {
        if (chunk == NULL) {
            break;
        }
        chunk->hash = generate_chunk_hash(chunk);
        chunk = chunk->next;
    }
}

size_t heap_get_largest_used_block_size(void) {
    if (heap_validate() == 1 || memory_manager.first_memory_chunk == NULL) {
        return 0;
    }

    size_t max_size = 0;
    struct memory_chunk_t *chunk = memory_manager.first_memory_chunk;
    while (1) {
        if (chunk->size > max_size && chunk->free == 0) {
            max_size = chunk->size;
        }
        if (chunk->next == NULL) {
            return max_size;
        }
        chunk = chunk->next;
    }
    return 0;
}

enum pointer_type_t get_pointer_type(const void *const pointer) {
    if (pointer == NULL) {
        return pointer_null;
    }
    struct memory_chunk_t *chunk = memory_manager.first_memory_chunk;
    if (chunk == NULL) {
        return pointer_unallocated;
    }
    if ((uint8_t *) pointer < (uint8_t *) memory_manager.first_memory_chunk) {
        return pointer_unallocated;
    }

    while (chunk != NULL) {
        if ((uint8_t *) pointer == (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE && chunk->free == 0) {
            return pointer_valid;
        } else if ((uint8_t *) pointer == (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE && chunk->free == 1) {
            return pointer_unallocated;
        } else if ((uint8_t *) pointer >= (uint8_t *) chunk &&
                   (uint8_t *) pointer < (uint8_t *) chunk + CHUNK_SIZE &&
                   chunk->free == 0) {
            return pointer_control_block;
        } else if ((uint8_t *) pointer >= (uint8_t *) chunk + CHUNK_SIZE &&
                   (uint8_t *) pointer < (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE && chunk->free == 0) {
            return pointer_inside_fences;
        } else if ((uint8_t *) pointer >= (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE + chunk->size &&
                   (uint8_t *) pointer < (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE + chunk->size + FENCE_SIZE &&
                   chunk->free == 0) {
            return pointer_inside_fences;
        } else if ((uint8_t *) pointer >= (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE &&
                   (uint8_t *) pointer < (uint8_t *) chunk + CHUNK_SIZE + FENCE_SIZE + chunk->size &&
                   chunk->free == 0) {
            return pointer_inside_data_block;
        }

        chunk = chunk->next;
    }

    return pointer_unallocated;
}

int heap_validate(void) {
    if (memory_manager.memory_start == NULL) {
        return 2;
    }
    struct memory_chunk_t *chunk = memory_manager.first_memory_chunk;

    while (1) {
        if (chunk == NULL) {
            break;
        }
        if (generate_chunk_hash(chunk) != chunk->hash) {
            return 3;
        }
        if (chunk->free == 0) {
            uint8_t *temp = (uint8_t *) chunk + CHUNK_SIZE;
            for (size_t i = 0; i < FENCE_SIZE; ++i) {
                if (*((uint8_t *) (temp) + i) != '#') {
                    return 1;
                }
            }

            temp = (uint8_t *) chunk + CHUNK_SIZE + chunk->size + FENCE_SIZE;
            for (size_t i = 0; i < FENCE_SIZE; ++i) {
                if (*((uint8_t *) (temp) + i) != '#') {
                    return 1;
                }
            }
        }
        chunk = chunk->next;
    }
    return 0;
}
