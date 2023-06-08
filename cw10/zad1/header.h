#define MAX_CLIENTS 64
#define PING_INTERVAL 10


#define MSG_LEN 256
#define NICKNAME_LEN 64


typedef enum {
    msg_ping,
    msg_username_taken,
    msg_server_full,
    msg_disconnect,
    msg_get,
    msg_list,
    msg_one,
    msg_all,
    msg_stop,
} message_type;


typedef struct {
    message_type type;
    char message[MSG_LEN];
    char additional_data[MSG_LEN];
} message;