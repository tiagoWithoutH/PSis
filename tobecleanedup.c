
#include <ncurses.h>
#include "server.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include<arpa/inet.h>

#include<ctype.h>

#define MAX_CLIENTS 10
#define WINDOW_SIZE 15
/*
void remove_position(struct sockaddr_in * array, int pos, int max)
{
    for(int j=pos; j<max; j++)
    {
        addr_book[j] = addr_book[j+1];
    }
}
*/
int main()
{	
        
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }
    
    // added for internet socket
    struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(SOCK_PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	int err = bind(sock_fd, (struct sockaddr *)&local_addr,
							sizeof(local_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}

	printf(" socket created and binded \n ");

    // null adress
    struct sockaddr_in null_add;
    null_add.sin_family = AF_INET;
    null_add.sin_addr.s_addr = inet_addr("0.0.0.0");
    null_add.sin_port = 0;

    struct sockaddr_in clientPlaying;
    struct sockaddr_in addr_book[MAX_CLIENTS];
    struct sockaddr_in requestBuffer[MAX_CLIENTS];
    socklen_t client_addr_size = sizeof(struct sockaddr_un);
    /*
	initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();			    
    */

    int n_bytes;
    message m;
    m.type = MOVE;
    message reply;
    reply.type = SEND;
    int nbClients = 0;
    int nbRequests = 0;

    

    while (1)
    {
        struct sockaddr_in client_addr;
        n_bytes = recvfrom(sock_fd, &m, sizeof(message), 0, 
                        (const struct sockaddr *) &client_addr, &client_addr_size);
        if (n_bytes!= sizeof(message)){
            continue;
        }	

        printf("From add %s, port %hu:\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
        // some_addr = inet_ntoa(antelope.sin_addr);

        //int isListed = 0;
        int pos = -1;
        for(int i=0; i<nbClients; i++)
        {
            if(inet_ntoa(client_addr.sin_addr) == inet_ntoa(addr_book[i].sin_addr) && client_addr.sin_port == addr_book[i].sin_port)
            {
                //isListed = 1;
                pos = i;
                break;
            }
        }

        if(pos < 0 && m.type != CONNECT)
        {
            printf("Client must be conected!!!\n");
            continue;
        }

        switch(m.type)
        {
            case CONNECT:
                if(pos >= 0)
                {
                    printf("This client is already conncected\n");
                    break;
                }
                addr_book[nbClients] = client_addr; 
                nbClients++;
                printf("Address %s, port %hu has joined\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port); //, client_addr.sin_port);
                printf("CONNECT received\n");
                requestBuffer[nbRequests] = client_addr;
                nbRequests++;
                break;
            case RELEASE:
                if(inet_ntoa(client_addr.sin_addr) == inet_ntoa(clientPlaying.sin_addr) && client_addr.sin_port == clientPlaying.sin_port)
                {
                    clientPlaying.sin_addr = null_add.sin_addr;
                    clientPlaying.sin_port = null_add.sin_port;
                }  
                printf("RELEASE received\n");
                break;
            case MOVE:
                if(nbClients > 1)
                {
                    for(int i=0; i<nbClients; i++)
                    {
                        if(inet_ntoa(client_addr.sin_addr) == inet_ntoa(addr_book[i].sin_addr) && client_addr.sin_port == addr_book[i].sin_port)
                        {
                            continue;
                        }
                        sendto(sock_fd, &m, sizeof(message), 0, 
                            (const struct sockaddr *) &addr_book[i], client_addr_size);
                    }
                }
                printf("MOVE received\n");
                break;
            case DISCONNECT:
                // remove_position(addr_book, pos, nbClients);
                for(int i=0; i<nbRequests; i++)
                {
                    if(inet_ntoa(addr_book[pos].sin_addr) == inet_ntoa(requestBuffer[i].sin_addr) && addr_book[pos].sin_port == requestBuffer[i].sin_port)
                    {
                        for(int j=i; j<nbRequests; j++)
                        {
                            addr_book[j] = addr_book[j+1];
                        }
                    }
                }
                for(int j=pos; j<nbClients; j++)
                {
                    addr_book[j] = addr_book[j+1];
                }
                nbClients--;
                clientPlaying.sin_addr = null_add.sin_addr;
                clientPlaying.sin_port = null_add.sin_port;
                printf("DISCONNECT received\n");
                break;        
            default:
                printf("message type not listed\n");
                break;
        }

        if(inet_ntoa(clientPlaying.sin_addr) == inet_ntoa(null_add.sin_addr) && clientPlaying.sin_port == null_add.sin_port)
        {
            if(nbClients > 0)
            {
                if(nbRequests > 0)
                {
                    clientPlaying = requestBuffer[0];
                    nbRequests--;
                    // shift the elements in the array to the left
                    for(int j=0; j<nbRequests; j++)
                    {
                        requestBuffer[j] = requestBuffer[j+1];
                    }
                }
                else
                {
                    if(pos+1 < nbClients)
                    {
                        clientPlaying = addr_book[pos+1];
                    }
                    else
                    {
                        clientPlaying = addr_book[0];
                    }
                    /*
                    if(nbClients > 1)
                        clientPlaying = addr_book[rand() % nbClients];
                    else
                        clientPlaying = addr_book[0];
                        */
                }   
                reply.type = SEND;
                sendto(sock_fd, &reply, sizeof(reply), 0, 
                    (const struct sockaddr *) &clientPlaying, client_addr_size);
            }
        }
        
        printf("ADDRESS BOOK:\n");
        for(int i=0; i<nbClients; i++)
        {
            printf("%i - Address %s, port %hu\n", i, inet_ntoa(addr_book[i].sin_addr), addr_book[i].sin_port);
        }

        printf("REQUEST BUFFER:\n");
        for(int i=0; i<nbRequests; i++)
        {
            printf("%i - Address %s, port %hu\n", i, inet_ntoa(requestBuffer[i].sin_addr), requestBuffer[i].sin_port);
        }

        printf("Client playing at address %s, port %hu\n", inet_ntoa(clientPlaying.sin_addr), clientPlaying.sin_port);
        printf("\n");
    }
  

	return 0;
}