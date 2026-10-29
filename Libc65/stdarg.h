//Variable arguments.  Arguments are pushed last first, each at its own size, so the ones after the last named
//argument are above it on the stack.  The W65C816 pushes in 16 bit units, so a char or bool takes 2 bytes.
#ifndef __LIBC65_STDARG_H__
#define __LIBC65_STDARG_H__


typedef char*		va_list;

#if defined(__W65C816__)
#define va_start(ap, last)	((ap) = (char*)&(last) + ((sizeof(last) + 1) & ~1))
#else
#define va_start(ap, last)	((ap) = (char*)&(last) + sizeof(last))
#endif // __W65C816__
#define va_arg(ap, type)	(*(type*)(((ap) += sizeof(type)) - sizeof(type)))
#define va_copy(dest, src)	((dest) = (src))
#define va_end(ap)			((ap) = 0)


#endif // __LIBC65_STDARG_H__
