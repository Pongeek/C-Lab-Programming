#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS 

#define MAX_BYTES 512
int my_bcmp(const void* b1, const void* b2, size_t len);

int main(void)
{
    /* declare variables */
    size_t len;
    int index1, index2;
    char* b1, * b2;
    char result;

    /* get & validate len argument for my_bcmp() */
    printf("Please enter len (number of bytes to be compared):\n");
    scanf("%u", &len);
    if (len > MAX_BYTES)
    {
        printf("INVALID LEN INPUT: NEED A POSITIVE NUMBER OR LESS THAN %d\n",MAX_BYTES);
        return EXIT_FAILURE;
    }

    /* get & validate index1 argument for my_bcmp() */
    printf("Please enter index1 (starts from index 0):\n");
    scanf("%d", &index1);
    if (MAX_BYTES < (len + index1) || index1 < 0)
    {
        printf("INVALID INDEX1 INPUT: NEED A POSITIVE NUMBER OR LESS THAN %d BYTES\n", MAX_BYTES);
        return EXIT_FAILURE;
    }

    /* get & validate index2 argument for my_bcmp() */
    printf("Please enter index2 (starts from index 0):\n");
    scanf("%d", &index2);
    if (MAX_BYTES < (len + index2))
    {
        printf("INVALID INDEX2 INPUT: NEED A POSITIVE NUMBER OR LESS THAN %d BYTES\n", MAX_BYTES);
        return EXIT_FAILURE;
    }

/* get & validate str argument for my_bcmp() */
    char str[MAX_BYTES];
    printf("\nPlease enter a line of text: ");
    fgets(str, sizeof(str), stdin);

    if (fgets(str, sizeof(str), stdin) == NULL)
    {
        printf("Error reading line of text!\n");
        return EXIT_FAILURE;
    }
    printf("\nYou entered: %s", str);

    /* check that str contains non-whitespace characters */
    if (strspn(str, " \t\n") == strlen(str))
    {
        printf("INVALID INPUT: THE LINE MUST CONTAIN NON-WHITESPACE CHARACTERS\n");
        return EXIT_FAILURE;
    }

    /* set up pointers to blocks of memory to compare */
    b1 = &str[index1];
    printf("index1 is: %.*s\n", (int)len, b1);
    b2 = &str[index2];
    printf("index 2 is: %.*s\n", (int)len, b2);
 
    /* compare blocks of memory */
    result = my_bcmp(b1, b2, len);
    /* print results */
    if (result == 0)
    {
        printf("\nThe indices are equal!");
    }
    if (result != 0)
    {
        printf("\nThe indices are not equal!");
    }

    return EXIT_SUCCESS;
}
