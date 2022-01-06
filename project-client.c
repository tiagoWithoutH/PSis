#include <ncurses.h>
#include "pong.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{

    //TODO_4
    // create and open the FIFO for writing
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }  
    printf(" socket created \n Ready to send\n");


    // added to get the server address
    char linha[100];  

    printf("What is the network address of the recipient? ");
	fgets(linha, 100, stdin);
	linha[strlen(linha)-1] = '\0';

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SOCK_PORT);
	if( inet_pton(AF_INET, linha, &server_addr.sin_addr) < 1){
		printf("no valid address: \n");
		exit(-1);
	}
    /* 
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(SOCK_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    int err = bind(sock_fd, (struct sockaddr *)&local_addr, 
                                sizeof(local_addr));
    if(err == -1)
    {
        perror("bind");
        exit(-1);
    }
    printf(" scoket created and binded");
    */

    while (1)
    {
        int i, x, y;
        message * m;
        message fromServer;
        printf("Pressed the number of the message to send:\n");
        printf("0 - CONNECT \n1 - RELEASE \n2 - SEND\n3 - MOVE\n 4 - DISCONNECT\n");
        scanf("%d", &i);
        if(i == 3)
        {
            printw("Shift x");
            scanf("%d", &x);
            printw("Shift y");
            scanf("%d", &y);
            m->type = MOVE;
            m->pos_x = x;
            m->pos_y = y;
        }
        m->type = i;
        sendto(sock_fd, m, sizeof(message), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));
        recv(sock_fd, &fromServer, sizeof(message), 0);
        if(fromServer.type == 3)
        {
            printf("x = %d, y = %d\n", fromServer.pos_x, fromServer.pos_y);
        }

    }
    
    
	return 0;
}