#include <stdio.h>
#include "mem_alloc.h"

int main() {
    int* array = my_malloc(10 * sizeof(int));
    if (array == NULL) {
        printf("Memory allocation failed\n");
        return -1;
    }
    for (int i = 0; i < 10; i++) {
        array[i] = i;
    }
    for (int i = 0; i < 10; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
    my_free(array);
    return 0;
}

