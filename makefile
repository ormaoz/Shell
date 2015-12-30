all: myshell copier

clean:
	rm myshell copier

myshell: myshell.c
	gcc -Wall -o myshell myshell.c

copier: Copier.c BoundedBuffer.c
	gcc -Wall -pthread -o copier Copier.c BoundedBuffer.c
