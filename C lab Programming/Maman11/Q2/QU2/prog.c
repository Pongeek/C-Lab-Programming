#include <stdio.h>
#include <string.h>

void base(unsigned n);
int leftRotate(int n, unsigned int d);
int rightRotate(int n, unsigned int d);

int main()
{
	int number, shifts, afterRotation;
	char direction[2];


	printf("Please enter a number:\n");
	scanf("%d", &number);

	printf("Enter side of rotation: type R or L\n");
	scanf("%s", direction, 2);

	printf("Enter a number of rotations:\n");
	scanf("%d", &shifts);

	printf("Following information shows the number you've entered in different bases before rotation:\n");
	printf("Your number in DEC base 10 is: %d\n", number);
	base(number);
	printf("\nYour number in OCT base 8 is: %o\n", number);
	printf("Your number in HEX base 16 is: %x\n", number);

	if (direction[0] == 'L')
		afterRotation = leftRotate(number, shifts);
	else if (direction[0] == 'R')
		afterRotation = rightRotate(number, shifts);

	printf("Following information shows the number you've entered in different bases after rotation:\n");
	printf("Your number in DEC base 10 is: %d\n", afterRotation);
	base(afterRotation);
	printf("\nYour number in OCT base 8 is: %o\n", afterRotation);
	printf("Your number in HEX base 16 is: %x\n", afterRotation);

}
