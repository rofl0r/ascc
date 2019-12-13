#include "funcdecls.h"
void itoa(int number, char* buffer) {
	int lentest, len = 0, i;

	if(number >> 31) {
		number = number * -1;
		buffer[0] = '-';
		buffer++;
	}
	lentest = number;
	do {
		len++;
		lentest /= 10;
	} while(lentest);
	buffer[len] = 0;
	do {
		i = number % 10;
		buffer[len - 1] = '0' + i;
		number -= i;
		len -= 1;
		number /= 10;
	} while (number);
}

void itoap(int* value, char* buf) {
	itoa(value[0], buf);
}

int main() {
	char buf[128];
	int ints[4];
	int i = 1;
	ints[0] = -1337;
	ints[i] = 0;
	i++;
	ints[i] = 100000;
	ints[i+1] = -5653452;

	itoap(ints, buf);
	puts(buf);
	int *p = ints;
	p++;
	itoap(p, buf);
	puts(buf);
	p = ints;
	//p += 2;
	p++; p++;
	itoap(p, buf);
	puts(buf);
	p++;
	itoap(p, buf);
	puts(buf);
	return 1337/70;
}
