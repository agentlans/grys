#ifndef _PASSWORDFUNC
#define _PASSWORDFUNC

int read_string_from_file(char *buf,const char *filename,size_t max_len);
void ask_password(char *password,size_t max_len);

#endif