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
#define SERVER_PORT 40017
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

    if (argc == 4) {
        host = argv[1];
        ServerPort = atoi(argv[2]);
        
        if( access(argv[3], F_OK ) != -1 ) { //if it is a file
            
            fp = fopen(argv[3], "rb");
            if(fp != NULL) {
                fseek( fp, 0L , SEEK_END);
                lSize = ftell( fp );
                rewind( fp );
                if( 1!=fread(buf, lSize, 1, fp)) {
                    fclose(fp);
                    printf("read fails\n");
                    exit(1);
                }
                fclose(fp);
            }
        } else {
            strcpy(buf, argv[3]);
        }
        
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
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation error!");
        exit(1);
    }

    //send arbitrary message to connect
    if (sendto(s, "random message", strlen("random message"), 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
        perror ("Error sending from client!\n");
        exit(1);
    }
    
    
    //recieve decription key
    int length; int keylength;
    if ( (keylength = recvfrom(s, key, sizeof(key), 0,  (struct sockaddr *)&sin, &addr_len))==-1){
            perror("Receive error!\n");
            exit(1);
    }
    
    
    //start timer
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    //send buf message
    if (sendto(s, buf, strlen(buf) + 1, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
        perror ("Error sending from client!\n");
        exit(1);
    } 
    
    
    //receive encrypted
    if ( (length = recvfrom(s, encryptedMessage, sizeof(encryptedMessage), 0,  (struct sockaddr *)&sin, &addr_len))==-1){
            perror("Receive error!\n");
            exit(1);
    }
    
    //calculate time it took
    clock_gettime(CLOCK_MONOTONIC, &tend);
           
    printf("omg");
    //decrypt
    int i;
    for (i = 0; i < length; i += 1)
    {
        encryptedMessage[i] = encryptedMessage[i]^key[i%keylength];
    }
    
    
    //print info to screen
    printf("decrypted response: %s\n", encryptedMessage);
    
    printf("RTT: %.0lf microseconds\n",
           (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
           ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec))*1000000);
    
    if (close(s) != 0) {
        perror("Was not closed!\n");
    }
    
} 

