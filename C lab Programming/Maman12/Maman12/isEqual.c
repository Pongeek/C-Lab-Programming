#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

int my_bcmp(const void* b1, const void* b2, size_t len)
{
    // Cast the pointers to unsigned char pointers
    const unsigned char* p1 = (const unsigned char*)b1;
    const unsigned char* p2 = (const unsigned char*)b2;

    // Iterate through the blocks of memory
    for (size_t i = 0; i < len; ++i)
    {
        // If the bytes are not equal, return the difference between them
        if (p1[i] != p2[i])
        {
            return 1;
        }
    }

    // If the loop completes, the blocks of memory are equal
    return 0;
}
