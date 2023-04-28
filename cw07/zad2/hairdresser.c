#include "header.h"
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>


sem_t *queue_sem;
sem_t *chairs_sem;
int queue_fd;


void handle_client(int client) {
    sleep(client);
    printf("Client handled, time: %d\n", client);
}


void work() {
    while (1) {
        sem_wait(chairs_sem);
        sem_wait(queue_sem);

        queue* queue = mmap(NULL, sizeof(queue), PROT_READ | PROT_WRITE, MAP_SHARED, queue_fd, 0);
        int client = queue->client_queue[queue->next_client];
        if (client != -1) {
            queue->client_queue[queue->next_client] = -1;
            queue->next_client = (queue->next_client + 1) % queue->length;
        }
        munmap(queue, sizeof(queue));
        sem_post(queue_sem);

        if (client != -1) handle_client(client);

        sem_post(chairs_sem);
    }
}


int main() {
    chairs_sem = sem_open("/chairs", O_RDWR);
    queue_sem = sem_open("/queue", O_RDWR);
    queue_fd = shm_open("/queue", O_RDWR, 0);

    work();
}