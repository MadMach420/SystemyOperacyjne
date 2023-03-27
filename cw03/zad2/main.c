#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments!");
    } else {
        setbuf(stdout, NULL);
        printf("%s  ", argv[0]);

        if (fork() == 0) {
            execl("/bin/ls", "ls", argv[1], NULL);
        }

        wait(NULL);
    }

    return 0;
}