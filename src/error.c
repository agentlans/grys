#include <stdio.h>
#include "error.h"
/*
void gerror(gcry_error_t err) {
    fprintf(stderr, "Error: %s\n",
        gcry_strerror(err));
}*/

void show_error(const char* what) {
    fprintf(stderr, "Error: %s\n", what);
}

void warn(const char* what) {
    fprintf(stderr, "Warning: %s\n", what);
}

void ok(const char* what) {
    fprintf(stderr, "OK: %s\n", what);
}

int is_empty(const char* str) {
    return (str[0] == 0);
}