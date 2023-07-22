#include <assert.h>  // for assert()
#include <unistd.h>  // For sbrk()
#include <string.h>  // For memset()
#include "mem_alloc.h"

Block* base = NULL;
Block* limit = NULL;

pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;

Block* expand_heap(size_t size) {
    pthread_mutex_lock(&malloc_lock);
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
        pthread_mutex_unlock(&malloc_lock);
        return block;
    }
}

void* my_malloc(size_t size) {
    Block* block;
    if (size <= 0) {
        return NULL;
    }

    // First time initialization
    pthread_mutex_lock(&malloc_lock);
    if (!base) {
        block = expand_heap(size);
        if (!block) {
            pthread_mutex_unlock(&malloc_lock);
            return NULL;
        }
        else {
            base = block;
            limit = block;
        }
    }
    else {
        block = find_and_remove_best_fit(size);
        if (!block) {
            block = expand_heap(size);
            if (!block) {
                pthread_mutex_unlock(&malloc_lock);
                return NULL;
            }
        }
        else {
            block->free = 0;
            block->magic = MAGIC;
        }
    }
    pthread_mutex_unlock(&malloc_lock);
    return(block + 1);
}

void my_free(void* ptr) {
    if (!ptr) {
        return;
    }
    pthread_mutex_lock(&malloc_lock);
    Block* block;
    block = (Block*)ptr - 1;
    if (block->magic != MAGIC) {
        pthread_mutex_unlock(&malloc_lock);
        return;
    }
    block->free = 1;
    coalesce();
    pthread_mutex_unlock(&malloc_lock);
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
    pthread_mutex_lock(&malloc_lock);
    Block* block = (Block*)ptr - 1;
    if (block->magic != MAGIC) {
        pthread_mutex_unlock(&malloc_lock);
        return NULL;
    }
    if (block->size >= size) {
        pthread_mutex_unlock(&malloc_lock);
        return ptr;
    }
    else {
        void* new_ptr;
        new_ptr = my_malloc(size);
        if (!new_ptr) {
            pthread_mutex_unlock(&malloc_lock);
            return NULL;
        }
        memcpy(new_ptr, ptr, block->size);
        my_free(ptr);
        pthread_mutex_unlock(&malloc_lock);
        return new_ptr;
    }
}

void coalesce() {
    Block* current = base;
    while (current && current->next) {
        if (current->free && current->
            next->free) {
            current->size += current->next->size + sizeof(Block);
            current->next = current->next->next;
        }
        else {
            current = current->next;
        }
    }
}

Block* find_best_fit(size_t size) {
    Block* current = base;
    Block* best_fit = NULL;
    while (current) {
        if (current->free && current->size >= size &&
            (!best_fit || current->size < best_fit->size)) {
            best_fit = current;
        }
        current = current->next;
    }
    return best_fit;
}

Block* find_and_remove_best_fit(size_t size) {
    Block* best_fit = find_best_fit(size);
    if (best_fit) {
        best_fit->free = 0;
        // Split block if enough space
        if (best_fit->size >= size + sizeof(Block) + 1) {
            Block* new_block = (Block*)((char*)(best_fit + 1) + size);
            new_block->size = best_fit->size - size - sizeof(Block);
            new_block->next = best_fit->next;
            new_block->free = 1;
            new_block->magic = MAGIC;
            best_fit->size = size;
            best_fit->next = new_block;
        }
    }
    return best_fit;
}
