//Marco Bruscia, Erin Turley, Michael Parowski
//netid: jbruscia, eturly, mparowsk
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <errno.h>
#define MAX_LINE 4096
#define CHUNK_SIZE 1024
#define MAX_PENDING 5

int fsize(const char *);

int fsize(const char *filename) {
	struct stat st;

	if(stat(filename, &st) == 0)
		return st.st_size;
	return -1;
}

int main(int argc, char * argv[]) {


	char timeStamp[20];
	time_t ltime;
	struct tm *Tm;
	struct timeval tv;
	int stampLength, i;
	struct sockaddr_in sin, client_addr;
	char buf[MAX_LINE];
	int len, addr_len,s, new_s, f, status, fd;
	int portNum, messageLength, bytes_sent;
	char *fullMessage;
	char encMessage[4096];
	char *encKey;
	char *opt;
	DIR *d;
	struct dirent *de;
	struct stat dirbuf;
	int exists;
	int total_size;
	char path[256], perms[MAX_LINE];

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
			bzero((char*)&buf,sizeof(buf));
			if((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
				perror("Server Received Error!\n");
				exit(1);
			}
			if(len == 0) {
				printf("len was zero\n");
				continue;
			}

			if(strcmp(buf, "DWLD\n") == 0) {
				// receive length of filename
				if((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
					perror("filename length recv error");
				}
				short fn_len = (short) atoi(buf);
				printf("Filename length: %d\n", fn_len);
				bzero((char *)& buf, sizeof(buf));

				// receive filename
				if((len = recv(new_s, buf, fn_len, 0)) == -1) {
					perror("filename recv error");
				}
				char filename[MAX_LINE];
				strcpy(filename, buf);
				bzero((char *)& buf, sizeof(buf));
				printf("filename: %s\n", filename);

				if(!access(filename, F_OK)) { // file exists
					int filesize = fsize(filename);
					printf("filesize: %d\n", filesize);
					if(filesize>-1){
						int32_t conv = htonl(filesize);
						char *err = (char*)&conv;
						// send filesize
						if(send(new_s, err, sizeof(err), 0) == -1) {
							perror("file size send failure");
						}
						// send file
						size_t nbytes = 0;
						char file_data[CHUNK_SIZE];
						off_t offset = 0;
						fd = open(filename, O_RDONLY);
						bytes_sent = sendfile(new_s, fd, &offset, filesize);
						if(bytes_sent==-1)
							perror("file send failure");
						if(bytes_sent != filesize)
							perror("bytes sent != filesize");
						printf("bytes sent: %d\n", bytes_sent);
						close(fd);
						close(new_s);
						printf("closed new_s\n");
						break;
					}
					else {
						perror("filesize error");
						close(new_s);
						exit(1);
					}
				}
				else { // file no exists
					int32_t conv = htonl(-1);
					char *err = (char*)&conv;
					if(send(new_s, err, sizeof(err), 0) == -1)
						perror("error code send failure");
					close(new_s);
					break;
				}
			}
			else if(strcmp(buf, "UPLD\n") == 0) {
				// upload
			}
			else if(strcmp(buf, "DELF\n") == 0) {
				// delete file
			}
			else if(strcmp(buf, "LIST\n") == 0) {
				if((d = opendir(".")) == NULL) {
					perror("prsize");
					exit(1);
				}
				total_size = 0;

				perms[0] = '\0';
				while((de = readdir(d)) != 0) {
					strcpy(path, ".");
					strcat(path, "/");
					strcat(path, de->d_name);
					if(!stat(path, &dirbuf)) {
						strcat(perms, (S_ISDIR(dirbuf.st_mode)) ? "d" : "-");
						strcat(perms, (dirbuf.st_mode & S_IRUSR) ? "r" : "-");
						strcat(perms, (dirbuf.st_mode & S_IWUSR) ? "w" : "-");
						strcat(perms, (dirbuf.st_mode & S_IXUSR) ? "x" : "-");
						strcat(perms, (dirbuf.st_mode & S_IRGRP) ? "r" : "-");
						strcat(perms, (dirbuf.st_mode & S_IWGRP) ? "w" : "-");
						strcat(perms, (dirbuf.st_mode & S_IXGRP) ? "x" : "-");
						strcat(perms, (dirbuf.st_mode & S_IROTH) ? "r" : "-");
						strcat(perms, (dirbuf.st_mode & S_IWOTH) ? "w" : "-");
						strcat(perms, (dirbuf.st_mode & S_IXOTH) ? "x" : "-");
					}
					else
						perror("error in stat");
					strcat(perms, "\t");
					strcat(perms, de->d_name);
					strcat(perms, "\n");
				}
				if(send(new_s, perms, strlen(perms)+1, 0) == -1) {
					perror("server send error");
					exit(1);
				}
				bzero((char *)& perms, sizeof(perms));
				closedir(d);
				close(new_s);
				break;
			}
			else if(strcmp(buf, "MDIR\n") == 0) {
			  // mkdir
			  int dir_size, confirm, rc;
			  char dir_name[1024];
			  //length of directory ???
			  if((len = recv(new_s, &dir_size, sizeof(dir_size), 0)) == -1){
			    perror("server received error!\n");
			    exit(1);
			  }
			  bzero((char *)&buf, sizeof(buf));
			  //directory name
			  if((len = recv(new_s, buf, sizeof(buf), 0)) == -1){
			    perror("server received error!\n");
			    exit(1);
			  }
			  strcpy(dir_name, buf);
			  dir_name[strlen(dir_name) - 1] = '\0';
			  printf("directory name: %s\n", dir_name); //debugging

			  //check if directory exists
			  DIR* dir = opendir(dir_name);
			  if(dir) {
			    confirm = -2;
			    closedir(dir);
			  } else {
			    //make directory
			    char cwd[1024];
			    getcwd(cwd, sizeof(cwd));
			    strcat(cwd,"/");
			    strcat(cwd,dir_name);
			    
			    rc = mkdir(cwd, S_ISVTX);
			    if(rc < 0){
			      confirm = -1;
			    } else {
			      confirm = 1;
			    }
			  }
			  if(send(new_s, &confirm, sizeof(confirm), 0) == -1) {
			    perror("client send error!\n");
			    exit(1);
			  }
			}
			else if(strcmp(buf, "RDIR\n") == 0) {
				int dir_size; char dir_name [1024]; int confirm; int r;
				// rmdir
				/* obtain length of directory name and directory name */
				if((len = recv(new_s, &dir_size, sizeof(dir_size), 0)) == -1) {
				    perror("Server Received Error!\n");
				    exit(1);
				}
				dir_size = ntohs(dir_size);
				//dir_size = atoi(buf);
				bzero((char*)&buf,sizeof(buf));
				//receive dir name
				if((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
				  perror("Server Received Error!\n");
				  exit(1);
				}
				//dir_name = buf;
				strcpy(dir_name, buf);
				dir_name[strlen(dir_name)-1] = '\0';
				printf("directory name: %s\n",dir_name);
				
				//check if directory exists
				DIR* dir = opendir(dir_name);
				if (dir) {
				    confirm = 1;
				    closedir(dir);
				} else if (ENOENT == errno ){
				    confirm = -1;
				    printf("Doesn't exist\n");
				} else {
				    perror("Couldn't open directory\n");
				    exit(1);
				}
				
				
				//send confirmation of directory
				confirm = htonl(confirm);
				if(send(new_s, &confirm, sizeof(confirm), 0) == -1) {
			        perror("client send error!\n");
			        exit(1);
		        }
		        /*
		        if(strcmp(confirm, "-1") == 0) {
		            break;
		        }*/
		        
		        confirm = ntohl(confirm);
		        if(confirm == -1) continue;
		        
		        
    			bzero((char*)&buf,sizeof(buf));
		        //receive confirmation of deletion
		        if((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
				    perror("Server Received Error!\n");
				    exit(1);
			    }
			    
			    if (strcmp(buf, "Yes") == 0) {
			    	printf("delete confirmed\n");
			        //delete directory
			        char cwd[1024];
			        getcwd(cwd, sizeof(cwd));
			        strcat(cwd,"/");
			        strcat(cwd,dir_name);
			        printf("full dir: %s\n", cwd);
			        if((rmdir(cwd)) == -1) {
			            if(send(new_s, "Fail", strlen("Fail")+1, 0) == -1) {
			                perror("client send error!\n");
			                exit(1);
		                }
			        } else {
			            if(send(new_s, "Success", strlen("Success")+1, 0) == -1) {
			                perror("client send error!\n");
			                exit(1);
		                }
			        }
			    } else if (strcmp(buf, "No") == 0) {
			        // return to wait for operation
        			bzero((char*)&buf,sizeof(buf));
			        break;
			    }
				
			}
			
				
			
			else if(strcmp(buf, "CDIR\n") == 0) {
				int dir_size; char dir_name [1024]; int confirm; int r;
				// rmdir
				/* obtain length of directory name and directory name */
				if((len = recv(new_s, &dir_size, sizeof(dir_size), 0)) == -1) {
				    perror("Server Received Error!\n");
				    exit(1);
			    }
			    //dir_size = atoi(buf);
    			bzero((char*)&buf,sizeof(buf));
			    //receive dir name
			    if((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
				    perror("Server Received Error!\n");
				    exit(1);
			    }
				//dir_name = buf;
				strcpy(dir_name, buf);
				dir_name[strlen(dir_name)-1] = '\0';
				printf("directory name: %s\n",dir_name);
				
				//check if directory exists
				DIR* dir = opendir(dir_name);
				if (dir) {
					char cwd[1024];
			        getcwd(cwd, sizeof(cwd));
			        strcat(cwd,"/");
			        strcat(cwd,dir_name);
			        if((chdir(cwd)) == -1){
			        	confirm = -1;
			        } else confirm = 1;
				    closedir(dir);
				} else if (ENOENT == errno ){
				    confirm = -2;
				    printf("Doesn't exist\n");
				} else {
				    perror("Couldn't open directory\n");
				    exit(1);
				}
				
				//send confirmation of directory
				if(send(new_s, &confirm, sizeof(confirm), 0) == -1) {
			        perror("client send error!\n");
			        exit(1);
		        }
		        
				
			}
			else if(strcmp(buf, "QUIT\n") == 0) {
				// quit
				break;
			}
			else if(strcmp(buf, "\n") == 0 || strcmp(buf, "") == 0) {
			}
			else {
				printf("%s: invalid command\n", buf);
			}

			/*create time stamp*/
			//ltime=time(NULL);
			//Tm=localtime(&ltime);
			//printf("%d:%d:%d\n", Tm->tm_hour, Tm->tm_min, Tm->tm_sec);
			//gettimeofday(&tv, NULL);
			//sprintf(timeStamp, "%d:%d:%d.%d", Tm->tm_hour, Tm->tm_min, Tm->tm_sec, tv.tv_usec);
			bzero((char*)&buf,sizeof(buf));
		}
		
		if (close(new_s) != 0) {
			perror("Was not closed!\n");
		}
}
 

