#ifndef __VALIDATION_H__
#define __VALIDATION_H__
#include <csignal>

#ifdef _MSC_VER
#define BREAK()	__debugbreak();
#endif // _MSC_VER


#ifdef LINUX_GNU_32
#define BREAK()	std::raise(SIGINT);
#endif // LINUX_GNU_32


#define Validate(test)	if (!test) BREAK();


#endif // !__VALIDATION_H__

