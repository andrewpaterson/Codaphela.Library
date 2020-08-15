#ifndef __MALLOCATOR_H__
#define __MALLOCATOR_H__
#include "Define.h"
#include "Constructable.h"
#include "Killable.h"

//Global Mallocators are created once on executable start.  Their will only be one of any type of global Mallocator.
//
//Local Mallocators are created as needed and are usually backed by a data structure like a free list.
//They can be read and written.  This does not read or write their allocated data but only the parameters with which they allocated their data.


class CFileReader;
class CFileWriter;
class CMallocator : public CConstructable, public CKillable
{
public:
	virtual void*		Malloc(size_t tSize) =0;
	virtual void*		Realloc(void* pv, size_t tSize) =0;
	virtual void		Free(void* pv) =0;

	virtual BOOL		IsLocal(void) =0;

	virtual size_t		SizeOffset(void) =0;
};


#endif // __MALLOCATOR_H__

