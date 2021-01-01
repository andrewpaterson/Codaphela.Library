#ifndef __INTER_PROCESS_WAIT_H__
#define __INTER_PROCESS_WAIT_H__
#include <atomic>
#include "SharedMemory.h"


struct SInterProcessWait
{
	std::atomic<int> iWait;
};


class CInterProcessWait
{
protected:
	SInterProcessWait*	mpsWait;
	CSharedMemory		mcSharedMemory;

public:
	void Init(char* szSharedMemoryName);
	void Kill(void);

	void Start(void);
	void Wait(void);
	void Stop(void);
};


#endif // __INTER_PROCESS_WAIT_H__


