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

#include "encode.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "decode.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_writer.h"

#define MAX_FILE_PATH_SIZE 512
#define MAX_BUFFER_SIZE 20000

const char *blurHashForFile(int xComponents, int yComponents,const char *filename);

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
			char file_path[MAX_FILE_PATH_SIZE] = "/usr/recv_image.png";
			FILE *fp = fopen(file_path, "w");
			int len = 0, passwhile = 1;
			while ((len = recv(connfd, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
				passwhile = 0;
				if(fwrite(buffer, sizeof(char), len, fp) < len) {
					printf("Image File: %s Write Failed\n", file_path);
					break; 
				}
				else
				memset(buffer, 0, MAX_BUFFER_SIZE);
			}
			fclose(fp);
			if(!passwhile){
				printf("Received Image File: %s Successful!\n", file_path);
				const char *hash = blurHashForFile(4, 3, file_path);
				if(!hash) {
					fprintf(stderr, "Failed to load image file \"%s\".\n", file_path);
					return 1;
				}
			}
		}
	        close(connfd);
	}

	return 0;
}

const char *blurHashForFile(int xComponents, int yComponents,const char *filename) {
	int width, height, channels;
	unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);
	if(!data) return NULL;

	const char *hash = blurHashForPixels(xComponents, yComponents, width, height, data, width * 3);
	printf("blurhash: %s\n", hash);
	stbi_image_free(data);

	const int nChannels = 4, punch = 1;
	char output_file[MAX_FILE_PATH_SIZE] = "/usr/blur_image.png";
	uint8_t *bytes = decode(hash, width, height, punch, nChannels);

	if (!bytes) {
		fprintf(stderr, "%s is not a valid blurhash, decoding failed.\n", hash);
		return 1;
	}
	if (stbi_write_png(output_file, width, height, nChannels, bytes, nChannels * width) == 0) {
		fprintf(stderr, "Failed to write PNG file %s\n", output_file);
		return 1;
	}
	freePixelArray(bytes);
	fprintf(stdout, "Decoded blurhash successfully, wrote PNG file %s\n", output_file);

	return hash;
}
