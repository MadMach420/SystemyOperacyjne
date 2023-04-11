#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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
    if (argc != 4) {
        printf("Wrong number of arguments!\n");
        return 1;
    }

    long i = strtol(argv[1], NULL, 10);
    double precision = strtod(argv[2], NULL);
    long n = strtol(argv[3], NULL, 10);
    double interval = 1.0 / (double) n;

    double start = (double) i * interval;
    double stop = (double) (i + 1) * interval;

    double result = calculate(start, stop, precision);

    FILE* fifo = fopen("./fifo", "w");
    fwrite(&result, sizeof(double), 1, fifo);
    fclose(fifo);

    return 0;
}