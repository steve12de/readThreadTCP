//////////////////////////////////////////////////////////////////////////////////////
//  PROGRAM      : gp_server.c
//  AUTHOR       : Stephen Beko
//	VERSION	     : 1
//  Date Created : 18/03/2016
//////////////////////////////////////////////////////////////////////////////////////
/*
 * Program is a TCP/IP Server. Accepts a single Client connection and runs 
 * socket read, in a thread. Displays incoming message from client to console.
 * Uses loopback address 127.0.0.1.
 * Requires port on command line. 
 * 
 * TO BUILD
 *   gcc -g -Wall -pthread gp_server.c -lpthread -o gp_server
 * TO RUN  
 * ./gp_server <portno>
 * 
 * Interesting : Uses pthread_join to avoid main exit.
 * <<< FIXME.. I noticed external laptop didn't connect, only local client !
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>    //for bzero
#include <string.h>
#include <unistd.h>     // for close
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h> 
#include <sys/socket.h>	//not really needed because added through incs below
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>  //for inet_addr - pulls in other includes
#include <errno.h>
#include <stdbool.h>    // for bool
#include <pthread.h>

/*
*******************************************************************************
Private declarations
*******************************************************************************
*/
// Threads
static void *socket_read_message(void *param);

/*
  Globals
*/
int sockfd = 0;
int listenfd = 0;

////////////////////////////////////
//////////// FUNCTIONS  ////////////
////////////////////////////////////
/*
 *   error()
 */
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*
 *   socket_block()
 */
static inline void socket_block(int sock, int block)
{
    if (sock >= 0)
    {
        fcntl(sock, F_SETFL, (block) ? 0 : O_NONBLOCK);
    }
}

/*
 *   *socket_read_message()
 *   read message handler as a thread
 */
static void *socket_read_message(void *param)
{
   char buffer[256];
   char sendBuf[20];
   int n = 0;
   bool clientConnect = false;
   
   sockfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
   ioctl(sockfd, FIONBIO, 0);
   // Single Connection Only
   bzero(buffer, 256);
   bzero(sendBuf, 256);
   if((n = (read(sockfd, &buffer,sizeof(buffer)))) > 0)
   {
       fprintf(stderr,"Connect Succeeded on fd%d: Client sent message,len=%d :%s\n",sockfd, n, (char*)buffer);
       clientConnect = true;
       strcpy(sendBuf,"CONNECT OK\n");
       send(sockfd, sendBuf, sizeof(sendBuf), 0);
   }
   else
   {
       fprintf(stderr,"Connect Failed\n"); 
   }
    // Same Connection - Read any further message 
    while(clientConnect)
    {
        // we can now read from the socket
        bzero(buffer, 256);
        bzero(sendBuf, 256);
        socket_block(sockfd, true); //TRUE = BLK / FALSE = NON BLK
        if ((n = (int)recv(sockfd, buffer, sizeof(buffer), 0)) < 0)
        {
            error("ERROR reading from socket");
            break;
        }
        if (n == 0)
        {
        	printf("tm_skt: Lost Existing Connection on fd%d\n", sockfd);
            break;
        }
        else
            fprintf(stderr,"Client sent message,len=%d :%s\n", n, (char*)buffer);
            // send response
            strcpy(sendBuf,"ACK\n");
            send(sockfd, sendBuf, sizeof(sendBuf), 0);
        sleep(1);  //seconds
    }
    fprintf(stderr,"exiting\n");
    close (listenfd);
    close (sockfd);
    exit(0);
    return NULL;
}

/*
 * main()
 */
int main(int argc, char *argv[])
{
    pthread_t pth1;
    struct sockaddr_in serv_addr;
    int portNum;
    char ip_address[] = "127.0.0.1";

    if (argc < 2) 
    {
   	    error("tm_skt: ERROR, NO PORT provided\n");
        exit(1);
    }    
    // create Thread for TCP/IP Server Socket for interface to client (GUI)
    fprintf(stderr,"Create TCP/IP Server\n");
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr,"\n Error : Could not create socket \n");
        exit(0);
    }
    // Set up IP socket
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    portNum = atoi(argv[1]);
    serv_addr.sin_port = htons(portNum);
    serv_addr.sin_addr.s_addr = inet_addr(ip_address);
    memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    // backlog - connections, 1 in this case
    listen(listenfd, 10);

    // Create thread and Read_Message handler
    fprintf(stderr,"TCP/IP Server created OK on fd%d, run in thread\n", listenfd);
    if ( (pthread_create(&pth1, NULL, socket_read_message, NULL)) != 0)
    {
        fprintf(stderr,"Failed to create Thread\n");
        exit(0);
    }
    // NORMALLY MAIN WOULD NOT EXIT, but here we call pthread_join 
    
    // pthread_join syncronises calling thread with called thread. 
    // Calling thread suspended until called thread exits.
    pthread_join(pth1,NULL);
    return 0;
}
