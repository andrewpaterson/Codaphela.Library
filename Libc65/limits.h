//The limits of the 65C64's and the W65C816's integers: char is signed and 8 bits, long is 32 bits, int is 32 bits on
//the 65C64 and 16 on the W65C816.
#ifndef __LIBC65_LIMITS_H__
#define __LIBC65_LIMITS_H__


#define CHAR_BIT		8
#define MB_LEN_MAX		1

#define SCHAR_MIN		(-128)
#define SCHAR_MAX		127
#define UCHAR_MAX		0xff
#define CHAR_MIN		SCHAR_MIN
#define CHAR_MAX		SCHAR_MAX

#define SHRT_MIN		(-32767 - 1)
#define SHRT_MAX		32767
#define USHRT_MAX		0xffff

#ifdef __W65C816__
#define INT_MIN			(-32767 - 1)
#define INT_MAX			32767
#define UINT_MAX		0xffffU
#else
#define INT_MIN			(-2147483647 - 1)
#define INT_MAX			2147483647
#define UINT_MAX		0xffffffffU
#endif // __W65C816__

#define LONG_MIN		(-2147483647L - 1)
#define LONG_MAX		2147483647L
#define ULONG_MAX		0xffffffffUL

#define LLONG_MIN		(-9223372036854775807LL - 1)
#define LLONG_MAX		9223372036854775807LL
#define ULLONG_MAX		0xffffffffffffffffULL


#endif // __LIBC65_LIMITS_H__
