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

WINDOW * message_win;

// my func
int hit(paddle_position_t paddle, ball_position_t ball)
{
    if(paddle.y != ball.y)
        return 0;

    int start_x = paddle.x - paddle.length;
    int end_x = paddle.x + paddle.length;
    for (int x = start_x; x <= end_x; x++)
    {
        if(x == ball.x)
            return 1;
    }
    return 0;
}

// sets the position of the paddle to the bottom middle of the window	    
void new_paddle (paddle_position_t * paddle, int length){
    paddle->x = WINDOW_SIZE/2;
    paddle->y = WINDOW_SIZE-2;
    paddle->length = length;
}

// draws the paddle inputed, 
// delete = 0 - erase
// delete = 1 - draw
void draw_paddle(WINDOW *win, paddle_position_t * paddle, int delete){
    int ch;
    if(delete){
        ch = '_';
    }else{
        ch = ' ';
    }
    int start_x = paddle->x - paddle->length;
    int end_x = paddle->x + paddle->length;
    for (int x = start_x; x <= end_x; x++){
        wmove(win, paddle->y, x); // moves the cursor to the inputed position
        waddch(win,ch);
    }
    wrefresh(win);
}

/* sets a new position to the paddle, 
preventing it from going out of bounds */
void moove_paddle (paddle_position_t * paddle, int direction){
    if (direction == KEY_UP){
        if (paddle->y  != 1){
            paddle->y --;
        }
    }
    if (direction == KEY_DOWN){
        if (paddle->y  != WINDOW_SIZE-2){
            paddle->y ++;
        }
    }
    

    if (direction == KEY_LEFT){
        if (paddle->x - paddle->length != 1){
            paddle->x --;
        }
    }
    if (direction == KEY_RIGHT)
        if (paddle->x + paddle->length != WINDOW_SIZE-2){
            paddle->x ++;
    }
}


void place_ball_random(ball_position_t * ball){
    ball->x = rand() % WINDOW_SIZE ;
    ball->y = rand() % WINDOW_SIZE ;
    ball->c = 'o';
    ball->up_hor_down = rand() % 3 -1; //  -1 up, 1 - down
    ball->left_ver_right = rand() % 3 -1 ; // 0 vertical, -1 left, 1 right
}

/* sets a new position to the ball, 
according to left_ver_right and up_hor_down
preventing it from going out of bounds */
void moove_ball(ball_position_t * ball){
    
    int next_x = ball->x + ball->left_ver_right;
    if( next_x == 0 || next_x == WINDOW_SIZE-1){
        ball->up_hor_down = rand() % 3 -1 ;
        ball->left_ver_right *= -1;
        mvwprintw(message_win, 2,1,"left right win");
        wrefresh(message_win);
     }else{
        ball->x = next_x;
    }

    
    int next_y = ball->y + ball->up_hor_down;
    if( next_y == 0 || next_y == WINDOW_SIZE-1){
        ball->up_hor_down *= -1;
        ball->left_ver_right = rand() % 3 -1;
        mvwprintw(message_win, 2,1,"bottom top win");
        wrefresh(message_win);
    }else{
        ball -> y = next_y;
    }
}

// draws the paddle inputed, 
// draw = 0 - erase
// draw = 1 - draw predefined character
void draw_ball(WINDOW *win, ball_position_t * ball, int draw){
    int ch;
    if(draw){
        ch = ball->c;
    }else{
        ch = ' ';
    }
    wmove(win, ball->y, ball->x);
    waddch(win,ch);
    wrefresh(win);
}

    
    paddle_position_t paddle;
    ball_position_t ball;

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

    int connected = 0; 
    int playState = 0;
    int shift_x;
    int shift_y;
    paddle_position_t paddle;
    ball_position_t ball;
    message m = {CONNECT, ball, paddle};
    message fromServer;
    char ch = ' ';
    int key = 0;

    while (1)
    {
        m.type = CONNECT;
        printf("Press the C key to connect to the server\n");
        while(ch != 'c'){scanf("%c", &ch);}
        sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));
        ch = ' ';
        connected = 1;

        initscr();		    	
        cbreak();				
        keypad(stdscr, TRUE);   
        noecho();	

        /* creates a window and draws a border */
        WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
        box(my_win, 0 , 0);	
        wrefresh(my_win);
        keypad(my_win, true);
        /* creates a window and draws a border */
        message_win = newwin(5, WINDOW_SIZE+10, WINDOW_SIZE, 0);
        box(message_win, 0 , 0);	
        wrefresh(message_win);


        new_paddle(&paddle, PADDLE_SIZE);
        draw_paddle(my_win, &paddle, true);

        draw_ball(my_win, &ball, true);

        while(connected)
        {
            recv(sock_fd, &fromServer, sizeof(message), 0);
            if(fromServer.type == MOVE)
            {
                paddle = fromServer.paddle;
                ball = fromServer.ball;
                //printf("MOVE received: (x = %d, y = %d)\n", x ,y);

                draw_paddle(my_win, &paddle, false);
                moove_paddle (&paddle, key);
                draw_paddle(my_win, &paddle, true);

                draw_ball(my_win, &ball, false);
                moove_ball(&ball);
                draw_ball(my_win, &ball, true);
            }
            else
            {
                if(fromServer.type == SEND)
                {
                    playState = 1;
                    printf("Play on!!!\n");
                    time_t start, current;
                    start = time(NULL);

                    current = time(NULL);
                    key = getch();

                    do
                    {
                        shift_x = 0;
                        shift_y = 0;
                        switch (key)
                        {
                        case 'r':
                            playState = 0;
                            m.type = RELEASE;
                            break;
                        case 'q':
                            playState = 0;
                            connected = 0;
                            m.type = DISCONNECT;
                            break;
                        case KEY_LEFT:
                            shift_x = -1;
                            m.type = MOVE;
                            break;
                        case KEY_RIGHT:
                            shift_x = 1;
                            m.type = MOVE;
                            break;
                        case KEY_DOWN:
                            shift_y = -1;
                            m.type = MOVE;
                            break;
                        case KEY_UP:
                            shift_y = 1;
                            m.type = MOVE;
                            break;
                        default:
                            break;
                        }
                        paddle.x += shift_x;
                        paddle.y += shift_y;

                        if(hit)
                        {
                            ball.up_hor_down = shift_x;
                            ball.left_ver_right = shift_y;  
                            draw_ball(my_win, &ball, false);
                            moove_ball(&ball);
                            draw_ball(my_win, &ball, true); 
                        }
                        m.ball = ball;
                        m.paddle = paddle;
                        sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));

                        draw_paddle(my_win, &paddle, false);
                        moove_paddle (&paddle, key);
                        draw_paddle(my_win, &paddle, true);

                        
                    }
                    while(difftime(current, start) < TEN_SEC 
                    && playState == 1);
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
        endwin();

        /*
        int i;
        scanf("%d", &i);
        message m = {i, 0, 0};        
        sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));
         */
    }
    
	return 0;
}