#include <stdio.h>

char findSequence(char str[]);

int main()
{
	char string[100];

	printf("Enter a string:\n");
	fgets(string, sizeof(string), stdin);

	printf("The string you entered is: %s\n", string);
	findSequence(string);
}



