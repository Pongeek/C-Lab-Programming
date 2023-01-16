/*Function that prints binary number*/
void base(unsigned n) 
{
	unsigned i;
	printf("Your number in base 2 is: ");
	for (i = 1 << 31; i > 0; i = i / 2)
		(n & i) ? printf("1") : printf("0");
}
