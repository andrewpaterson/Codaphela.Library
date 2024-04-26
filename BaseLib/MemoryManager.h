#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__


class CMemoryManager
{
protected:
	void* mpvHeapStart;
	void* mpvHeapEnd;

public:
	void Init(void* pvHeapStart, void* pvHeapEnd);
	void Kill(void);
};


#endif // __MEMORY_MANAGER_H__

