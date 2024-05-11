#include "SystemAllocator.h"
#include "MemoryBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Init(void)
{
	Init(&gcSystemAllocator);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Init(CMallocator* pcMalloc)
{
	CMalloc::Init(pcMalloc);
	mpvMem = NULL;
	mtSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Kill(void)
{
	Free(mpvMem);
	mpvMem = NULL;
	mtSize = 0;
	CMalloc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryBlock::GetMemory(void)
{
	return mpvMem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryBlock::Allocate(size uiSize)
{
	if (mpvMem == NULL)
	{
		mpvMem = Malloc(uiSize);
	}
	else
	{
		mpvMem = Realloc(mpvMem, uiSize);
	}

	if (mpvMem != NULL)
	{
		mtSize = uiSize;
	}
	else
	{
		mtSize = 0;
	}

	return mpvMem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryBlock::GetSize(void)
{
	return mtSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryBlock::Malloc(size uiSize)
{
	return mpcMalloc->Malloc(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Free(void* pv)
{
	mpcMalloc->Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryBlock::Realloc(void* pv, size uiSize)
{
	pv = mpcMalloc->Realloc(pv, uiSize);
	return pv;
}

