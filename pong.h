
typedef enum msg_type {CONNECT, RELEASE, SEND, MOVE, DISCONNECT} msg_type;

#define SOCKET_NAME "/tmp/sock_snail"
#define SOCK_PORT 5000

#define WINDOW_SIZE 20
#define PADDLE_SIZE 2

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
    //ball_position_t ball;
    //paddle_position_t paddle;
    int pos_x;
    int pos_y;
    int move_x;
    int move_y;
}message;
