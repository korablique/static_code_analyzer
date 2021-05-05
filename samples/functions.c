unsigned int do_something(double x, int a, float b) {
	double res = x/b + a;
	while (a) {
		while(b) {
			++res;
		}
	}
	return res;
}

void print(double x) {
	printf("%lf\n", x);
}

int main() {
	double x = 0;
	x = do_something(x);
	for (int i = 0; i < n; ++i) {
		print(do_something(x));
	}
}