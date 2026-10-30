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
template<class M>
class CArrayTemplatePtr;
typedef CArrayTemplatePtr<void>	CArrayVoidPtr;
class CMallocator : public CConstructable, public CKillable
{
public:
	virtual void*		Malloc(lsize uiSize) =0;
	virtual void*		Malloc(lsize uiSize, char(** pacDebugName)[4]);
	virtual void*		Realloc(void* pv, lsize uiSize) =0;
	virtual bool		Free(void* pv) =0;
	virtual size		FreeMultiple(CArrayVoidPtr* pav);

	virtual bool		IsLocal(void) =0;

	virtual size		SizeOffset(void) =0;

	virtual const char*	ShortName(void) =0;
};


extern lsize guiMaxDebugAllocatorSize;


void InitAllocatorDebug(lsize uiMaxDebugAllocatorSize);
void KillAllocatorDebug(void);

//Read and write a Mallocator through gcMallocators without including GlobalMemory.h (which would be a circular include from the template headers).
CMallocator*	ReadMallocator(CFileReader* pcFileReader);
bool			WriteMallocator(CFileWriter* pcFileWriter, CMallocator* pcMalloc);


#endif // __MALLOCATOR_H__

