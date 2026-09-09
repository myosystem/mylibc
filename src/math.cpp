#include <math.h>
double floor(double x) {
    double t = (double)(long long)x;
    return (t > x) ? t - 1.0 : t;
}

double ceil(double x) {
    double t = (double)(long long)x;
    return (t < x) ? t + 1.0 : t;
}

double round(double x) {
    return (x >= 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}
static constexpr unsigned long long factorial(int n) {
    unsigned long long result = 1;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}
static double _sin(double x) {
    constexpr double n3P = -(1.0 / factorial(3));
    constexpr double n5P = +(1.0 / factorial(5));
    constexpr double n7P = -(1.0 / factorial(7));
    constexpr double n9P = +(1.0 / factorial(9));
    constexpr double n11P = -(1.0 / factorial(11));
    double x2 = x * x;
    return x * (1.0 + x2 * (n3P + x2 * (n5P + x2 * (n7P + x2 * (n9P + x2 * n11P)))));
}
static double _cos(double x) {
	constexpr double n2P = -(1.0 / factorial(2));
	constexpr double n4P = +(1.0 / factorial(4));
	constexpr double n6P = -(1.0 / factorial(6));
	constexpr double n8P = +(1.0 / factorial(8));
	constexpr double n10P = -(1.0 / factorial(10));
	double x2 = x * x;
	return 1.0 + x2 * (n2P + x2 * (n4P + x2 * (n6P + x2 * (n8P + x2 * n10P))));
}
double sin(double x) {
	long long n = round(x / (MATH_PI / 2));
	double r = x - n * (MATH_PI / 2);
	if (n % 4 == 0) return _sin(r);
	if (n % 4 == 1) return _cos(r);
	if (n % 4 == 2) return -_sin(r);
	return -_cos(r);
}
double cos(double x) {
	long long n = round(x / (MATH_PI / 2));
	double r = x - n * (MATH_PI / 2);
	if (n % 4 == 0) return _cos(r);
	if (n % 4 == 1) return -_sin(r);
	if (n % 4 == 2) return -_cos(r);
	return _sin(r);
}