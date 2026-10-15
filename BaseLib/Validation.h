#ifndef __VALIDATION_H__
#define __VALIDATION_H__
#include <csignal>

#ifdef _MSC_VER
#define BREAK()	__debugbreak();
#endif // _MSC_VER


#ifdef WIN_GNU_32
#define BREAK()	__asm__ volatile("int3\n\tnop");  //nop keeps the debugger on the breaking line.
#endif // WIN_GNU_32


#ifdef LINUX_GNU_32
#define BREAK()	__asm__ volatile("int3\n\tnop");  //nop keeps the debugger on the breaking line.
#endif // LINUX_GNU_32


#define Validate(test)	if (!test) BREAK();


#endif // __VALIDATION_H__

