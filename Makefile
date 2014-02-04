all: ece353sh

clean:
	rm -f *.o ece353sh

main.o: main.c
	gcc -g -c -Wall main.c

ece353sh: main.o
	gcc -g -Wall -o ece353sh main.o

