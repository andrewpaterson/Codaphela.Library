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
void* CGlobalMemoryAllocator::Malloc(size uiSize)
{
	return gcMemory.Add((uint32)uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGlobalMemoryAllocator::Realloc(void* pv, size uiSize)
{
	void*	pvNew;

	pvNew = gcMemory.Grow(pv, (uint32)uiSize);
	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGlobalMemoryAllocator::Free(void* pv)
{
	return gcMemory.Remove(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGlobalMemoryAllocator::IsLocal(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGlobalMemoryAllocator::SizeOffset(void)
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

