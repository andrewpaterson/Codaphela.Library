//Error numbers for the 65C64.  errno is in stdlib.cpp.
#ifndef __LIBC65_ERRNO_H__
#define __LIBC65_ERRNO_H__


#define EDOM		33
#define ERANGE		34
#define EINVAL		22
#define ENOMEM		12


extern int	errno;


#endif // __LIBC65_ERRNO_H__
