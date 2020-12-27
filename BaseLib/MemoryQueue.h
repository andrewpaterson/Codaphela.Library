#ifndef __MEMORY_QUEUE_H__
#define __MEMORY_QUEUE_H__
#include "CircularMemoryList.h"
#include "MemoryCache.h"
#include "LinkedList.h"


class CMemoryQueue : public CCircularMemoryList
{
public:
	void	Init(void* pvMemory, size_t uiByteSize);
	void	Kill(void);

	void*	InsertAfterTail(size_t uiDataSize);
	void*	GetHead(void);
	void	RemoveHead(void);

protected:
	BOOL	FindOverlapping(void* pvNew, size_t uiNewSize);
};


#endif // __MEMORY_QUEUE_H__

