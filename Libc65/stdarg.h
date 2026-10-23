//Variable arguments.  Arguments are pushed last first, each at its own size, so the ones after the last named
//argument are above it on the stack.
#ifndef __LIBC65_STDARG_H__
#define __LIBC65_STDARG_H__


typedef char*		va_list;

#define va_start(ap, last)	((ap) = (char*)&(last) + sizeof(last))
#define va_arg(ap, type)	(*(type*)(((ap) += sizeof(type)) - sizeof(type)))
#define va_copy(dest, src)	((dest) = (src))
#define va_end(ap)			((ap) = 0)


#endif // __LIBC65_STDARG_H__
