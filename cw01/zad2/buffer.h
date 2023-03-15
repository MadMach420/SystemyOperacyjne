typedef struct {
    char** buffers;
    int max_size;
    int current_size;
} BufferStruct;

BufferStruct init_buffer_struct(int size);

char* read_wc_to_buffer(char* filename);

void wc_file_to_buffer_struct(BufferStruct* buffer_struct, char* filename);

char* get_buffer_at_index(BufferStruct* buffer_struct, int i);

void remove_buffer_at_index(BufferStruct* buffer_struct, int i);

void free_buffers(BufferStruct* buffer_struct);
