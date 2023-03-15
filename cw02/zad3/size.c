#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

void read_directory() {
    char directory_name[1025];
    getcwd(directory_name, 1024);
    DIR* dir = opendir(directory_name);

    long long int total_size = 0;

    struct dirent* dirent;
    struct stat *stat_buf = malloc(sizeof (struct stat));
    char *path = malloc(1025 * sizeof (char));
    while ((dirent = readdir(dir))) {
        path[0] = '\0';
        strcat(path, directory_name);
        strcat(path, "/");
        strcat(path, dirent->d_name);

        stat(path, stat_buf);
        if (!S_ISDIR(stat_buf->st_mode)) {
            printf("Name: %s, size: %ld\n", dirent->d_name, stat_buf->st_size);
            total_size += stat_buf->st_size;
        }
    }
    free(stat_buf);
    free(path);

    printf("Total size: %lld\n", total_size);
    closedir(dir);
}


int main() {
    read_directory();
}