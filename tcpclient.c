//Marco Bruscia, Erin Turley, Mike Parowski
//netid: jbruscia, eturley, mparowsk

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#define MAX_LINE 4096


int main(int argc, char * argv[]) {
    
    //initialize timing variables
    struct timespec tstart={0,0}, tend={0,0};
    
    //initialize vaiables
    FILE *fp;
    long lSize;
    struct hostent *hp;
    struct sockaddr_in sin;
    socklen_t addr_len;
    char *host;
    char buf[MAX_LINE];
    char key[MAX_LINE];
    char encryptedMessage[MAX_LINE];
    int s, len, ServerPort;

    if (argc == 3) {
        host = argv[1];
        ServerPort = atoi(argv[2]);
    } else {
        printf("Incorrect amount of arguments. Usage: \n ./main [host name] [port number] [text to send] \n" );
        exit(1);
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }
    
    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(ServerPort);
    addr_len = sizeof(sin);
    
    /* active open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation error!");
        exit(1);
    }

	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("simplex-talk: connect\n");
		close(s);
		exit(1);
	}
	while(1) {
		printf("Enter operation: ");
		fgets(buf, sizeof(buf), stdin);
		buf[MAX_LINE-1] = '\0';
		if(!strncmp(buf, "QUIT", 4)) {
			break;
		}

		len = strlen(buf) + 1;
		if(send(s, buf, len, 0) == -1) {
			perror("client send error!\n");
			exit(1);
		}
		bzero((char *)& buf, sizeof(buf));
	}
    
    if (close(s) != 0) {
        perror("Was not closed!\n");
    }

    ////start timer
    //clock_gettime(CLOCK_MONOTONIC, &tstart);
    ////send buf message
    //if (sendto(s, buf, strlen(buf) + 1, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
    //    perror ("Error sending from client!\n");
    //    exit(1);
    //} 
    //
    ////calculate time it took
    //clock_gettime(CLOCK_MONOTONIC, &tend);
    //
    //printf("RTT: %.0lf microseconds\n",
    //       (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
    //       ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec))*1000000);
} 

