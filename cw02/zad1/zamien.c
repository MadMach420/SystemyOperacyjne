#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>


long change_characters_lib(char a, char b, char* input_filename, char* output_filename) {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    FILE* input = fopen(input_filename, "r");
    FILE* output = fopen(output_filename, "w");
    if (!input || !output) {
        printf("Cannot open file!\n");
        return -1;
    }

    char c;
    while ((c = fgetc(input)) != EOF) {
        if (c == a) {
            fputc(b, output);
        } else {
            fputc(c, output);
        }
    }

    fclose(input);
    fclose(output);

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    return end.tv_nsec - start.tv_nsec;
}


long change_characters_sys(char a, char b, char* input_filename, char* output_filename) {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    int input_fd = open(input_filename, O_RDONLY);
    int output_fd = open(output_filename, O_WRONLY | O_CREAT);
    if (input_fd < 0 || output_fd < 0) {
        printf("Cannot open file!\n");
        return -1;
    }

    char c;
    int bytes_read = read(input_fd, &c, 1);
    while (!bytes_read) {
        if (c == a) {
            write(output_fd, &b, 1);
        } else {
            write(output_fd, &c, 1);
        }

        bytes_read = read(input_fd, &c, 1);
    }

    close(input_fd);
    close(output_fd);

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    return end.tv_nsec - start.tv_nsec;
}


int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Wrong number of arguments!\n");
    } else {
        long lib_time = change_characters_lib(argv[1][0], argv[2][0], argv[3], argv[4]);
        printf("Lib: %ld ns\n", lib_time);

        long sys_time = change_characters_sys(argv[1][0], argv[2][0], argv[3], argv[4]);
        printf("Sys: %ld ns\n", sys_time);
    }
}
