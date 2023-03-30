#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

void handler_info(int sig, siginfo_t *info, void *ucontext) {
    printf("Nr sygnalu: %d\n", sig);
    printf("Proces wysylacjacy sygnal: %d\n", info->si_pid);
    printf("UID procesu wysylajacego sygnal: %d\n", info->si_uid);
    printf("Czas uzytkownika: %ld\n", info->si_utime);
    printf("Czas systemowy: %ld\n", info->si_stime);
}


void handler(int sig) {
    printf("Nr sygnalu: %d\n", sig);
}


int main() {
    sigset_t mask;
    sigemptyset(&mask);
    struct sigaction action;
    action.sa_mask = mask;

    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler_info;
//    action.sa_flags = SA_RESETHAND;
//    action.sa_flags = SA_NODEFER;
//    action.sa_handler = handler;

    sigaction(SIGUSR1, &action, NULL);
    raise(SIGUSR1);
}
