#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

double fun(double x) {
    return 4.0 / (x * x + 1);
}


double calculate(double start, double stop, double precision) {
    double i = start, j = start + precision;
    double result = 0;

    while(j < stop) {
        result += fun(j) * precision;
        i = j; j += precision;
    }
    result += fun(stop) * (stop - i);

    return result;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments!\n");
        return 1;
    }
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    double precision = strtod(argv[1], NULL);
    long n = strtol(argv[2], NULL, 10);
    double interval = 1.0 / n;

    int fd[n][2];

    for (int i = 0; i < n; ++i) {
        pipe(fd[i]);

        if (fork() == 0) {
            close(fd[i][0]);

            double result = calculate(i * interval, (i + 1) * interval, precision);
            write(fd[i][1], &result, sizeof(double));

            exit(0);
        }

        close(fd[i][1]);
    }

    while (wait(NULL) > 0);

    double integral = 0;
    for (int i = 0; i < n; ++i) {
        double result;
        read(fd[i][0], &result, sizeof(double));
        integral += result;
        close(fd[i][1]);
    }

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    printf("Integral value: %f\n", integral);
    printf("Time: %ld ns\n", (end.tv_sec * 1000000000 + end.tv_nsec) - (start.tv_sec * 1000000000 + start.tv_nsec));

    return 0;
}
