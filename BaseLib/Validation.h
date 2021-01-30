#ifndef __VALIDATION_H__
#define __VALIDATION_H__
#include <csignal>

#ifdef _MSC_VER
#define Break()	__debugbreak();
#endif // _MSC_VER


#ifdef LINUX_GNU_32
#define Break()	std::raise(SIGINT);
#endif // LINUX_GNU_32


#define Validate(test)	if (!test) Break();


#endif // __VALIDATION_H__

