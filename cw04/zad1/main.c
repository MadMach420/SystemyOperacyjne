#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

void set_ignore() {
    signal(SIGUSR1, SIG_IGN);
}


void handler(int sig_no) {
    printf("Received signal %d\n", sig_no);
}


void set_handler() {
    signal(SIGUSR1, handler);
}


void set_mask() {
//    struct sigaction act;
//    act.sa_handler = handler;
//    sigemptyset(&act.sa_mask);
//    act.sa_flags = 0;
//    sigaction(SIGUSR1, &act, NULL);
    sigset_t new_mask;
    sigset_t old_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    sigprocmask(SIG_SETMASK, &new_mask, &old_mask);
}


void check_pending() {
    sigset_t pending;
    sigpending(&pending);
    if (sigismember(&pending, SIGUSR1)) {
        printf("Signal pending\n");
    } else {
        printf("Signal not pending\n");
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
    } else {
        char* argument = argv[1];
        if (strcmp(argument, "ignore") == 0) {
            set_ignore();
        } else if (strcmp(argument, "handler") == 0) {
            set_handler();
        } else if (strcmp(argument, "mask") == 0 || strcmp(argument, "pending") == 0) {
            set_mask();
        } else {
            printf("Invalid argument!\n");
            return 1;
        }

        printf("Parent:\n");
        raise(SIGUSR1);
        if (strcmp(argument, "pending") == 0) {
            check_pending();
        }

        if (fork() == 0) {
            printf("Child:\n");
            if (strcmp(argument, "pending") == 0) {
                check_pending();
            } else {
                raise(SIGUSR1);
            }

            execl("./secondary", "secondary", NULL);
            printf("Exec failed\n");
        } else {
            wait(NULL);
        }
    }
    return 0;
}