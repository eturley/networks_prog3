all: myftp

myftp: tcpclient.o
	gcc tcpclient.o -lm -o myftp

tcpclient.o: tcpclient.c
	gcc -g -c tcpclient.c -o tcpclient.o

clean:
	rm -f tcpclient.o myftp