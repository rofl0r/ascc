#include "funcdecls.h"

int main(int argc, ARGV argv) {
	int x = 1;
	while(argv[x]) {
		puts(argv[x]);
		x++;
	}
	return x;
}
