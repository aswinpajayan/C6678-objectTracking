/*
 * =====================================================================================
 *
 *       Filename:  UDP_client.c
 *
 *    Description: code take from cs.cmu.edu
 *        Version:  1.0
 *        Created:  Thursday 21 March 2019 08:03:18  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         modified by:  Aswin P Ajayan (), aswinp2008@gmail.com
 *   Organization:  iitb
 *
 * =====================================================================================
 */


/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#define BUFSIZE 1024
#define SERVER_IP "10.107.2.126"
#define PORT 7 //set according to server settings
#define IN_SIZE 14
#define NUM_OF_ROWS 75
pthread_t recvPointTID;


unsigned char outBuf[BUFSIZE]; //buffer to transmit the images
unsigned char inBuf[IN_SIZE]; //recieve back the coordinates and other info

void* getCenterThread(void *arg);

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
int startProcessing(int curRowNum, char *cmd) {
    int sockfd, portno, n;
    static int rowNum = 0;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;

    hostname = SERVER_IP;
    portno = PORT;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */

    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, outBuf, sizeof(outBuf), 0, (const struct sockaddr *) &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    //printf("\nRowNum to be send : %d marker: %d",curRowNum,rowNum);
    rowNum++;
recvfrom(sockfd, inBuf, BUFSIZE, 0, (struct sockaddr *) &serveraddr, (socklen_t *)&serverlen);
if (n < 0) 
  error("ERROR in recvfrom");
//printf("Echo from server: %s %d\n",inBuf,rowNum);
    if(curRowNum >= NUM_OF_ROWS -1){
	/* print the server's reply */
//	pthread_create(&recvPointTID,NULL,&getCenterThread,&serveraddr);
        rowNum = 0;
    }
        close(sockfd);
    return 0;
}

void* getCenterThread(void *arg){
	int serverlen;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	int sockfd,n;
	char *hostname;

	hostname = SERVER_IP;
	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	printf("Waitin.....lastByte: %d \n",outBuf[512]);
	if (server == NULL) {
	    fprintf(stderr,"ERROR, no such host\n");
	    exit(0);
	}
        /* build the server's Internet address */
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
    	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
        serveraddr.sin_port = htons(PORT);
      

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
	    error("ERROR opening socket");
	serverlen = sizeof(serveraddr);
	 n = recvfrom(sockfd, inBuf, sizeof(inBuf), 0, (struct sockaddr *) &arg, (socklen_t *)&serverlen);
	if (n <= 0) 
	  error("ERROR in recvfrom");
	printf("Echo from server: %s \n",inBuf);
	close(sockfd);
	pthread_exit(NULL);
}

//int main(){
//	memcpy(outBuf,"Sending images",14);
//	startProcessing(1,"Testing out the code");
//	return 0;
//}
