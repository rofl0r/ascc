int puts(char*x){}
int strlen(char*x){}
int main() {
	char buf[16];
	char *x = buf; //+1;
	buf[0] = 'H';
	x[0] = 'i';
	x++;
	x[0] = 0;
	puts(x);
	puts(buf);
	return strlen(buf);
}
