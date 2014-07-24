#ifndef __MALLOCATOR_H__
#define __MALLOCATOR_H__
#include "Define.h"


class CFileReader;
class CFileWriter;
class CMallocator
{
public:
	virtual void*	Malloc(size_t tSize) =0;
	virtual void*	Realloc(void* pv, size_t tSize) =0;
	virtual void	Free(void* pv) =0;

	virtual char*	GetName(void) =0;

	virtual BOOL	IsLocal(void) =0;
};


#endif // __MALLOCATOR_H__

