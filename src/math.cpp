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
static float _sinf(float x) {
    constexpr float n3P = -(1.0f / factorial(3));
    constexpr float n5P = +(1.0f / factorial(5));
    constexpr float n7P = -(1.0f / factorial(7));
    float x2 = x * x;
    return x * (1.0f + x2 * (n3P + x2 * (n5P + x2 * n7P)));
}
static double _cos(double x) {
	constexpr double n2P = -(1.0 / factorial(2));
	constexpr double n4P = +(1.0 / factorial(4));
	constexpr double n6P = -(1.0 / factorial(6));
	constexpr double n8P = +(1.0 / factorial(8));
	constexpr double n10P = -(1.0 / factorial(10));
	constexpr double n12P = +(1.0 / factorial(12));
	double x2 = x * x;
	return 1.0 + x2 * (n2P + x2 * (n4P + x2 * (n6P + x2 * (n8P + x2 * (n10P + x2 * n12P)))));
}
static float _cosf(float x) {
	constexpr float n2P = -(1.0f / factorial(2));
	constexpr float n4P = +(1.0f / factorial(4));
	constexpr float n6P = -(1.0f / factorial(6));
	constexpr float n8P = +(1.0f / factorial(8));
	float x2 = x * x;
	return 1.0f + x2 * (n2P + x2 * (n4P + x2 * (n6P + x2 * n8P)));
}
double sin(double x) {
	long long n = round(x / (MATH_PI / 2));
	double r = x - n * (MATH_PI / 2);
	if ((n & 3) == 0) return _sin(r);
	if ((n & 3) == 1) return _cos(r);
	if ((n & 3) == 2) return -_sin(r);
	return -_cos(r);
}
float sinf(float x) {
	long long n = round(x / (MATH_PI / 2));
	float r = x - n * (MATH_PI / 2);
	if ((n & 3) == 0) return _sinf(r);
	if ((n & 3) == 1) return _cosf(r);
	if ((n & 3) == 2) return -_sinf(r);
	return -_cosf(r);
}
double cos(double x) {
	long long n = round(x / (MATH_PI / 2));
	double r = x - n * (MATH_PI / 2);
	if ((n & 3) == 0) return _cos(r);
	if ((n & 3) == 1) return -_sin(r);
	if ((n & 3) == 2) return -_cos(r);
	return _sin(r);
}
float cosf(float x) {
	long long n = round(x / (MATH_PI / 2));
	float r = x - n * (MATH_PI / 2);
	if ((n & 3) == 0) return _cosf(r);
	if ((n & 3) == 1) return -_sinf(r);
	if ((n & 3) == 2) return -_cosf(r);
	return _sinf(r);
}
double sqrt(double x) {
	return __builtin_sqrt(x);
}