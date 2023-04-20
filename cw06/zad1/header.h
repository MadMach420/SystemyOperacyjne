#define STOP 1
#define LIST 2
#define ALL 3
#define ONE 4
#define INIT 5
#define MESSAGE 6

#define PROJECT 'A'

#define MAX_CLIENTS 32
#define SERVER_ID MAX_CLIENTS

#define MAX_MESSAGE_LENGTH 256


typedef struct {
    long type;
    int sender_id;
    char message[MAX_MESSAGE_LENGTH];
    int additional_data;
} custom_message;