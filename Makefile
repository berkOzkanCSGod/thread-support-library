test: tsl.c tsl.h main.c
	gcc -c tsl.c -o tsl.o
	gcc -c main.c -o main.o
	gcc tsl.o main.o -o queuetest

