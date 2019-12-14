#include "funcdecls.h"

void itoa(int number, char* buffer) {
	int lentest, len = 0, i, start;

	if(number >> 31) {
		number = number * -1;
		buffer[0] = '-';
		start = 1;
	} else
		start = 0;

	lentest = number;
	do {
		len++;
		lentest /= 10;
	} while(lentest);
	buffer[start+len] = 0;
	do {
		i = number % 10;
		buffer[start+len - 1] = '0' + i;
		number -= i;
		len -= 1;
		number /= 10;
	} while (number);
}

#ifndef LIBRARY
int main() {
	char buf[128];
	itoa(-1337, buf);
	puts(buf);
	itoa(0, buf);
	puts(buf);
	itoa(100000, buf);
	puts(buf);
	itoa(-5653452, buf);
	puts(buf);
	return 1337/70;
}
#endif

