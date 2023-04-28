#include "header.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>


int semaphores = -1;
int queue_id = -1;


void handler(int signum) {
    shmctl(queue_id, IPC_RMID, NULL);

    semctl(semaphores, 0, IPC_RMID);
    semctl(semaphores, 1, IPC_RMID);

    kill(-1 * getpid(), SIGTERM);
}


void init_queue() {
    key_t key = ftok("/queue", 1);
    queue_id = shmget(key, sizeof(queue), IPC_CREAT | 0660);
    queue* q = shmat(queue_id, NULL, 0);
    q->next_client = 0;
    q->next_client = 0;
    q->length = P;
    for (int i = 0; i < P; ++i) q->client_queue[i] = -1;
    shmdt(q);
}


void start() {
    struct sembuf q_sembuf;
    q_sembuf.sem_num = 1;
    q_sembuf.sem_flg = 0;
    while (1) {
        q_sembuf.sem_op = -1;
        semop(semaphores, &q_sembuf, 1);

        printf("Client arrived!\n");

        queue* queue = shmat(queue_id, NULL, 0);
        queue->client_queue[queue->next_space] = random() % 10 + 1;
        queue->next_space = (queue->next_space + 1) % queue->length;
        shmdt(queue);

        q_sembuf.sem_op = 1;
        semop(semaphores, &q_sembuf, 1);

        sleep(random() % 6 + 3);
    }
}


int main() {
    sigset_t mask;
    sigemptyset(&mask);
    struct sigaction action;
    action.sa_mask = mask;
    action.sa_flags = 0;
    action.sa_handler = handler;
    sigaction(SIGINT, &action, NULL);

    srand(time(NULL));

    semaphores = semget(ftok("/", 1), 2, IPC_CREAT | 0660);

    union semun arg;
    arg.val = N;
    semctl(semaphores, 0, SETVAL, arg);
    arg.val = 1;
    semctl(semaphores, 1, SETVAL, arg);

    init_queue();

    for (int i = 0; i < M; ++i) {
        if(fork() == 0) {
            execl("./hairdresser", "hairdresser", NULL);
        }
    }

    printf("Clients arriving!\n");
    start();
}
