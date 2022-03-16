#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "file_check.h"
#include "gcrypto.h"
#include "password.h"

extern int shred(const char* filename);

#define max_password_len 50
#define max_file_len 300

// Copies option's argument to ptr truncating at
// maximum length
void copy_set(char* dest, const char* src, int max_len) {
    strncpy(dest, src, max_len);
    dest[max_len] = 0; // Trailing null
}

int main(int argc, char** argv) {
    initialize_gcrypt(); // for GCrypt

    int e = 0;
    int d = 0;
    char in_filename[max_file_len+1];
    char out_filename[max_file_len+1];
    char pwd_filename[max_file_len+1];
    char pwd[max_password_len+1];
    int k = 0;
    int f = 0;
    int n = 0;
    int p = 0;

    int c;
    while ((c = getopt(argc, argv, "edf:np:k")) != -1) {
        switch (c) {
            case 'e':
                //printf("Encrypt\n");
                e = 1;
                break;
            case 'd':
                //printf("Decrypt\n");
                d = 1;
                break;
            case 'f':
                //printf("File: %s\n", optarg);
                copy_set(pwd_filename, optarg, max_file_len);
                f = 1;
                break;
            case 'n':
                //printf("Environment\n");
                n = 1;
                break;
            case 'p':
                //printf("Password %s\n", optarg);
                copy_set(pwd, optarg, max_password_len);
                p = 1;
                break;
            case 'k':
                //printf("Delete original file.\n");
                k = 1;
                break;
        }
    }
    // Files
    if (optind + 2 != argc) {
        show_error("Must have exactly two files (input and output files).");
        return 1;
    }
    // Copy the remaining arguments
    copy_set(in_filename, argv[optind], max_file_len);
    copy_set(out_filename, argv[optind+1], max_file_len);

    if (!is_readable(in_filename)) {
        show_error("Input file isn't readable.");
        return 1;
    }
    if (file_exists(out_filename)) {
        show_error("Output file already exists.");
        return 1;
    }
    // See if input file is deletable
    if (k && !is_regular(in_filename)) {
        show_error("Can't delete input file because it's not a regular file.");
        return 1;
    }

    // Action
    if ((e && d) || (!e && !d)) {
        show_error("Must choose whether to encrypt (-e) or decrypt (-d).");
        return 1;
    }

    // Password
    int pwd_ways = f + n + p;
    switch (pwd_ways) {
        case 0:
            if (is_empty(in_filename)) {
                // Can't read password from stdin when stdin is the input file!
                show_error("No password has been specified. Please use the -f or -n options.");
                return 1;
            } else {
                // Ask user for password.
		printf("Please enter password: ");
                ask_password(pwd, max_password_len);
            }
            break;
        case 1:
            if (f == 1) {
                // read password from file (might fail)
                int err = read_string_from_file(pwd, pwd_filename, max_password_len);
                if (err) {
                    show_error("Couldn't read password file.");
                    return 1;
                }
            } else if (n == 1) {
                // Read password from environment variable.
                char* p = getenv("GRYS_PASSWORD");
                if (!p) {
                    show_error("Couldn't read GRYS_PASSWORD environment variable.");
                    return 1;
                }
            } else if (p == 1) {
                // Read password from command line (already done while parsing)
            }
            break;
        default:
            // Too many other parameters
            show_error("Can only choose one method for supplying the password.");
            return 1;
    }

    // Finally open the files
    FILE* in_file = NULL;
    FILE* out_file = NULL;

    // Open input file
    if (is_empty(in_filename)) {
        in_file = stdin;
    } else {
        in_file = fopen(in_filename, "rb");
    }
    // Open output file
    if (is_empty(out_filename)) {
        out_file = stdout;
    } else {
        out_file = fopen(out_filename, "wb");
    }
    if (!in_file) {
        show_error("Couldn't open input file.");
        goto cleanup;
    }
    if (!out_file) {
        show_error("Couldn't open output file.");
    }

    // Carry out the action
    int err = 0;
    if (e == 1) {
        err = encrypt(in_file, out_file, pwd);
    } else if (d == 1) {
        err = decrypt(in_file, out_file, pwd);
    }
    if (err) {
        show_error("Problem occurred during the encryption/decryption process.");
        goto cleanup;
    }

    // Afterwards, shred input file and delete if requested.
    if (k) {
        int err = 0;
        if (is_regular(in_filename)) {
            err = shred(in_filename);
        }
        if (err)
            show_error("Problem occurred while deleting input file.");
    }

    fclose(in_file);
    fclose(out_file);
    return 0;
cleanup:
    if (in_file) fclose(in_file);
    if (out_file) fclose(out_file);
    return 2;
}