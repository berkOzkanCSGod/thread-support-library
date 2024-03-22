build: tsl.o lib main
tsl.o: tsl.c tsl.h
	gcc -m32 -D_GNU_SOURCE -c -nostartfiles tsl.c -o tsl.o

lib: tsl.o
	ar rcs libtsl.a tsl.o
main:
	gcc -m32 -D_GNU_SOURCE -nostartfiles -o main main.c -L.  -ltsl
