#define M 10
#define N 6
#define P 4


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};


typedef struct {
    int length;
    int next_space;
    int next_client;
    int client_queue[P];
} queue;
