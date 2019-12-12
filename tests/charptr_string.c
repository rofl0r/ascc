int puts(char*x){}
int strlen(char*x){}
//void memcpy(char*d,char*s,int n){}
void mystrcpy(char*d,char*s) {
	int n = strlen(s)+1;
	char *e = d+n;
	while(d < e) {
		d[0] = s[0];
		d++; s++;
	}
}
int main() {
	char buf[16];
	mystrcpy(buf, "foobar");
	puts(buf);
	return strlen(buf);
}
