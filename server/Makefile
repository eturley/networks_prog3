all: myftpd

myftpd: tcpserver.o
	gcc tcpserver.o -lm -o myftpd

tcpserver.o: tcpserver.c
	gcc -g -c tcpserver.c -o tcpserver.o

clean:
	rm -f tcpserver.o myftpd