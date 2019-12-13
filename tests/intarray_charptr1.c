#include "funcdecls.h"

int foo(int argc, int *argv) {
	int i;
	for(i=0;i<argc;i++) {
		char* p = argv[i];
		puts(p);
	}
}

int main() {
	int x[4], i = 0;
	x[i] = "foo";
	i++;
	x[i] = "bar";
	i+=1;
	x[i] = "baz";
	i++;
	x[i] = "qux";
	i++;
	foo(4, x);
	return 0;
}
