#include "Define.h"
#include "NullAllocator.h"


CNullAllocator	gcNullAllocator;


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CNullAllocator::Malloc(size_t tSize)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CNullAllocator::Free(void* pv)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CNullAllocator::Realloc(void* pv, size_t tSize)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CNullAllocator::GetName(void)
{
	return "CNullAllocator";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNullAllocator::IsLocal(void)
{
	return FALSE;
}

