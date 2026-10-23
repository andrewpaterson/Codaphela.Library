//size_t, ptrdiff_t, NULL and offsetof for the 65C64 (32 bit pointers).
#ifndef __LIBC65_STDDEF_H__
#define __LIBC65_STDDEF_H__


typedef unsigned int			size_t;
typedef int						ptrdiff_t;
typedef long long				max_align_t;

#ifndef NULL
#define NULL					0
#endif // NULL

#define offsetof(type, member)	((size_t)&(((type*)0)->member))


#endif // __LIBC65_STDDEF_H__
