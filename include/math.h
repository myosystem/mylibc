#ifndef __MATH_H__
#define __MATH_H__
#define MATH_PI 3.14159265358979323846
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

double floor(double x);
double ceil(double x);
double round(double x);
double sin(double x);
float sinf(float x);
double cos(double x);
float cosf(float x);
double sqrt(double x);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif /* __MATH_H__ */