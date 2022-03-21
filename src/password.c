#include <stdio.h>
#include <unistd.h>

#ifdef __gnu_linux__
#include <termios.h>
#endif

#ifdef __gnu_linux__
// max_len: maximum length of password not including trailing NULL
void ask_password(char* password, size_t max_len) {
	// Keystroke detection based on 
	// https://stackoverflow.com/a/3571400

	// Get the terminal properties
	struct termios attr;
	tcgetattr(0, &attr);

	// Switch to non-canonical mode
	tcflag_t original_flags = attr.c_lflag;
	attr.c_lflag &= ~ICANON & ~ECHO;
	tcsetattr(0, TCSANOW, &attr);

	// Read the characters and print asterisks
	int i;
	for (i = 0; i < max_len; ++i) {
		int c = getchar();
		if (c == '\n') {
			break;
		} else {
			password[i] = c;
		}
		printf("*");
	}
	// Trailing NULL
	password[i] = 0;

	// Switch back to the original mode
	attr.c_lflag = original_flags;
	tcsetattr(0, TCSANOW, &attr);

	// To make up for the user's newline
	printf("\n");
}
#endif

// Reads up to max_len bytes into buf then appends a trailing NULL
int read_string_from_file(char* buf, const char* filename, size_t max_len) {
	FILE* f = fopen(filename, "rb");
	if (!f) return 1;
	int len = fread(buf, 1, max_len, f);
	buf[len] = 0;
	fclose(f);
}
