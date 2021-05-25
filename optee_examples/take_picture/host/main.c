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


int main(int argc , char *argv[])
{

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
    
    int len;
    len = sizeof(struct ucred);
    struct ucred uc;
    if(getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &uc, &len) == -1){
        printf("get peer cred failed\n");
    }
    else{
    	printf("peer pid: %d\n", uc.pid);
    }
    
    //Send a message to server
    char message[20] = "Requester: fanfan";
    char receiveMessage[100] = {};


    send(sockfd,message,sizeof(message), uc.pid);
    //recv(sockfd,receiveMessage,sizeof(receiveMessage),0);

    //printf("%s",receiveMessage);
    printf("close Socket\n");
    close(sockfd);
    
    return 0;
}
