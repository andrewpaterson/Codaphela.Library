#ifndef __SHARED_MEMORY_QUEUE_H__
#define __SHARED_MEMORY_QUEUE_H__
#include "BaseLib/MemoryQueue.h"
#include "ResizableSharedMemory.h"
#include "InterProcessMutex.h"


class CSharedMemoryQueue
{
private:
	CResizableSharedMemory	mcMemory;
	CMemoryQueue			mcQueue;
	CInterProcessMutex		mcMutex;
	BOOL					mbMaster;

public:
	BOOL Init(BOOL bMaster, char* szName, size_t uiByteSize);
	void Kill(void);
};


#endif // __SHARED_MEMORY_QUEUE_H__

