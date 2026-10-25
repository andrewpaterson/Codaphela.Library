//size_t, ptrdiff_t, NULL and offsetof for the 65C64 and the W65C816 (32 bit pointers; long is 32 bits on both).
#ifndef __LIBC65_STDDEF_H__
#define __LIBC65_STDDEF_H__


typedef unsigned long			size_t;
typedef long					ptrdiff_t;
typedef long long				max_align_t;

#ifndef NULL
#define NULL					0
#endif // NULL

#define offsetof(type, member)	((size_t)&(((type*)0)->member))


#endif // __LIBC65_STDDEF_H__
