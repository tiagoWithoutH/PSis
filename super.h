#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <sys/socket.h>
#include <sys/un.h>

#include<arpa/inet.h>

#include<ctype.h>
//typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;
typedef enum msg_type {CONNECT, PADDLE_MOVE, DISCONNECT} msg_type;

typedef struct ball_position_t{
    int x, y;
    int up_hor_down; //  -1 up, 0 horizontal, 1 down
    int left_ver_right; //  -1 left, 0 vertical,1 right
    char c;
} ball_position_t;

typedef struct paddle_position_t{
    int x, y;
    int length;
} paddle_position_t;

typedef struct message
{
    msg_type type;
    int key;
}message;

typedef struct board_update
{   
    ball_position_t ball;
    int own_paddle;
    paddle_position_t paddles[10];
    int scores[10];
    int nbClients;
}board_update;

// #define FIFO_NAME "/tmp/fifo_snail"
#define SOCK_PORT 5000 
#define WINDOW_SIZE 20
#define PADDLE_SIZE 2
