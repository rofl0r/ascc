int main() {
	short x[2];
	short *y = x+1;
	y[0] = 42;
	return x[1];
}
