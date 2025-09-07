#ifndef SAFE_ALLOCATIONS_H
#define SAFE_ALLOCATIONS_H

#include <stdlib.h>

/**
 * A safe version of malloc, when there is an error, just exit.
 *
 * @note the exact same as malloc but with error raise on error.
*/
void * safe_malloc(size_t __size);

/**
 * A safe version of realloc, when there is an error, just exit.
 *
 * @note the exact same as realloc but with error raise on error.
*/
void *safe_realloc(void *__ptr, size_t __size);

/**
 * A safe version of calloc, when there is an error, just exit.
 *
 * @note the exact same as calloc but with error raise on error.
*/
void *safe_calloc(size_t __nmemb, size_t __size);

#endif /*SAFE_ALLOCATIONS_H*/