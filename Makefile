build: tsl.o lib main.o main
tsl.o: tsl.c tsl.h
	gcc -m32 -D_GNU_SOURCE -c -nostartfiles tsl.c -o tsl.o

lib: tsl.o
	ar rcs libtsl.a tsl.o

main.o: main.c
	gcc -m32 -D_GNU_SOURCE -c -nostartfiles main.c -o main.o -L. -ltsl

main: tsl.o main.o
	gcc -m32 tsl.o main.o -o main
	rm tsl.o main.o
	clear
	
clean:
	rm -f *.o *.a main
	clear