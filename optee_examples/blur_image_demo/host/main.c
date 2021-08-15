#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>

#define MAX_FILE_PATH_SIZE 512
#define MAX_BUFFER_SIZE 1024

int main(){

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("Can't allocate sockfd");
		exit(1);
	}

	if(fcntl(sockfd, F_SETOWN, getpid()) == -1){
		printf("Set socket_server pid to sockfd failed");
	}

	struct sockaddr_un server;
	memset(&server, 0, sizeof(server));
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, "./socket");

	bind(sockfd, (const struct sockaddr *) &server, sizeof(server));
	listen(sockfd, 5);

	char buffer[MAX_BUFFER_SIZE];
	memset(buffer, 0, MAX_BUFFER_SIZE);
	while(1) {  // receive image
		int connfd;
		if(connfd = accept(sockfd, 0, 0) < 0);
		else {
			char file_path[MAX_FILE_PATH_SIZE] = "//usr//recv_image.png";
			FILE *fp = fopen(file_path, "w");
			int len = 0, success = 1;
			while ((len = recv(connfd, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
				if(fwrite(buffer, sizeof(char), len, fp) < len) {
					printf("Image File: %s Write Failed\n", file_path);
					success = 0;
					break; 
				}
				memset(buffer, 0, MAX_BUFFER_SIZE);
			}
			fclose(fp);
			/*if(success)
				printf("Received Image File: %s Successful!\n", file_path);*/
		}
	        close(connfd);
	}

	return 0;
}
