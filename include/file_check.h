#ifndef _FILECHECK
#define _FILECHECK

#include <stddef.h>

size_t file_size(const char *filename);
int is_regular(const char *filename);
int is_writable(const char *filename);
int is_readable(const char *filename);
int file_exists(const char* filename);

#endif