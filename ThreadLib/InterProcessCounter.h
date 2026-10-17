#ifndef __INTER_PROCESS_COUNTER_H__
#define __INTER_PROCESS_COUNTER_H__
#include "AtomicInt.h"
#include "InterProcessFlow.h"


struct SInterProcessCounter
{
	CAtomicInt	iCount;
};


//A counter in named shared memory.  One process Creates it, starting at 0, and
//the others Open it by the same name.
class CInterProcessCounter : public CInterProcessFlow
{
protected:
	SInterProcessCounter*	mpsCounter;

public:
	void	Init(char* szSharedMemoryName);
	void	Init(char* szSharedMemoryName, char* szSharedMemoryNamePostfix);
	void	Kill(void);

	bool	Create(void);
	bool	Open(void);

	int		Increment(void);
	int		Get(void);
	bool	WaitUntilAtLeast(int iCount, int iTimeoutMilliseconds);
};


#endif // __INTER_PROCESS_COUNTER_H__

