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

#include <time.h>

#define TEN_SEC 10
		    


int main()
{
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

    initscr();		    	
    cbreak();				
    keypad(stdscr, TRUE);   
    noecho();	

    int connected = 0; 
    int playState = 0;
    int x = 0;
    int y = 0;
    message m = {CONNECT, x, y};
    message fromServer;
    char ch = ' ';
    int key = 0;


    printf("Press the number of the message to send:\n");
    printf("0 - CONNECT \n1 - RELEASE \n2 - SEND\n3 - MOVE\n4 - DISCONNECT\n");

    while (1)
    {
        m.type = CONNECT;
        printf("Press the C key to connect to the server\n");
        while(ch != 'c'){scanf("%c", &ch);}
        sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));
        ch = ' ';
        connected = 1;
        while(connected)
        {
            recv(sock_fd, &fromServer, sizeof(message), 0);
            if(fromServer.type == MOVE)
            {
                x = fromServer.pos_x;
                y = fromServer.pos_y;
                printf("MOVE received: (x = %d, y = %d)\n", x ,y);
            }
            else
            {
                if(fromServer.type == SEND)
                {
                    playState = 1;
                    printf("Play on!!!\n");
                    time_t start, current;
                    start = time(NULL);
                    do
                    {
                        current = time(NULL);
                        key = getch();
                        switch (key)
                        {
                        case 'r':
                            playState = 0;
                            connected = 0;
                            m.type = DISCONNECT;
                            break;
                        case KEY_LEFT:
                            m.pos_x = --x;
                            m.type = MOVE;
                            break;
                        case KEY_RIGHT:
                            m.pos_x = ++x;
                            m.type = MOVE;
                            break;
                        case KEY_DOWN:
                            m.pos_y = --y;
                            m.type = MOVE;
                            break;
                        case KEY_UP:
                            m.pos_y = ++y;
                            m.type = MOVE;
                            break;
                        default:
                            break;
                        }
                        sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));
                    }while(difftime(current, start) < TEN_SEC);
                    m.type = RELEASE;
                    sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));
                }
                else
                {
                    printf("INVALID MESSAGE\n");
                }
                
            }
            
        }
        m.type = DISCONNECT;
        sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));

        /*
        int i;
        scanf("%d", &i);
        message m = {i, 0, 0};        
        sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));
         */
    }
    endwin();
    
	return 0;
}