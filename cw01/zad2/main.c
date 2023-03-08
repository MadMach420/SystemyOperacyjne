#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <regex.h>
#include "buffer.h"


void read_input() {
    BufferStruct buffer_struct;
    int buffer_initialized = 0;
    char line[128];

    regex_t number, file;
    regcomp(&number, "^[0-9][0-9]*$", 0);
    regcomp(&file, "..*", 0);

    printf("> ");
    while (fgets(line, 128, stdin)) {
        char* command = strtok(line, " \n\0");
        char* arg = strtok(NULL, " \n\0");
        if (!strcmp(command, "quit")) break;

        struct tms start;
        struct timespec start_realtime;
        struct tms end;
        struct timespec end_realtime;
        clock_gettime(CLOCK_REALTIME, &start_realtime);
        times(&start);
//        printf("%ld, %ld, %ld, %ld\n", start.tms_utime, start.tms_stime, start.tms_cstime, start.tms_cutime);

        if (buffer_initialized) {
            if (!strcmp(command, "init")) {
                printf("Array already initialized!\n");
            } else if (!strcmp(command, "count")) {
                if (arg && !regexec(&file, arg, 0, NULL, 0)) {
                    wc_file_to_buffer_struct(&buffer_struct, arg);
                } else {
                    printf("Wrong arguments!\n");
                }
            } else if (!strcmp(command, "show")) {
                if (arg && !regexec(&number, arg, 0, NULL, 0)) {
                    printf("%s", get_buffer_at_index(&buffer_struct, atoi(arg)));
                } else {
                    printf("Wrong arguments!\n");
                }
            } else if (!strcmp(command, "delete")) {
                if (arg && !regexec(&number, arg, 0, NULL, 0)) {
                    remove_buffer_at_index(&buffer_struct, atoi(arg));
                } else {
                    printf("Wrong arguments!\n");
                }
            } else if (!strcmp(command, "destroy")) {
                free_buffers(&buffer_struct);
                buffer_initialized = 0;
            }
        } else {
            if (!strcmp(command, "init")) {
                if (arg && !regexec(&number, arg, 0, NULL, 0)) {
                    buffer_struct = init_buffer_struct(atoi(arg));
                    buffer_initialized = 1;
                } else {
                    printf("Wrong arguments!\n");
                }
            } else {
                printf("Array not initialized!\n");
            }
        }

        clock_gettime(CLOCK_REALTIME, &end_realtime);
        times(&end);
//        printf("%ld, %ld, %ld, %ld\n", end.tms_utime, end.tms_stime, end.tms_cstime, end.tms_cutime);
        clock_t realtime = end_realtime.tv_nsec - start_realtime.tv_nsec;
        clock_t usertime = end.tms_utime - start.tms_utime;
        clock_t systime = end.tms_stime - start.tms_stime;
        printf("Time: real: %ld ns, user: %ld ticks, system: %ld ticks\n", realtime, usertime, systime);
        printf("> ");
    }
}


int main () {
    read_input();
}