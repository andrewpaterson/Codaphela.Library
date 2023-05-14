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
	bool	Init(char* szName, size_t uiByteSize);
	bool	Init(char* szName);
	void	Kill(void);

	bool	Push(void* pvData, size_t uiDataSize);
	bool	Pop(void* pvData, size_t* puiDataSize, size_t uiMaxDataSize);

	bool	IsEmpty(void);
	int		NumElements(void);
	size_t	GetCacheSize(void);
	size_t	GetAllocatedSize(void);

	bool	ValidateQueue(void);

protected:
	bool	InitMaster(char* szName, size_t uiByteSize);
	bool	InitClient(char* szName);
};


#endif // !__SHARED_MEMORY_QUEUE_H__

