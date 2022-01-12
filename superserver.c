
#include <ncurses.h>
#include "super.h"
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

ball_position_t ball;
paddle_position_t paddles[10];
int scores[10];

void new_paddle (paddle_position_t * paddle, int legth){
    paddle->x = (rand() % (WINDOW_SIZE-4)) - 2;
    paddle->y = (rand() % (WINDOW_SIZE-4)) - 2;
    paddle->length = legth;
}

void moove_paddle (paddle_position_t * paddle, int direction, int pos, int nbClients){
    
    for(int i=0; i<nbClients; i++)
    {
        if(i == pos)
            continue;

        if (direction == KEY_UP){
            if (paddle[pos].y - 1 == paddle[i].y){
                return;
            }
        }
        if (direction == KEY_DOWN){
            if (paddle[pos].y + 1 == paddle[i].y){
                return;
            }
        }
        

        if (direction == KEY_LEFT){
            if (paddle[pos].x - paddle[pos].length == paddle[i].x + paddle[i].length){
                return;
            }
        }
        if (direction == KEY_RIGHT)
            if (paddle[pos].x - paddle[pos].length == paddle[i].x + paddle[i].length){
                return;
        }
        
    }


    if (direction == KEY_UP){
        if (paddle[pos].y  != 1){
            paddle[pos].y --;
        }
    }
    if (direction == KEY_DOWN){
        if (paddle[pos].y  != WINDOW_SIZE-2){
            paddle[pos].y ++;
        }
    }
    

    if (direction == KEY_LEFT){
        if (paddle[pos].x - paddle[pos].length != 1){
            paddle[pos].x --;
        }
    }
    if (direction == KEY_RIGHT)
        if (paddle[pos].x + paddle[pos].length != WINDOW_SIZE-2){
            paddle[pos].x ++;
    }


}

int hit(paddle_position_t paddle, ball_position_t ball)
{
    int next_y = ball.y + ball.up_hor_down;
    int next_x = ball.x + ball.left_ver_right;

    if(paddle.y != next_y && paddle.y != ball.y)
        return 0;

    int start_x = paddle.x - paddle.length;
    int end_x = paddle.x + paddle.length;

    if(next_x == start_x && ball.left_ver_right == 1)
        return 1;

    if(next_x == end_x && ball.left_ver_right == -1)
        return 1;

    for (int x = start_x; x <= end_x; x++)
    {
        if(x == next_x || x == ball.x)
            return 2;
    }
    return 0;
}

void moove_ball(ball_position_t * ball, int hit){
    
    int next_x = ball->x + ball->left_ver_right;
    if( next_x == 0 || next_x == WINDOW_SIZE-1 || hit == 1){
        ball->up_hor_down = rand() % 3 -1 ;
        ball->left_ver_right *= -1;
        //mvwprintw(message_win, 2,1,"left right win");
        //wrefresh(message_win);
     }else{
        ball->x = next_x;
    }

    
    int next_y = ball->y + ball->up_hor_down;
    if( next_y == 0 || next_y == WINDOW_SIZE-1 || hit == 2){
        ball->up_hor_down *= -1;
        ball->left_ver_right = rand() % 3 -1;
        //mvwprintw(message_win, 2,1,"bottom top win");
        //wrefresh(message_win);
    }else{
        ball -> y = next_y;
    }
}

void place_ball_random(ball_position_t * ball){
    ball->x = rand() % WINDOW_SIZE ;
    ball->y = rand() % WINDOW_SIZE ;
    ball->c = 'o';
    ball->up_hor_down = rand() % 3 -1; //  -1 up, 1 - down
    ball->left_ver_right = rand() % 3 -1 ; // 0 vertical, -1 left, 1 right
}

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

    struct sockaddr_in addr_book[MAX_CLIENTS];
    int moves[MAX_CLIENTS];
    //for(int i=0; i<MAX_CLIENTS; i++)
    //    moves[i] = 0;
    int nbMoves = 0;
    socklen_t client_addr_size = sizeof(struct sockaddr_un);
    int n_bytes;
    message m;
    board_update board;
    m.type = PADDLE_MOVE;
    int nbClients = 0;
    int h = 0;

    place_ball_random(&ball);

    
    while (1)
    {
        struct sockaddr_in client_addr;
        n_bytes = recvfrom(sock_fd, &m, sizeof(message), 0, 
                        (const struct sockaddr *) &client_addr, &client_addr_size);
        if (n_bytes!= sizeof(message)){
            continue;
        }	

        printf("From add %s, port %hu:\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

        int pos = -1;
        for(int i=0; i<nbClients; i++)
        {
            if(inet_ntoa(client_addr.sin_addr) == inet_ntoa(addr_book[i].sin_addr) && client_addr.sin_port == addr_book[i].sin_port)
            {
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
                if(nbClients > MAX_CLIENTS)
                {
                    printf("SERVER IS FULL!!!\n Wait until a client leaves\n");
                    continue;
                }
                addr_book[nbClients] = client_addr; 
                paddle_position_t p;
                new_paddle(&p, PADDLE_SIZE);
                paddles[nbClients] = p;
                scores[nbClients] = 0;
                nbClients++;
                printf("Address %s, port %hu has joined\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port); 
                printf("CONNECT received\n");
                sendto(sock_fd, &board, sizeof(board_update), 0, 
                        (const struct sockaddr *) &client_addr, client_addr_size);
                printf("Board Update sent to address %s, port %hu\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
                break;
            case PADDLE_MOVE:
                nbMoves++;
                // reading from the client message
                moove_paddle (paddles, m.key, pos, nbClients);
                h = hit(paddles[pos], ball);
                if(h > 0){
                    scores[pos]++;}
                if(nbMoves >= nbClients)
                {
                    moove_ball(&ball, h);
                    nbMoves = 0;
                }
                // setting up Board Update
                board.ball = ball;
                board.own_paddle = pos;
                board.paddles[pos] = paddles[pos];
                board.scores[pos] = scores[pos]; 
                board.nbClients = nbClients;
                printf("PADDLE MOVE received\n");
                sendto(sock_fd, &board, sizeof(board_update), 0, 
                        (const struct sockaddr *) &client_addr, client_addr_size);
                printf("Board Update sent to address %s, port %hu\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);     
                break;
            case DISCONNECT:
                for(int j=pos; j<nbClients; j++)
                {
                    addr_book[j] = addr_book[j+1];
                    paddles[j] = paddles[j+1];
                    scores[j] = scores[j+1];
                }
                nbClients--;
                printf("DISCONNECT received\n");
                printf("Address %s, port %hu was removed\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
                break;        
            default:
                printf("message type not listed\n");
                break;
        }

        
        printf("ADDRESS BOOK:\n");
        for(int i=0; i<nbClients; i++)
        {
            printf("%i - Address %s, port %hu\n", i, inet_ntoa(addr_book[i].sin_addr), addr_book[i].sin_port);
        }
        printf("\n");
    }
  

	return 0;
}