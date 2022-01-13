#include <stdlib.h>
#include <ncurses.h>

#include<netinet/in.h>
#include<arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "super.h"

WINDOW * message_win;

void draw_paddle(WINDOW *win, paddle_position_t * paddle, int delete, int own){
    int ch;
    if(delete){
        if(own)
        {
            ch = '=';
        }
        else
        {
            ch = '_';
        }
        
    }else{
        ch = ' ';
    }
    int start_x = paddle->x - paddle->length;
    int end_x = paddle->x + paddle->length;
    for (int x = start_x; x <= end_x; x++){
        wmove(win, paddle->y, x);
        waddch(win,ch);
    }
    wrefresh(win);
}

/*
drwas the inputed paddle,
draw = 0 - erase
draw = 1 - draw predefined character
*/
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



int main(){
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }  
    printf("socket created \n Ready to send\n");

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


    paddle_position_t paddles[10];
    ball_position_t ball;
    message m;
    board_update fromServer;
    char ch;
    
    while(1)
    {
        int n = 0; 
        ch = ' ';
        m.type = CONNECT;
        printf("Press the C key to connect to the server\n");
        while(ch != 'c'){scanf("%c", &ch);}
        sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));

        initscr();		    	/* Start curses mode 		*/
        cbreak();				/* Line buffering disabled	*/
        keypad(stdscr, TRUE);   /* We get F1, F2 etc..		*/
        noecho();			    /* Don't echo() while we do getch */

        /* creates a window and draws a border */
        WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
        box(my_win, 0 , 0);	
        wrefresh(my_win);
        keypad(my_win, true);
        /* creates a window and draws a border */
        message_win = newwin(10, WINDOW_SIZE, WINDOW_SIZE, 0);
        box(message_win, 0 , 0);	
        wrefresh(message_win);


        
        mvwprintw(message_win, 1,1,"entering while");
        wrefresh(message_win);
        while(1)
        {
            // delete old values
            if(n>0){
                draw_ball(my_win, &fromServer.ball, false);
                for(int i=0; i<fromServer.nbClients; i++){
                    draw_paddle(my_win, &fromServer.paddles[i], false, false);
                }
                mvwprintw(message_win, 2,1,"values deleted");
                wrefresh(message_win);
            }
            
            recv(sock_fd, &fromServer, sizeof(board_update), 0);
            n++;

            // draw new values
            for(int i=0; i<fromServer.nbClients; i++)
            {
                if(i == fromServer.own_paddle){
                    draw_paddle(my_win, &fromServer.paddles[i], true, true);
                    mvwprintw(message_win, i,1,"P%d - %d <--\n", i+1, fromServer.scores[i]); 
                }
                else{
                    draw_paddle(my_win, &fromServer.paddles[i], true, false);
                    mvwprintw(message_win, i,1,"P%d - %d\n", i+1, fromServer.scores[i]);
                }
                
            }
            
            draw_ball(my_win, &fromServer.ball, true);
            wrefresh(message_win); 
            int key = -1;
            key = wgetch(my_win);	
            if(key == 'q')
            {
                m.type = DISCONNECT;
                sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));
                break;
            }       
            if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN)
            {
                m.type = PADDLE_MOVE;
                m.key = key;
                sendto(sock_fd, &m, sizeof(message), 0, 
        (const struct sockaddr *)&server_addr, sizeof(server_addr));
            }

            //n = fromServer.nbClients;
            //paddle = fromServer.paddles[fromServer.own_paddle];
            //ball = fromServer.ball;
        }
        endwin();
    }
    exit(0);
}