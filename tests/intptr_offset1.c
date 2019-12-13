int main() {
	int x[2];
	int *y = x;
	y[1] = 42;
	int *z = y - 1;
//	z++;
//	z++;
	z += 2;
	if(z[0] != x[1] || y[1] != x[1]) return 1;
	return x[1];
}
