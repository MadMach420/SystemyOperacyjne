#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments!\n");
        return 1;
    }

    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    mkfifo("./fifo", 0777);

    long n = strtol(argv[2], NULL, 10);
    for (long i = 0; i < n; ++i) {
        char i_string[100];
        sprintf(i_string, "%ld", i);

        if (fork() == 0) {
            execl("./secondary","secondary", i_string, argv[1], argv[2], NULL);
        }
    }

    FILE* fifo = fopen("./fifo", "r");
    double integral = 0;
    double value;

    for (long i = 0; i < n; ++i) {
        fread(&value, sizeof(double), 1, fifo);
        integral += value;
    }

    fclose(fifo);

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    printf("Integral value: %f\n", integral);
    printf("Time: %ld ns\n", (end.tv_sec * 1000000000 + end.tv_nsec) - (start.tv_sec * 1000000000 + start.tv_nsec));

    return 0;
}