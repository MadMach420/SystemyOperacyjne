#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>


pthread_cond_t elf_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t elf_mutex = PTHREAD_MUTEX_INITIALIZER;
int elves_waiting = 0;
pthread_cond_t reindeer_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t reindeer_mutex = PTHREAD_MUTEX_INITIALIZER;
int reindeer_waiting = 0;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;


void handler(int _) {
    pthread_cond_destroy(&elf_cond);
    pthread_mutex_destroy(&elf_mutex);
    pthread_cond_destroy(&reindeer_cond);
    pthread_mutex_destroy(&reindeer_mutex);
    pthread_cond_destroy(&santa_cond);
    pthread_mutex_destroy(&santa_mutex);
}


void* elf(void* _) {
    while (1) {
        sleep(random() % 4 + 2);

        pthread_mutex_lock(&elf_mutex);

        if (elves_waiting >= 3) {
            printf("Elf: sam rozwiązuje swój problem, ID: %lu\n", pthread_self());
        } else {
            elves_waiting++;

            if (elves_waiting == 3) {
                printf("Elf: wybudzam Mikołaja, ID: %lu\n", pthread_self());
                pthread_cond_broadcast(&santa_cond);
            } else {
                printf("Elf: czeka %d elfów na Mikołaja, ID: %lu\n", elves_waiting, pthread_self());
            }

            while (elves_waiting != 0) {
                pthread_cond_wait(&elf_cond, &elf_mutex);
            }
        }

        pthread_mutex_unlock(&elf_mutex);
    }
}


void* reindeer(void* _) {
    while (1) {
        sleep(random() % 6 + 5);

        pthread_mutex_lock(&reindeer_mutex);

        reindeer_waiting++;

        if (reindeer_waiting == 9) {
            printf("Renifer: wybudzam Mikołaja, ID: %lu\n", pthread_self());
            pthread_cond_broadcast(&santa_cond);
        } else {
            printf("Renifer: czeka %d reniferów na Mikołaja, ID: %lu\n", reindeer_waiting, pthread_self());
        }

        while (reindeer_waiting != 0) {
            pthread_cond_wait(&reindeer_cond, &reindeer_mutex);
        }

        pthread_mutex_unlock(&reindeer_mutex);
    }
}


void santa() {
    pthread_mutex_lock(&santa_mutex);
    while (1) {
        printf("Mikołaj: zasypiam\n");
        while (elves_waiting < 3 && reindeer_waiting < 9) {
            pthread_cond_wait(&santa_cond, &santa_mutex);
        }
        printf("Mikołaj: budzę się\n");

        if (reindeer_waiting == 9) {
            printf("Mikołaj: dostarczam zabawki\n");
            sleep(random() % 3 + 2);

            pthread_mutex_lock(&reindeer_mutex);
            reindeer_waiting = 0;
            pthread_mutex_unlock(&reindeer_mutex);

            pthread_cond_broadcast(&reindeer_cond);
        }
        if (elves_waiting == 3) {
            printf("Mikołaj: rozwiązuję problemy elfów\n");
            sleep(random() % 2 + 1);

            pthread_mutex_lock(&elf_mutex);
            elves_waiting = 0;
            pthread_mutex_unlock(&elf_mutex);

            pthread_cond_broadcast(&elf_cond);
        }
    }
}


int main() {
    signal(SIGINT, handler);
    pthread_t thread_id;
    for (int i = 0; i < 10; ++i) {
        pthread_create(&thread_id, NULL, elf, NULL);
    }
    for (int i = 0; i < 9; ++i) {
        pthread_create(&thread_id, NULL, reindeer, NULL);
    }

    santa();
}