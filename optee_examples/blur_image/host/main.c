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

	char data[50] = {};
	while(1){  // receive data
		int connfd;
		if(connfd = accept(sockfd, 0, 0) < 0);
		else{
			recv(connfd, data, 50, 0);
			printf("%s \n", data);
		}
	        close(connfd);
	}

	return 0;
}
