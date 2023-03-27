#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

void check_pending() {
    sigset_t pending;
    sigpending(&pending);
    if (sigismember(&pending, SIGUSR1)) {
        printf("Signal pending\n");
    } else {
        printf("Signal not pending\n");
    }
}


int main() {
    printf("Child after exec:\n");
    check_pending();
    raise(SIGUSR1);
    return 0;
}