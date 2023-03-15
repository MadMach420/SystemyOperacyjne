#include <stdio.h>
#include <time.h>
#include <string.h>

void reverse_string(char buffer[]) {
    size_t n = strlen(buffer);
    char reversed[1024];
    for (int i = 0; i < n; ++i) {
        reversed[i] = buffer[n - 1 - i];
    }
    reversed[n] = '\0';
    strcpy(buffer, reversed);
}


long reverse_by_block(FILE* input, FILE* output) {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    fseek(input, 0, SEEK_END);
    long file_size = ftell(input);
    if (file_size >= 1024) {
        fseek(input, -1024, SEEK_CUR);
        file_size -= 1024;
    }

    char buffer[1025];

    while (file_size >= 1024) {
        fread(buffer, 1, 1024, input);
        buffer[1024] = '\0';
        reverse_string(buffer);
        fwrite(buffer, 1, 1024, output);
        fseek(input, -2048, SEEK_CUR);
        file_size -= 1024;
    }
    fread(buffer, 1, 1024, input);
    buffer[1024] = '\0';
    reverse_string(buffer);
    fwrite(buffer, 1, 1024, output);
    fseek(input, -(1024 + file_size), SEEK_CUR);
    fread(buffer, 1, file_size, input);
    buffer[file_size] = '\0';
    reverse_string(buffer);
    fwrite(buffer, 1, file_size, output);

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    return end.tv_nsec - start.tv_nsec;
}


long reverse_by_byte(FILE* input, FILE* output) {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    fseek(input, 0, SEEK_END);
    long file_size = ftell(input);
    if (file_size > 0) {
        fseek(input, -1, SEEK_CUR);
        file_size--;
    }

    char buffer[2];
    while (file_size > 0) {
        fread(buffer, 1, 1, input);
        fwrite(buffer, 1, 1, output);
        fseek(input, -2, SEEK_CUR);
        file_size--;
    }
    fread(buffer, 1, 1, input);
    fwrite(buffer, 1, 1, output);

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    return end.tv_nsec - start.tv_nsec;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments!\n");
    } else {
        FILE* input = fopen(argv[1], "r");
        FILE* output = fopen(argv[2], "w");
        if (!input || !output) {
            printf("Cannot open file!\n");
            return 1;
        }

        long by_byte = reverse_by_byte(input, output);
        printf("By byte: %ld\n", by_byte);

        long by_block = reverse_by_block(input, output);
        printf("By block: %ld\n", by_block);

        fclose(input);
        fclose(output);
    }
}
