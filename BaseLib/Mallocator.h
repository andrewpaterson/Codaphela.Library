#ifndef __MALLOCATOR_H__
#define __MALLOCATOR_H__


class CMallocator
{
public:
	virtual void*	Malloc(size_t tSize) =0;
	virtual void*	Realloc(void* pv, size_t iMemSize) =0;
	virtual void	Free(void* pv) =0;
};


#endif // __MALLOCATOR_H__

