#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
    char** buffers;
    int max_size;
    int current_size;
} BufferStruct;


BufferStruct init_buffer_struct(int size) {
    BufferStruct new_buffer;
    new_buffer.buffers = calloc(4, sizeof (char*));
    new_buffer.max_size = size;
    new_buffer.current_size = 0;
    return new_buffer;
}


char* read_wc_to_buffer(char* filename) {
    char tmp_name[32];
    tmp_name[0] = '\0';
    strcat(tmp_name, "/tmp/tmp_file_XXXXXX");
    int tmp_file_desc = mkstemp(tmp_name);

    char command[100];
    command[0] = '\0';
    strcat(command, "wc ");
    strcat(command, filename);
    strcat(command, " > ");
    strcat(command, tmp_name);
    system(command);void resize_buffer_struct(BufferStruct* buffer_struct);


    FILE* tmp_file = fdopen(tmp_file_desc, "rb");
    char* wc_buffer = NULL;
    size_t size;
    getdelim(&wc_buffer, &size, '\0', tmp_file);
    fclose(tmp_file);
    unlink(tmp_name);

    return wc_buffer;
}


void wc_file_to_buffer_struct(BufferStruct* buffer_struct, char* filename) {
    char* wc_buffer = read_wc_to_buffer(filename);
    if (buffer_struct->current_size == buffer_struct->max_size) {
        printf("Array full!");
    } else {
        buffer_struct->buffers[buffer_struct->current_size++] = wc_buffer;
    }
}


char* get_buffer_at_index(BufferStruct* buffer_struct, int i) {
    return i < buffer_struct->current_size ? buffer_struct->buffers[i] : NULL;
}


void remove_buffer_at_index(BufferStruct* buffer_struct, int i) {
    if (buffer_struct->current_size <= i) return;
    memmove(&buffer_struct->buffers[i],
           &buffer_struct->buffers[i + 1],
           (sizeof (char*)) * (--(buffer_struct->current_size) - i));
}


void free_buffers(BufferStruct* buffer_struct) {
    for (int i = 0; i < buffer_struct->current_size; ++i) {
        free(buffer_struct->buffers[i]);
    }
    free(buffer_struct->buffers);
    buffer_struct->current_size = 0;
    buffer_struct->max_size = 0;
}
