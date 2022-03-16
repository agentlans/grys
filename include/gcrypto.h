#ifndef _GCRYPTWRAP
#define _GCRYPTWRAP

int encrypt(FILE *in_file,FILE *out_file,const char *password);
int decrypt(FILE *in_file,FILE *out_file,const char *password);
void print_bytes(char *hex,size_t n);
void initialize_gcrypt();

#endif