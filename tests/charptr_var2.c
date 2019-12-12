int puts(char*x){}
int strlen(char*x){}
int main() {
	char buf[16];
	char *x = buf;
	x[0] = 'H';
	x[1] = 'i';
	x[2] = 0;
	buf[1] = x[1];
	x[0] = buf[0];
	puts(x);
	puts(buf);
	return strlen(buf);
}
