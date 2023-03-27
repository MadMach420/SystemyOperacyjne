#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


void compare_file_beginnings(char* file_path, char* file_beginning) {
    FILE* file = fopen(file_path, "r");
    char* buffer = malloc(strlen(file_beginning) + 1);

    fread(buffer, 1, strlen(file_beginning), file);
    if (strcmp(buffer, file_beginning) == 0) {
        printf("File path: %s, PID: %d\n", file_path, getpid());
    }

    free(buffer);
    fclose(file);
}


void browse_directory(char* dir_path, char* file_beginning) {
    DIR* dir = opendir(dir_path);
    struct dirent *dirent;
    struct stat *stat_buf = malloc(sizeof(struct stat));
    char *new_path = malloc((PATH_MAX + 1) * sizeof(char));

    while ((dirent = readdir(dir))) {
        if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0) {
            new_path[0] = '\0';
            strcat(new_path, dir_path);
            strcat(new_path, "/");
            strcat(new_path, dirent->d_name);

            stat(new_path, stat_buf);
            if (S_ISDIR(stat_buf->st_mode)) {
                if (fork() == 0) {
                    browse_directory(new_path, file_beginning);
                    exit(EXIT_SUCCESS);
                } else {
                    wait(NULL);
                }
            } else {
                compare_file_beginnings(new_path, file_beginning);
            }
        }
    }

    free(stat_buf);
    free(new_path);
    closedir(dir);
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("%s\n", argv[2]);
        printf("Wrong number of arguments!\n");
    } else {
        browse_directory(argv[1], argv[2]);
    }
    return 0;
}