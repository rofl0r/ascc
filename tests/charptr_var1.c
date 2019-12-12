int puts(char*x){}
int strlen(char*x){}
char* foo(char*x) {
	x[0] = 'H';
	x[1+(1*0)] = 'i';
	x[2] = 0;
	return x;
}
int main() {
	char buf[16];
	char *p = buf;
	foo(p);
	puts(p);
	puts(buf);
	return strlen(buf);
}
