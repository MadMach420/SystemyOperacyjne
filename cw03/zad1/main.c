#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
    } else {
        int child_process_count = atoi(argv[1]);
        pid_t child_pid;

        for (int i = 0; i < child_process_count; ++i) {
            child_pid = fork();

            if (child_pid != 0) {
                waitpid(child_pid, NULL, 0);
            } else {
                printf("Parent PID: %d, child PID: %d\n", getppid(), getpid());
                exit(EXIT_SUCCESS);
            }
        }

        printf("argv[1]: %d\n", child_process_count);
    }
    
    return 0;
}
