#ifndef __SHARED_MEMORY_QUEUE_H__
#define __SHARED_MEMORY_QUEUE_H__
#include "BaseLib/MemoryQueue.h"
#include "ResizableSharedMemory.h"
#include "InterProcessMutex.h"


class CSharedMemoryQueue
{
private:
	CResizableSharedMemory	mcSharedMemory;
	CMemoryQueue			mcQueue;
	CInterProcessMutex		mcMutex;
	bool					mbMaster;

public:
	bool	Init(char* szName, size uiByteSize);
	bool	Init(char* szName);
	void	Kill(void);

	bool	Push(void* pvData, size uiDataSize);
	bool	Pop(void* pvData, size* puiDataSize, size uiMaxDataSize);

	//The size of the next element, or 0 if the queue is empty.
	size	PeekSize(void);
	//Without the mutex: false if the queue was empty a moment ago, true if it
	//may have elements and must be looked at with the mutex.  Cheap enough to
	//poll; another process may push just after it returns false.
	bool	MayHaveElements(void);
	bool	IsEmpty(void);
	int		NumElements(void);
	size	GetCacheSize(void);
	size	GetAllocatedSize(void);

	bool	ValidateQueue(void);

protected:
	bool	InitMaster(char* szName, size uiByteSize);
	bool	InitClient(char* szName);
};


#endif // __SHARED_MEMORY_QUEUE_H__

