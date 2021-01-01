#ifndef __INTER_PROCESS_DONE_H__
#define __INTER_PROCESS_DONE_H__
#include <atomic>
#include "InterProcessFlow.h"


struct SInterProcessDone
{
	std::atomic<int> iDone;
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
	void Touch(void);
	void Wait(int iCount);
	void Stop(void);
};


#endif // __INTER_PROCESS_WAIT_H__

