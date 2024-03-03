tsl.o: tsl.c tsl.h
	gcc -m32 -D_GNU_SOURCE -c -nostartfiles tsl.c -o tsl.o

