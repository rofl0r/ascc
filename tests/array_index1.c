void puts(char*){}
int main() {
	char buf[16];
	int i = 0;
	while(1) {
		if(i >= 10) break;
		buf[i] = 'A';
		i++;
	}
	puts(buf);
	return 0;
}
