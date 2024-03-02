test: tsl.c tsl.h queue-test.c
	gcc -c tsl.c -o tsl.o
	gcc -c queue-test.c -o queue-test.o
	gcc tsl.o queue-test.o -o queuetest

