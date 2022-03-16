#include <stdio.h>
#include <stdlib.h>

#include "file_check.h"

// Tries to make a file unrecoverable
// Returns nonzero value if there's a problem
int shred(const char* filename) {
    size_t size = file_size(filename);
    FILE* f = fopen(filename, "wb");
    if (!f) return 1;
    // Fill with random bytes
    for (size_t i = 0; i < size; ++i) {
        char x = rand() % 256;
        fwrite(&x, 1, 1, f);
    }
    fclose(f);
    // Delete the file
    int err = remove(filename);
    return err;
}
