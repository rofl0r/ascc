int puts(char*x){}
int strlen(char*x){}
//void memcpy(char*d,char*s,int n){}
void fill(char*s) {
	char c = 'A';
	while(c <= 'Z') {
		s[0] = c;
		c++;
		s++;
	}
	s[0] = 0;
}
int main() {
	char buf[64];
	char *s = buf;
	fill(s);
	puts(buf);
	return strlen(buf);
}
