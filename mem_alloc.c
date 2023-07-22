#include <unistd.h>  // For sbrk()
#include "mem_alloc.h"
#include <assert.h>  // for assert()
#include <string.h>  // for memset() and memcpy()


Block* base = NULL;
Block* limit = NULL;

Block* expand_heap(size_t size) {
    Block* block;
    block = sbrk(0);
    void* request = sbrk(size + sizeof(Block));
    assert((void*)block == request);  // Not thread safe.
    if (request == (void*)-1) {
        return NULL;  // sbrk failed.
    }
    else {
        assert(block);  // Not thread safe.
        block->size = size;
        block->next = NULL;
        block->free = 0;
        block->magic = MAGIC;
        if (base) {   // NULL on first request.
            assert(base->next == NULL);  // Not thread safe.
            base->next = block;
        }
        base = block;
        return block;
    }
}

void* my_malloc(size_t size) {
    Block* block;
    if (size <= 0) {
        return NULL;
    }

    // First time initialization
    if (!base) {
        block = expand_heap(size);
        if (!block) {
            return NULL;
        }
        else {
            base = block;
            limit = block;
        }
    }
    else {
        block = limit;
        Block* last = block;
        while (block && !(block->free && block->size >= size)) {
            last = block;
            block = block->next;
        }
        if (!block) {
            block = expand_heap(size);
            if (!block) {
                return NULL;
            }
            last->next = block;
        }
        else {
            block->free = 0;
            block->magic = MAGIC;
        }
    }
    return(block + 1);
}

void my_free(void* ptr) {
    if (!ptr) {
        return;
    }
    Block* block;
    block = (Block*)ptr - 1;
    if (block->magic != MAGIC) {
        return;
    }
    block->free = 1;
}

void* my_calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void* block = my_malloc(total);
    if (!block) {
        return NULL;
    }
    memset(block, 0, total);
    return block;
}

void* my_realloc(void* ptr, size_t size) {
    if (!ptr || size <= 0) {
        return NULL;
    }
    Block* block = (Block*)ptr - 1;
    if (block->magic != MAGIC) {
        return NULL;
    }
    if (block->size >= size) {
        return ptr;
    }
    else {
        void* new_ptr;
        new_ptr = my_malloc(size);
        if (!new_ptr) {
            return NULL;
        }
        memcpy(new_ptr, ptr, block->size);
        my_free(ptr);
        return new_ptr;
    }
}
