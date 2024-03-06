tsl.o: tsl.c tsl.h
	gcc -m32 -D_GNU_SOURCE -c -nostartfiles tsl.c -o tsl.o

main.o: main.c
	gcc -m32 -D_GNU_SOURCE -c -nostartfiles main.c -o main.o

main: tsl.o main.o
	gcc -m32 tsl.o main.o -o main
	rm tsl.o main.o
	# clear