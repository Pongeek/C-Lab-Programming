isSequence: prog1.o findSequence.o
	gcc -ansi -Wall -g prog1.o findSequence.o -o isSequence
prog1.o: prog1.c findSequence.c
	gcc -c -ansi -Wall prog1.c -o prog1.o
findSequence: prog1.c
	gcc -c -ansi -Wall findSequence.c -o findSequence.o	
