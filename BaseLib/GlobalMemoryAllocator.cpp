#include "GlobalMemoryAllocator.h"


CGeneralMemory			gcMemory;
CGlobalMemoryAllocator	gcMemoryAllocator;


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CGlobalMemoryAllocator::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGlobalMemoryAllocator::Malloc(size_t tSize)
{
	return gcMemory.Add((unsigned int)tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGlobalMemoryAllocator::Realloc(void* pv, size_t tSize)
{
	void*	pvNew;

	pvNew = gcMemory.Grow(pv, (unsigned int)tSize);
	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGlobalMemoryAllocator::Free(void* pv)
{
	return gcMemory.Remove(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGlobalMemoryAllocator::IsLocal(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CGlobalMemoryAllocator::SizeOffset(void)
{
	return sizeof(SGeneralMemoryAllocation);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CGlobalMemoryAllocator::ShortName(void)
{
	return "GMA";
}

