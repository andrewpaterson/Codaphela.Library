#ifndef __VALIDATION_H__
#define __VALIDATION_H__
#include <signal.h>

#ifdef _MSC_VER
#define BREAK()	__debugbreak();
#endif // _MSC_VER


#ifdef WIN_GNU_32
#define BREAK()	__asm__ volatile("int3\n\tnop");  //nop keeps the debugger on the breaking line.
#endif // WIN_GNU_32


#ifdef LINUX_GNU
#define BREAK()	__asm__ volatile("int3\n\tnop");  //nop keeps the debugger on the breaking line.
#endif // LINUX_GNU


#ifdef __LIBC65__
#define BREAK()	__builtin_trap();
#endif // __LIBC65__


//What the environment variable BASELIB_BREAK asks of a run: "0" turns its breaks off (the logger's on an error or a
//warning, and a failed Validate's), "1" turns the logger's on, and anything else leaves them as the build has them.
enum EBreakSwitch
{
	BSW_AsBuilt,
	BSW_Off,
	BSW_On
};


EBreakSwitch	ParseBreakSwitch(const char* szValue);
EBreakSwitch	GetBreakSwitch(void);


#define Validate(test)	if (!test && (GetBreakSwitch() != BSW_Off)) BREAK();


#endif // __VALIDATION_H__

