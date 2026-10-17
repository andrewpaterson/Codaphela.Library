#ifndef __INTER_PROCESS_DONE_H__
#define __INTER_PROCESS_DONE_H__
#include "AtomicInt.h"
#include "InterProcessFlow.h"


struct SInterProcessDone
{
	CAtomicInt	iDone;
};


class CInterProcessDone : public CInterProcessFlow
{
protected:
	SInterProcessDone*	mpsDone;

public:
	void Init(char* szSharedMemoryName);
	void Init(char* szSharedMemoryName, char* szSharedMemoryNamePostfix);
	void Kill(void);

	void Start(void);
	void Open(void);
	bool IsDone(int iCount);
	void Wait(int iCount);
	void Stop(void);
};


#endif // __INTER_PROCESS_WAIT_H__

