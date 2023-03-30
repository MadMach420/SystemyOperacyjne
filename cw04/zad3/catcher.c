#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>

int COUNTER = 0;
int CODE = -1;


void handler(int sig, siginfo_t *info, void *ucontext) {
    COUNTER++;
    CODE = info->si_value.sival_int;
    pid_t sender_pid = info->si_pid;
    kill(sender_pid, SIGUSR1);

    switch (CODE) {
        case 1: {
            for (int i = 1; i <= 100; ++i) {
                printf("%d ", i);
            }
            printf("\n");
            break;
        }
        case 2: {
            time_t now = time(NULL);
            printf("Aktualny czas: %ld\n", now);
            break;
        }
        case 3: {
            printf("Liczba zadan: %d\n", COUNTER);
            break;
        }
        case 4: {
            break;
        }
        case 5: {
            exit(0);
        }
        default: {
            printf("Niepoprawny kod!");
            exit(1);
        }
    }
}


int main() {
    printf("Catcher PID: %d\n", getpid());
    sigset_t mask;
    sigemptyset(&mask);
    struct sigaction action;
    action.sa_mask = mask;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler;

    sigaction(SIGUSR1, &action, NULL);

    while (1) {
        if (CODE == 4) {
            time_t now = time(NULL);
            printf("Aktualny czas: %ld\n", now);
            sleep(1);
        }
    };
}