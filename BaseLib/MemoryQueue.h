#ifndef __MEMORY_QUEUE_H__
#define __MEMORY_QUEUE_H__
#include "CircularMemoryList.h"
#include "MemoryCache.h"
#include "LinkedList.h"


class CMemoryQueue : public CCircularMemoryList
{
public:
	void	Init(void* pvMemory, size_t iByteSize);
	void	Kill(void);

	void*	InsertAfterTail(int iDataSize);
	void*	GetHead(void);
	void	RemoveHead(void);
};


#endif // __MEMORY_QUEUE_H__

