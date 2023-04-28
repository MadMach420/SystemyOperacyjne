#define M 10
#define N 6
#define P 4


typedef struct {
    int length;
    int next_space;
    int next_client;
    int client_queue[P];
} queue;