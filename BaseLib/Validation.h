#ifndef __VALIDATION_H__
#define __VALIDATION_H__
#include <csignal>

#ifdef _MSC_VER
#define Break()	__asm int 3;

/* Should use:
 * #define Break()	std::raise(SIGABRT);
 */
#endif // _MSC_VER


#ifdef LINUX_GNU_32
#define Break()	std::raise(SIGINT);
#endif // LINUX_GNU_32


#define Validate(test)	if (!test) Break();


#endif // __VALIDATION_H__

