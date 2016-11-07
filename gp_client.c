/////////////////////////////////////////////////////////////////////////////////////////////
//  PROGRAM      : gp_client.c
//  AUTHOR       : Stephen Beko
//	VERSION	     : 1
//  Date Created : 18/03/2016
//////////////////////////////////////////////////////////////////////////////////////
/*
 * Program is a TCP/IP Client to send messages to server.
 * Uses a thread to display incoming messages from Server to console.
 * 
 * TO BUILD
 *   gcc -g -Wall -pthread gp_client.c -lpthread -o gp_client
 * TO RUN  
 * ./gp_client 127.0.0.1 <portno>
 * 
 * Interesting : Uses while loop to write, thread to read.
 *             : Main passed  &sockfd in pthread_create as param.
 * <<< FIXME.. I noticed message len read back the same. Minor. 
 */
#include <stdio.h>
#include <stdlib.h>   // for exit
#include <strings.h>    //for bzero
#include <string.h>
#include <unistd.h>     // for close
#include <sys/socket.h>	//not really needed because added through incs below
#include <netinet/in.h>
#include <netdb.h> 
//#include <sys/ioctl.h>
#include <arpa/inet.h>  //for inet_addr - pulls in other includes
#include <errno.h>
#include <stdbool.h>    // for bool
#include <pthread.h>

void *client_read_message(void *param);

////////////////////////
///    FUNCTIONS    ////
////////////////////////
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// thread to read and display responses 
void *client_read_message(void *param)
{
    int *sockfd = (int*)param;
    int n;
    char display_buffer[256]; // display to screeen
    while (1)
    {
        // use this to read a response back on the socket
        // if server doesn't send one you will wait here
        // so advisable to add a timeout
        bzero(display_buffer,256);
        n = read(*sockfd,display_buffer,256);
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        else
        	 fprintf(stderr,"Server sent message,len=%d :%s\n", n, (char*)display_buffer);
        usleep(50000);  // (50ms)
    }
       return NULL;
}

/* MAIN */
int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    pthread_t pth;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    bool threadCreated = false;
    char buffer[256];    // text buffer - keyboard
    if (argc < 3) {
       fprintf(stderr,"gp_client: usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"gp_client: ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
     {
        error("ERROR connecting");
        exit(0);
     }
    else
        printf("Connected to Server\n");
    while (1)
    {
       	printf("Please enter the message (quit to exit): ");
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        if (strncmp(buffer,"quit", 4) == 0)
        {
        	break;
        }
    	n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
           error("ERROR writing to socket");
        // Create thread run in GPS_TEST_Read_Message
        if (!threadCreated)
        {
           if ( (pthread_create(&pth, NULL, client_read_message, &sockfd)) != 0)
           {
               fprintf(stderr,"Failed to create Thread to read Message\n");
           }
           else
           {
               threadCreated = true;
               fprintf(stderr,"gp_client: Read Thread created OK\n");
           }
        }
        sleep (2); // seconds
    }
    printf("EXIT program\n");
    close(sockfd);
    return 0;
}