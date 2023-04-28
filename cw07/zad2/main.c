#include "header.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


sem_t *queue_sem;
sem_t *chairs_sem;
int queue_fd;

void handler(int signum) {
    sem_unlink("/queue");
    shm_unlink("/queue");
    kill(-1 * getpid(), SIGTERM);
}


void init_queue() {
    queue_fd = shm_open("/queue", O_RDWR | O_CREAT, 0660);
    ftruncate(queue_fd, sizeof(queue));
    queue* queue = mmap(NULL, sizeof(queue), PROT_READ | PROT_WRITE, MAP_SHARED, queue_fd, 0);
    queue->length = P;
    queue->next_client = 0;
    queue->next_space = 0;
    for (int i = 0; i < P; ++i) queue->client_queue[i] = -1;
    munmap(queue, sizeof(queue));
}


void start() {
    while (1) {
        sem_wait(queue_sem);

        printf("Client arrived!\n");

        queue* queue = mmap(NULL, sizeof(queue), PROT_READ | PROT_WRITE, MAP_SHARED, queue_fd, 0);
        queue->client_queue[queue->next_space] = random() % 10 + 1;
        queue->next_space = (queue->next_space + 1) % queue->length;
        munmap(queue, sizeof(queue));

        sem_post(queue_sem);

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

    chairs_sem = sem_open("/chairs", O_RDWR | O_CREAT, 0660, N);
    queue_sem = sem_open("/queue", O_RDWR | O_CREAT, 0660, 1);

    init_queue();

    for (int i = 0; i < M; ++i) {
        if(fork() == 0) {
            execl("./hairdresser", "hairdresser", NULL);
        }
    }

    printf("Clients arriving!\n");
    start();
}