int puts(char*x){}
int strlen(char*x){}
void foo(char*x) {
	x[0] = 'H';
	x[1] = 'i';
	x[2] = 0;
}
int main() {
	char buf[16];
	foo(buf);
	puts(buf);
	return !(strlen(buf) == 2);
}
