#include <stdio.h>
#include <dirent.h>
#include <ftw.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

int read_directory(const char* directory_name, const struct stat *sb, int typeflag) {
    if (typeflag == FTW_D) {
        DIR *dir = opendir(directory_name);

        long long int total_size = 0;

        struct dirent *dirent;
        struct stat *stat_buf = malloc(sizeof(struct stat));
        char *path = malloc(1025 * sizeof(char));
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

        printf("Name: %s, size: %lld\n", directory_name, total_size);

        closedir(dir);
    }
    return 0;
}


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
    } else {
        ftw(argv[1], read_directory, 0);
    }
}
