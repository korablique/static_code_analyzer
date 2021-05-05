int main() {
	int a = 5;
	bool b = false;
	do {
		++a;
		int c = 0;
		do {
			c += a;
		} while (!b);
	} while(b);
	printf("%d\n", a);

	do c += a;
	while (!b);
}