#ifndef __INTER_PROCESS_HOLD_H__
#define __INTER_PROCESS_HOLD_H__
#include "AtomicInt.h"
#include "InterProcessFlow.h"


struct SInterProcessWait
{
	CAtomicInt	iWait;
};


class CInterProcessHold : public CInterProcessFlow
{
protected:
	SInterProcessWait*	mpsWait;

public:
	void Init(char* szSharedMemoryName);
	void Init(char* szSharedMemoryName, char* szSharedMemoryNamePostfix);
	void Kill(void);

	void Start(void);
	void Touch(void);
	void Wait(void);
	void Stop(void);
};


#endif // __INTER_PROCESS_WAIT_H__

