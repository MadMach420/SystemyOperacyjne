#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

void handler(int sig) {
    printf("Otrzymano sygnal %d\n", sig);
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Not enough arguments!\n");
        return 1;
    }
    int catcher_pid = atoi(argv[1]);
    int i = 2;

    sigset_t mask;
    sigemptyset (&mask);
    struct sigaction action;
    action.sa_mask = mask;
    action.sa_flags = 0;
    action.sa_handler = handler;
    sigaction(SIGUSR1, &action, NULL);

    while (argv[i] != NULL) {
        union sigval val;
        val.sival_int = atoi(argv[i]);
        sigqueue(catcher_pid, SIGUSR1, val);
        i++;
        sigsuspend (&mask);
    }
}