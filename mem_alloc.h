#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include <stddef.h>  // For size_t

#define MAGIC 0x12345678

typedef struct Block {
    size_t size;
    struct Block* next;
    int free;
    int magic;
} Block;

void* my_malloc(size_t size);
void* my_calloc(size_t nmemb, size_t size);
void* my_realloc(void* ptr, size_t size);
void my_free(void* ptr);

#endif  // MEM_ALLOC_H
