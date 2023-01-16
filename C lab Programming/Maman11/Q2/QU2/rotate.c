#define INT_BITS 32
/*Function that rotates bits to Left*/
int leftRotate(int n, unsigned int d)
{
	/* In n<<d, last d bits are 0. To put first 3 bits of n at
	  last, do bitwise or of n<<d with n >>(INT_BITS - d) */
	return (n << d) | (n >> (INT_BITS - d));
}
/*Function that rotates bits to Right*/
int rightRotate(int n, unsigned int d)
{
	/* In n>>d, first d bits are 0. To put last 3 bits of at
	  first, do bitwise or of n>>d with n <<(INT_BITS - d) */
	return (n >> d) | (n << (INT_BITS - d));
}
