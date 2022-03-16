#ifndef _ERRORMSG
#define _ERRORMSG

//#include <gcrypt.h>

void ok(const char *what);
void warn(const char *what);
void show_error(const char *what);
//void gerror(gcry_error_t err);
int is_empty(const char* str);

#endif