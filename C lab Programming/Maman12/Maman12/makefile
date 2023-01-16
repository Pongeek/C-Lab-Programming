CompareMemoryBlocks: prog.o isEqual.o
	gcc -ansi -Wall -g prog.o isEqual.o -o CompareMemoryBlocks
prog.o: prog.c isEqual.c
	gcc -c -ansi -Wall prog.c -o prog.o
isEqual: prog.c
	gcc -c -ansi -Wall isEqual.c -o isEqual.o	
