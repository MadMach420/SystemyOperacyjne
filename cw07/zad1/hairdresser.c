#include "header.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>


int semaphores = -1;
int queue_id = -1;


void handle_client(int client) {
    sleep(client);
    printf("Client handled, time: %d\n", client);
}


void work() {
    struct sembuf chair_sembuf;
    chair_sembuf.sem_num = 0;
    chair_sembuf.sem_flg = 0;

    struct sembuf q_sembuf;
    q_sembuf.sem_num = 1;
    q_sembuf.sem_flg = 0;

    while (1) {
        chair_sembuf.sem_op = -1;
        semop(semaphores, &chair_sembuf, 1);

        q_sembuf.sem_op = -1;
        semop(semaphores, &q_sembuf, 1);

        queue* queue = shmat(queue_id, NULL, 0);

        int client = queue->client_queue[queue->next_client];
        if (client != -1) {
            queue->client_queue[queue->next_client] = -1;
            queue->next_client = (queue->next_client + 1) % queue->length;
        }

        shmdt(queue);
        q_sembuf.sem_op = 1;
        semop(semaphores, &q_sembuf, 1);

        if (client != -1) handle_client(client);

        chair_sembuf.sem_op = 1;
        semop(semaphores, &chair_sembuf, 1);
    }
}


int main() {
    semaphores = semget(ftok("/", 1), 0, 0);
    queue_id = shmget(ftok("/queue", 1), 0, 0);

    work();
}
