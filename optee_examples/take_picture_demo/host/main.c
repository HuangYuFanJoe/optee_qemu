#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/socket.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_FILE_PATH_SIZE 512

int main(int argc , char *argv[])
{
	char file_path[MAX_FILE_PATH_SIZE];
	memset(file_path, 0, MAX_FILE_PATH_SIZE);
	if(argc == 2)
		strcpy(file_path, argv[1]);
	else if(argc == 1)
		strcpy(file_path, "//usr//image.png");
	else{
		printf("Usage: %s file_path", argv[0]);
		return 0;
	}
	printf("file path: %s\n", file_path);
	
	//socket的建立
	int sockfd = 0;
	sockfd = socket(AF_UNIX , SOCK_STREAM , 0);

	if (sockfd == -1){
		printf("Fail to create a socket.\n");
	}

	int optval = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(optval)) == -1){
		printf("SET socket opt failed\n");
	}

	if(fcntl(sockfd, F_SETOWN, getpid()) == -1){
		printf("Set socket_client pid to sockfd failed\n");
	}

	//socket的連線

	struct sockaddr_un info;
	bzero(&info,sizeof(info));
	info.sun_family = AF_UNIX;
	strcpy(info.sun_path, "./socket");

	int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
	if(err==-1){
		printf("Connection error\n");
	}

	FILE *fp = fopen(file_path, "r");
	if(fp == NULL)
		printf("Image file %s not found", file_path);

	//Send a picture to server
	char buffer[MAX_BUFFER_SIZE];
	memset(buffer, 0, MAX_BUFFER_SIZE);
	int len = 0, success = 1;
	while ((len = fread(buffer, sizeof(char), MAX_BUFFER_SIZE, fp)) > 0) {
		if (send(sockfd, buffer, len, 0) < 0) { 
			printf("Send Image File: %s Failed\n", file_path);
			success = 0;
			break;
		}
		memset(buffer, 0, MAX_BUFFER_SIZE); 
	} 
 
	fclose(fp);
	if(success)
		printf("Image File: %s send successful\n", file_path); 
	
	//char receiveMessage[100] = {};
	//recv(sockfd,receiveMessage,sizeof(receiveMessage),0);

	printf("close Socket\n");
	close(sockfd);
  
	return 0;
}
