#include "funcdecls.h"

int atoi(char* s) {
	int res = 0;
	while(s[0]) {
		int c = s[0];
		if(c < '0' || c > '9') break;
		res = res*10 + (c-'0');
		s++;
	}
	return res;
}

#ifndef LIBRARY
int main(int argc, ARGV argv) {
	return atoi(argv[1]);
}
#endif
