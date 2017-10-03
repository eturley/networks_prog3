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
#define MAX_LINE 4096
#define MAX_PENDING 5

int main(int argc, char * argv[]) {
    
	
	char timeStamp[20];
	time_t ltime;
	struct tm *Tm;
	struct timeval tv;
	int stampLength, i;
	struct sockaddr_in sin, client_addr;
	char buf[MAX_LINE];
	int len, addr_len,s, new_s;
	int portNum; int messageLength;
	char *fullMessage;
	char encMessage[4096];
	char *encKey;
	char *opt;
	
	
	if (argc == 2){
		portNum = atoi(argv[1]);
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
	if ((s = socket(PF_INET, SOCK_STREAM,0)) < 0) {
		perror("simplex-talk:socket");
	}

	/* set socket option*/
	if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(int))) < 0) {
		perror("simplex-talk:setsockopt\n");
		exit(1);
	}
	
	if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("simplex-talk: bind");
		exit(1);
	}
	
	addr_len = sizeof(sin);

	if((listen(s, MAX_PENDING)) < 0) {
		perror("simplex-talk: bind\n");
	}
	while (1) {
		if((new_s = accept(s, (struct sockaddr *)&sin, &addr_len)) < 0) {
			perror("simplex-talk: accept\n");
			exit(1);
		}

		if((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
			perror("Server Received Error!\n");
			exit(1);
		}
		if(len == 0) break;
		printf("TCP Server Received: %s\n", buf);

		if(strcmp(buf, "DWNLD") == 0) {
			// do download stuff
		}
		else if(strcmp(buf, "UPLD") == 0) {
			// upload
		}
		else if(strcmp(buf, "DELF") == 0) {
			// delete file
		}
		else if(strcmp(buf, "LIST") == 0) {
			// ls
		}
		else if(strcmp(buf, "MDIR") == 0) {
			// mkdir
		}
		else if(strcmp(buf, "RDIR") == 0) {
			// rmdir
		}
		else if(strcmp(buf, "CDIR") == 0) {
			// cd
		}
		else if(strcmp(buf, "QUIT") == 0) {
			// quit
		}
		else {
			printf("Invalid Command\n");
		}
		
		/*create time stamp*/
		//ltime=time(NULL);
		//Tm=localtime(&ltime);
		//printf("%d:%d:%d\n", Tm->tm_hour, Tm->tm_min, Tm->tm_sec);
		//gettimeofday(&tv, NULL);
		//sprintf(timeStamp, "%d:%d:%d.%d", Tm->tm_hour, Tm->tm_min, Tm->tm_sec, tv.tv_usec);
		bzero((char*)&buf,sizeof(buf));
	}
	if (close(s) != 0) {
        perror("Was not closed!\n");
    }
} 

