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
#include <sys/time.h>

int main(int argc , char *argv[])
{
    struct timeval before, after;
    gettimeofday(&before, NULL);
    
    int k;
    if(argc == 2)
    	k = atoi(argv[1]);
    else if(argc == 1)
    	k = 1;
    else{
    	printf("Usage: %s count(optional)\n", argv[0]);
    	return 0;
    }
    
    for(int i = 0; i < k; i++){
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
	    
	    //Send a message to server
	    char message[1024] = "Requester: fanfan";
	    char receiveMessage[100] = {};
	    
	    send(sockfd,message,sizeof(message), 0);
	    //recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
		
	    printf("close Socket\n");
	    close(sockfd);
    }
    
    gettimeofday(&after, NULL);
    printf("Before: %ld.%ld\n", before.tv_sec, before.tv_usec);
    printf("After: %ld.%ld\n", after.tv_sec, after.tv_usec);
    
    return 0;
}
