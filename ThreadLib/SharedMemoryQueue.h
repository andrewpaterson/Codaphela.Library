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
	BOOL					mbMaster;

public:
	BOOL	Init(char* szName, size_t uiByteSize);
	BOOL	Init(char* szName);
	void	Kill(void);

	BOOL	Push(void* pvData, size_t uiDataSize);
	BOOL	Pop(void* pvData, size_t* puiDataSize, size_t uiMaxDataSize);

	BOOL	IsEmpty(void);
	size_t	GetCacheSize(void);
	size_t	GetAllocatedSize(void);

	BOOL	ValidateQueue(void);

protected:
	BOOL	InitMaster(size_t uiByteSize);
	BOOL	InitClient(void);
	void	InitNames(char* szName);
};


#endif // __SHARED_MEMORY_QUEUE_H__

