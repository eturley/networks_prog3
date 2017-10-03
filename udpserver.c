//Marco Bruscia
//netid: jbruscia
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#define SERVER_PORT 41100
#define MAX_LINE 4096
#define MAX_KEY 16


int main(int argc, char * argv[]) {
    
	
	char timeStamp[20];
	time_t ltime;
	struct tm *Tm;
	struct timeval tv;
	int stampLength, i;
	struct sockaddr_in sin, client_addr;
	char buf[MAX_LINE];
	int len, addr_len,s;
	int portNum; int messageLength;
	char key[MAX_KEY];
	char *fullMessage;
	char encMessage[4096];
	char *encKey;
	
	
	if (argc == 3){
		portNum = atoi(argv[1]);
		encKey = argv[2];
		printf("%d\n",portNum);
	}
	else {
		printf("Incorrect number of arguments. Usage: \n ./main [port number] [encryptionkey] \n");
		exit(1);
	}
	
    /* build address data structure */ 
	bzero ((char *)&sin, sizeof(sin)); 
	sin.sin_family = AF_INET; 
	sin.sin_addr.s_addr = INADDR_ANY; //Use the default IP address of server 
	sin.sin_port = htons(portNum); 
	
	
	/* setup passive open*/
	if ((s = socket(PF_INET, SOCK_DGRAM,0)) < 0) {
		perror("simplex-talk:socket");
	}
	
	if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("simplex-talk: bind");
		exit(1);
	}
	
	addr_len = sizeof(sin);
	
	while (1) {
		/* receive initial message */
		if (recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &sin, &addr_len)==-1) {
			perror("Receive error!\n");
			exit(1);
		}
		
		
		bzero((char*)&buf,sizeof(buf));
		
		
		//send encryption key
		if (sendto(s, encKey, strlen(encKey) + 1, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
		    perror ("Error sending from client!\n");
		    exit(1);
		}
		
		/* receive message to encrypt*/
		if (messageLength = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &sin, &addr_len)==-1) {
			perror("Receive error!\n");
			exit(1);
		}
		
		/*create time stamp*/
		ltime=time(NULL);
		Tm=localtime(&ltime);
		printf("%d:%d:%d\n", Tm->tm_hour, Tm->tm_min, Tm->tm_sec);
		gettimeofday(&tv, NULL);
		sprintf(timeStamp, "%d:%d:%d.%d", Tm->tm_hour, Tm->tm_min, Tm->tm_sec, tv.tv_usec);
		
		
		/* create message to encrypt */
		messageLength = sprintf(encMessage, "%s Timestamp: %s", buf, timeStamp);
		
		
		/* encrypt message */
		for (i=0; i < messageLength; i+=1) {
			encMessage[i] = encMessage[i]^encKey[i%(strlen(encKey)+1)];
		}
		
		/* send encrypted message back to client */
		if (sendto(s, encMessage, messageLength, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
		    perror ("Error sending from client!\n");
		    exit(1);
		}
		
		
		bzero((char*)&buf,sizeof(buf));
		
		
	}
	if (close(s) != 0) {
        perror("Was not closed!\n");
    }
} 

