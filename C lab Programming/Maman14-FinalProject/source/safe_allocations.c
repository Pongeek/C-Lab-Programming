#include <stdio.h>
#include "../headers/safe_allocations.h"

#define RED_COLOR   "\x1B[1;91m"
#define GRN_COLOR   "\x1B[32m"
#define YEL_COLOR   "\x1B[33m"
#define BLU_COLOR   "\x1B[34m"
#define MAG_COLOR   "\x1B[35m"
#define CYN_COLOR   "\x1B[36m"
#define WHT_COLOR   "\x1B[37m"
#define RESET_COLOR "\x1B[0m"

void * safe_malloc(size_t __size) {
    void * out = malloc(__size);

    if (out == NULL) {
        printf("%sMemory Error:%s memory allocator returned NULL.\n", RED_COLOR, RESET_COLOR);
        exit(1);
    }
    
    return out;
}


void *safe_realloc(void *__ptr, size_t __size) {
    void * out = realloc(__ptr, __size);

    if (out == NULL) {
        printf("%sMemory Error:%s memory allocator returned NULL.\n", RED_COLOR, RESET_COLOR);
        exit(1);
    }
    
    return out;
}

void *safe_calloc(size_t __nmemb, size_t __size) {
    void * out = calloc(__nmemb, __size);

    if (out == NULL) {
        printf("%sMemory Error:%s memory allocator returned NULL.\n", RED_COLOR, RESET_COLOR);
        exit(1);
    }
    
    return out;
}