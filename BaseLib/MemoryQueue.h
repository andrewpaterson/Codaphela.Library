#ifndef __MEMORY_QUEUE_H__
#define __MEMORY_QUEUE_H__
#include "CircularMemoryList.h"
#include "MemoryCache.h"
#include "LinkedList.h"


class CMemoryQueue : public CCircularMemoryList
{
public:
	void	Init(void* pvMemory, size uiByteSize);
	void	Kill(void);

	void*	Push(size uiDataSize);
	void*	Peek(size* puiDataSize);
	bool	Drop(void* pvHead);
	bool	Pop(void);

	void*	GetFirst(void);
	void*	GetLast(void);

protected:
	bool	FindOverlapping(SMemoryCacheDescriptor* pvCacheBasedNew, size uiNewSize);
};


#endif // __MEMORY_QUEUE_H__

