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
#include <sys/stat.h>
#include <fcntl.h>
#define MAX_LINE 4096

char *trim(char *s) {
	//int i = strlen(s);
	//if((i>0) && (s[i]=='\n'))
	//	s[i]=='\0';
	//return s;
	return strtok(s, "\n");
}

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
    int s, len, ServerPort, fd, bytes_read, bytes_written;

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
	    bzero((char *)& buf, sizeof(buf));
		printf("Enter operation: ");
		fgets(buf, sizeof(buf), stdin);
		buf[MAX_LINE-1] = '\0';
		if(!strncmp(buf, "QUIT\n", 5)) {
		    len = strlen(buf) + 1;
			if(send(s, buf, len, 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
		    }
			break;
		} 
		
		//LIST
		else if(strcmp(buf, "LIST\n")==0){
		    len = strlen(buf) + 1;
		    if(send(s, buf, len, 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
		    }
		    bzero((char *)& buf, sizeof(buf));
		    if(recv(s, buf, sizeof(buf), 0) == -1) {
			    perror("client receive error");
			    exit(1);
		    }
		    printf("%s", buf);
		    bzero((char *)& buf, sizeof(buf));
		} 
		
		//DELF
		else if(strcmp(buf, "DELF\n") == 0) {
		  int confirm, c;
		  len = strlen(buf) + 1;
		  if(send(s, buf, len, 0) == 1){
		    perror("client send error!\n");
		    exit(1);
		  }
		  bzero((char *)& buf, sizeof(buf));
		  printf("Enter length of file name: ");
		  fgets(buf, sizeof(buf), stdin);
		  len = strlen(buf) + 1;
		  c = htons(atoi(buf));
		  if(send(s, &c, sizeof(c), 0) == -1){
		    perror("client send error!\n");
		    exit(1);
		  }
		  bzero((char *)& buf, sizeof(buf));
		  printf("Enter name of file: ");
		  fgets(buf, sizeof(buf), stdin);
		  len = strlen(buf) + 1;
		  if(send(s, buf, len, 0) == -1){
		    perror("client send error!\n");
		    exit(1);
		  }
		  bzero((char *)& buf, sizeof(buf));
		  if(recv(s, &confirm, sizeof(confirm), 0) == -1){
		    perror("client receive error!\n");
		    exit(1);
		  }

		  //receive numerical confirmation from server
		  confirm = ntohl(confirm);
		  if(confirm  == -1) {
		    printf("The file does not exist on server\n");
		    continue;
		  } 
		  else if (confirm == 1) {
		    printf("Are you sure you want to delete the file? (Yes/No) ");
		    fgets(buf, sizeof(buf), stdin);
		    
		    if (strcmp(buf, "Yes\n") == 0) {
		      if(send(s, "Yes", strlen("Yes") + 1, 0) == -1) {//send the confirm
			perror("client send error!\n");
			exit(1);
		      }		      
		      bzero((char *)& buf, sizeof(buf));
		      //wait for acknowledgement of deletion
		      if(recv(s, buf, sizeof(buf), 0) == -1) {
			perror("client receive error");
			exit(1);
		      }
		      
		      if(strcmp(buf,"Fail") == 0) {
			printf("Failed to delete file\n");
			continue;
		      } else if (strcmp(buf, "Success") == 0) {
			printf("File deleted\n");
			continue;
		      }
		      
		      
		    } else if (strcmp(buf, "No\n") == 0 ) {
		      if(send(s, "No", strlen("No") + 1, 0) == -1) {
			perror("client send error!\n");
			exit(1);
		      }
		      
		      printf("Delete abandoned by the user!\n");
		      continue;
		    } 
		  }
		}
		
		//MDIR
		else if(strcmp(buf, "MDIR\n") == 0) {
		  int confirm, c;
		  len = strlen(buf) + 1;
		  //tell server command is MDIR
		  if(send(s, buf, len, 0) == -1){
		    perror("client send error!\n");
		    exit(1);
		  }
		  bzero((char *)& buf, sizeof(buf));
		  printf("Enter length of directory name: ");
		  fgets(buf, sizeof(buf), stdin);
		  len = strlen(buf) + 1;
		  c = atoi(buf);
		  if(send(s, &c, sizeof(c), 0) == -1){
		    perror("client send error!\n");
		    exit(1);
		  }
		  bzero((char *)& buf, sizeof(buf));
		  printf("Enter name of new directory: ");
		  fgets(buf, sizeof(buf), stdin);
		  len = strlen(buf) + 1;
		  if(send(s, buf, len, 0) == -1) {
		    perror("client send error!\n");
		    exit(1);
		  }
		  bzero((char *)& buf, sizeof(buf));
		  //receive numerical confirmation from server
		  if(recv(s, &confirm, sizeof(confirm), 0) == -1) {
		    perror("client receive error!\n");
		    exit(1);
		  }
		  if(confirm == -1){
		    printf("Error in creating directory\n");
		  } else if(confirm == -2){
		    printf("The directory already exists on server\n");
		  } else {
		    printf("Directory created successfully\n");
		  }
		} 
		
		//RDIR
		else if(strcmp(buf, "RDIR\n") == 0) {
		    int confirm; int c;
		    len = strlen(buf) + 1;
		    //send RDIR initial message
		    if(send(s, buf, len, 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
		    }
		    
		    bzero((char *)& buf, sizeof(buf));
		    
		    printf("Enter Length of directory name: ");
		    fgets(buf, sizeof(buf), stdin);
		    
		    len = strlen(buf) + 1;
		    //send length of dir
		    c = htons(atoi(buf));
		    if(send(s, &c, sizeof(c), 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
		    }
		    
		    bzero((char *)& buf, sizeof(buf));
		    printf("Enter directory name: ");
		    fgets(buf, sizeof(buf), stdin);
		    printf("23\n");
		    len = strlen(buf) + 1;
		    //send dir name
		    if(send(s, buf, len, 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
		    }
		    
		    bzero((char *)& buf, sizeof(buf));
		    
		    // receive confirm from server
		    if(recv(s, &confirm, sizeof(confirm), 0) == -1) {
			    perror("client receive error");
			    exit(1);
		    }
		    
		    confirm = ntohl(confirm);
		   
		   printf("confirm: %d\n",confirm);
		   if(confirm  == -1) {
		        printf("The directory does not exist on server\n");
		        continue;
		    } else if (confirm == 1) {
		        printf("Are you sure you want to delete the directory? (Yes/No) ");
		        fgets(buf, sizeof(buf), stdin);
		        
		        if (strcmp(buf, "Yes\n") == 0) {
		            if(send(s, "Yes", strlen("Yes") + 1, 0) == -1) {//send the confirm
			            printf("yooo\n");
			            perror("client send error!\n");
			            exit(1);
		            }
		            
        		    bzero((char *)& buf, sizeof(buf));
		            //wait for acknowledgement of deletion
		            printf("what's up\n");
		            if(recv(s, buf, sizeof(buf), 0) == -1) {
			            perror("client receive error");
			            exit(1);
		            }
		            
		            //let user know if it worked and return to prompt
		            if(strcmp(buf,"Fail") == 0) {
		                printf("Failed to delete directory\n");
		                continue;
		            } else if (strcmp(buf, "Success") == 0) {
		                printf("Directory deleted\n");
		                continue;
		            }
		            
		            
		        } else if (strcmp(buf, "No\n") == 0 ) { //user changed their mind
		            if(send(s, "No", strlen("No") + 1, 0) == -1) {
			            perror("client send error!\n");
			            exit(1);
		            }
		            
		            printf("Delete abandoned by the user!\n");
		            continue;
		       }
		    } 
		}
		len = strlen(buf) + 1;
		if(send(s, buf, len, 0) == -1) {
			perror("client send error!\n");
			exit(1);
		}

		//DWLD
		if(!strncmp(buf, "DWLD", 4)) {
			char fn[MAX_LINE];
			char fn_len[MAX_LINE];
			printf("Filename length: ");
			fgets(fn_len, sizeof(fn_len), stdin);
			printf("Filename: ");
			fgets(fn, sizeof(fn), stdin);

			/* Send length of filename */
			if(send(s, fn_len, strlen(fn_len)+1, 0) == -1) {
				perror("client filename length send error!\n");
				exit(1);
			}
			/* Send filename */
			if(send(s, fn, strlen(fn)+1, 0) == -1) {
				perror("client filename send error!\n");
				exit(1);
			}

			/* Get file size */
			int32_t fsize;
			char *sizedata = (char *)&fsize;
			if(recv(s, sizedata, sizeof(sizedata), 0) == -1) {
				perror("client file size recv error");
			}
			if(ntohl(fsize)==-1) {
				printf("File does not exist on %s\n", host);
				continue;
			}
			else { // receive and save file
				fn[strlen(fn)-1]='\0'; // get rid of trailing newline for filename
				fd = open(fn, O_CREAT | O_RDWR, S_IRUSR|S_IWUSR);
				bytes_written=0;
				int bytes=0;
				bytes_read = 0;
				while(1) {
					bzero((char *)&buf, sizeof(buf));
					if(bytes_written == ntohl(fsize))
						break;
					bytes_read = read(s, buf, sizeof(buf));
					if(bytes_read < 0) {
						perror("read bytes error");
						exit(1);
					}

					void *p = buf;
					while(bytes_read > 0) {
						bytes = write(fd, p, bytes_read);
						if(bytes<=0) {
							perror("write bytes error");
							exit(1);
						}
						bytes_read -= bytes;
						p += bytes;
						bytes_written += bytes;
					}
				}
				printf("Successfully downloaded file\n");
				bzero((char *)&buf, sizeof(buf));
			}
		}
	
		/*if(!strncmp(buf, "LIST", 4)) {
			if(recv(s, buf, sizeof(buf), 0) == -1) {
				perror("client receive error");
				exit(1);
			}
			printf("%s", buf);
			bzero((char *)& buf, sizeof(buf));
			}*/
		
		//CDIR
		else if(strcmp(buf, "CDIR\n") == 0) {
			int confirm; int c;
		    len = strlen(buf) + 1;
		    //send RDIR initial message
		    if(send(s, buf, len, 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
		    }
		    
		    bzero((char *)& buf, sizeof(buf));
		    
		    printf("Enter Length of directory name: ");
		    fgets(buf, sizeof(buf), stdin);
		    
		    len = strlen(buf) + 1;
		    //send length of dir
		    c = atoi(buf);
		    if(send(s, &c, sizeof(c), 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
		    }
		    
		    bzero((char *)& buf, sizeof(buf));
		    printf("Enter directory name: ");
		    fgets(buf, sizeof(buf), stdin);
		    printf("23\n");
		    len = strlen(buf) + 1;
		    //send dir name
		    if(send(s, buf, len, 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
		    }
		    
		    bzero((char *)& buf, sizeof(buf));
		    
		     // receive confirm from server
		    if(recv(s, &confirm, sizeof(confirm), 0) == -1) {
			    perror("client receive error");
			    exit(1);
		    }
		    
		    if(confirm == 1){
		     	printf("Changed current directory\n");
		     	continue;
		    } else if (confirm == -1) {
		    	printf("Error changing directory\n");
		    	continue;
		    } else if (confirm == -2) {
		    	printf("The directory does not exist on server\n");
		    	continue;
		    }
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

