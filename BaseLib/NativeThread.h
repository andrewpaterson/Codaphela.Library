#ifndef __NATIVE_THREAD_H__
#define __NATIVE_THREAD_H__
#include "PrimitiveTypes.h"


//Large enough to hold a Windows thread HANDLE or a Linux pthread_t.
#define NATIVE_THREAD_HANDLE_SIZE		8


typedef void (*NativeThreadFunction)(void* pvData);


//An operating system thread.  Start runs fFunction(pvData) on a new thread.
//A started thread must be either joined or detached.  A detached
//CNativeThread can be freed while its thread is still running.
class CNativeThread
{
protected:
	alignas(8) uint8	maThread[NATIVE_THREAD_HANDLE_SIZE];
	bool				mbStarted;

public:
	void	Init(void);
	void	Kill(void);

	bool	Start(NativeThreadFunction fFunction, void* pvData);
	void	Join(void);
	void	Detach(void);
	bool	IsStarted(void);
};


struct SNativeThreadStart
{
	NativeThreadFunction	fFunction;
	void*					pvData;
};


SNativeThreadStart*	NewNativeThreadStart(NativeThreadFunction fFunction, void* pvData);
void				RunNativeThreadStart(SNativeThreadStart* psStart);


void	ThreadYield(void);
void	ThreadSleep(int iMilliseconds);
int		NumHardwareThreads(void);
int		CurrentThreadId(void);


#endif // __NATIVE_THREAD_H__

