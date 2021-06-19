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

	void*	Push(size_t uiDataSize);
	void*	Peek(size_t* puiDataSize);
	BOOL	Drop(void* pvHead);
	BOOL	Pop(void);

	void*	GetFirst(void);
	void*	GetLast(void);

protected:
	BOOL	FindOverlapping(SMemoryCacheDescriptor* pvCacheBasedNew, size_t uiNewSize);
};


#endif // !__MEMORY_QUEUE_H__

