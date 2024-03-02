queue-test: tsl.o queue-test.o
	gcc tsl.o queue-test.o -o queuetest

tsl.o: tsl.c tsl.h
	gcc -c tsl.c -o tsl.o

queue-test.o: tests/queue-test.c tsl.h
	gcc -c tests/queue-test.c -o queue-test.o -I.

