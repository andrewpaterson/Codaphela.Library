//Mathematical functions.  The 65C64 generator does not support floating point yet; these are only declared.
#ifndef __LIBC65_MATH_H__
#define __LIBC65_MATH_H__


#define HUGE_VAL		(__builtin_huge_val())
#define HUGE_VALF		(__builtin_huge_valf())
#define INFINITY		(__builtin_inff())
#define NAN				(__builtin_nanf(""))
#define M_PI			3.14159265358979323846


extern "C"
{
double	fabs(double x);
float	fabsf(float x);
double	sqrt(double x);
float	sqrtf(float x);
double	sin(double x);
float	sinf(float x);
double	cos(double x);
float	cosf(float x);
double	tan(double x);
float	tanf(float x);
double	asin(double x);
double	acos(double x);
double	atan(double x);
float	atanf(float x);
double	atan2(double y, double x);
float	atan2f(float y, float x);
double	exp(double x);
float	expf(float x);
double	log(double x);
float	logf(float x);
double	log10(double x);
double	pow(double x, double y);
float	powf(float x, float y);
double	floor(double x);
float	floorf(float x);
double	ceil(double x);
float	ceilf(float x);
double	fmod(double x, double y);
float	fmodf(float x, float y);
double	modf(double x, double* pdInteger);
double	ldexp(double x, int iExponent);
double	frexp(double x, int* piExponent);
double	round(double x);
float	roundf(float x);
double	trunc(double x);
float	truncf(float x);

//long double is double.
long double	fabsl(long double x);
long double	sqrtl(long double x);
long double	floorl(long double x);
long double	ceill(long double x);
long double	truncl(long double x);
long double	roundl(long double x);
long double	fmodl(long double x, long double y);
long double	powl(long double x, long double y);
long double	logl(long double x);
long double	log10l(long double x);
long double	expl(long double x);
}


#endif // __LIBC65_MATH_H__
