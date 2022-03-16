#include <unistd.h>
#include <sys/stat.h>

int is_readable(const char* filename) {
    return (access(filename, R_OK) == 0);
}

int is_writable(const char* filename) {
    return (access(filename, W_OK) == 0);
}

int file_exists(const char* filename) {
    return (access(filename, F_OK) == 0);
}

// Returns non-zero if filename is a regular file
int is_regular(const char* filename) {
    struct stat stat_obj;
    stat(filename, &stat_obj);
    return S_ISREG(stat_obj.st_mode);
}

// Returns size of regular file in bytes
size_t file_size(const char* filename) {
    struct stat stat_obj;
    stat(filename, &stat_obj);
    return stat_obj.st_size;
}
