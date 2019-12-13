#include "funcdecls.h"
int main() {
	char buf[16];
	puts(strcpy(buf, "foobar"));
	return strlen(buf);
}
