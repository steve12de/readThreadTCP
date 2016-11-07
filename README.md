# readThreadTCP
TCP/IP Serverto demonstrate use of socket read in a pthread.
Accepts a single Client connection and runs socket read, in a thread. 
Displays incoming message from client to console.
TO BUILD
  gcc -g -Wall -pthread gp_server.c -lpthread -o gp_server
TO RUN  
 ./gp_server <portno>
Interesting : Uses pthread_join to avoid main exit.
FIXME. I noticed external laptop didn't connect to this, only local client !

The TCP/IP Client allows user to send a messages to a server.
It also uses a thread to display incoming responses from Server to console.
TO BUILD
 gcc -g -Wall -pthread gp_client.c -lpthread -o gp_client
TO RUN  
./gp_client 127.0.0.1 <portno>
Interesting : Uses while loop to write, thread to read.
            : Main passed  &sockfd in pthread_create as param.
FIXME.. I noticed message len read back the same. Minor.
