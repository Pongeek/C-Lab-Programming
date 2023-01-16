#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char findSequence(char str[])
{
    int len = strlen(str);
    int k, i;
    int j = 0;
    char dst[100];

    for (i = 0; str[i] != '\0'; i++);
    {
        dst[i] = str[i];
    }

    for (i = 0; i < len; i++)
    {
        for (k = i; k < len; k++) /*loop that checks the sequence and stops whenever the sequence ends*/
        {
            if (str[k] >= 47 && str[k] <= 57) /*breaks the loop if we get numbers*/
                break;
            if (str[k + 1] != str[k] + 1)
                break; /*breaks if next letter in the string is not a sequence*/
        }
        if (k >= i + 2) /*if there is at least a sequence of 3 letters*/
        {
            /*adds '-' between the first and the last letters in a sequence*/
            dst[j++] = str[i];
            dst[j++] = '-';
            dst[j++] = str[k];
            i = k;
        }
        else
            dst[j++] = str[i]; 
    }
    dst[j] = '\0';

    printf("Output: %s\n", dst);
    return(0);



}
