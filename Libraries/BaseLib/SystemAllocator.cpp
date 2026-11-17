#include <stdlib.h>
#include "Logger.h"
#include "Validation.h"
#include "ThreadMallocator.h"
#include "SystemAllocator.h"


//Where the system allocator's memory comes from.  On the hosts, CThreadMallocator.  On the 65C64 and the W65C816,
//Libc65's malloc (a CTrueMallocator).  And malloc when AddressSanitizer is watching: it and LeakSanitizer see
//malloc's blocks and none of CThreadMallocator's, and seeing them is what an ASan build is for.  Defining
//SYSTEM_ALLOCATOR_USES_MALLOC asks for malloc too (to run under valgrind, say).
#if defined(__LIBC65__) || defined(__SANITIZE_ADDRESS__)
#define SYSTEM_ALLOCATOR_USES_MALLOC
#endif // __LIBC65__ || __SANITIZE_ADDRESS__

#ifdef SYSTEM_ALLOCATOR_USES_MALLOC
#define SYSTEM_ALLOCATOR_MALLOC(uiSize)			malloc(uiSize)
#define SYSTEM_ALLOCATOR_REALLOC(pv, uiSize)	realloc(pv, uiSize)
#define SYSTEM_ALLOCATOR_FREE(pv)				free(pv)
#else
#define SYSTEM_ALLOCATOR_MALLOC(uiSize)			gcThreadMallocator.Malloc(uiSize)
#define SYSTEM_ALLOCATOR_REALLOC(pv, uiSize)	gcThreadMallocator.Realloc(pv, uiSize)
#define SYSTEM_ALLOCATOR_FREE(pv)				gcThreadMallocator.Free(pv)
#endif // SYSTEM_ALLOCATOR_USES_MALLOC


CSystemAllocator	gcSystemAllocator;


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSystemAllocator::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CSystemAllocator::Malloc(lsize uiSize)
{
#ifdef _DEBUG
	if (uiSize > guiMaxDebugAllocatorSize)
	{
		BREAK();
	}
#endif // _DEBUG

	return SYSTEM_ALLOCATOR_MALLOC(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSystemAllocator::Free(void* pv)
{
	if (pv)
	{
		SYSTEM_ALLOCATOR_FREE(pv);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CSystemAllocator::Realloc(void* pv, lsize uiSize)
{
#ifdef _DEBUG
	if (uiSize > guiMaxDebugAllocatorSize)
	{
		BREAK();
	}
#endif // _DEBUG

	return SYSTEM_ALLOCATOR_REALLOC(pv, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSystemAllocator::IsLocal(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSystemAllocator::SizeOffset(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSystemAllocator::ShortName(void)
{
	return "Sys";
}


