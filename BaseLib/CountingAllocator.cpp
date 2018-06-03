#include "DataMacro.h"
#include "CountingAllocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCountingAllocator::Init(CMallocator* pcAlloc)
{
	mpcAlloc = pcAlloc;
	mtSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCountingAllocator::Kill(void)
{
	mtSize = 0;
	mpcAlloc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCountingAllocator::AllocatedSize(void)
{
	return mtSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CCountingAllocator::Malloc(size_t tSize)
{
	SCountingMemoryAllocation*	ps;
	void*						pv;
	size_t						tOffset;
	size_t						tAllocSize;
	
	tOffset = mpcAlloc->SizeOffset();
	if (tOffset == 0)
	{
		tAllocSize = tSize + sizeof(SCountingMemoryAllocation);
	}
	else
	{
		tAllocSize = tSize;
	}

	pv = mpcAlloc->Malloc(tAllocSize);
	if (pv)
	{
		mtSize += tSize;

		if (tOffset == 0)
		{
			ps = (SCountingMemoryAllocation*)pv;
			ps->tSize = tSize;
			return HeaderGetData<SCountingMemoryAllocation, void>(ps);
		}
	}

	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CCountingAllocator::Realloc(void* pv, size_t tSize)
{
	SCountingMemoryAllocation*	ps;
	size_t						tOffset;
	size_t						tAllocSize;

	tOffset = mpcAlloc->SizeOffset();
	if (tOffset == 0)
	{
		tAllocSize = tSize + sizeof(SCountingMemoryAllocation);
	}
	else
	{
		tAllocSize = tSize;
	}

	if (pv)
	{
		if (tOffset == 0)
		{
			ps = DataGetHeader<SCountingMemoryAllocation, void>(pv);
		}
		else
		{
			ps = (SCountingMemoryAllocation*)RemapSinglePointer(pv, -(int)(tOffset));
		}
		mtSize -= ps->tSize;
	}

	if (tOffset == 0)
	{
		ps = (SCountingMemoryAllocation*)mpcAlloc->Realloc(DataGetHeader<SCountingMemoryAllocation, void>(pv), tAllocSize);
		if (ps)
		{
			mtSize += tSize;
			ps->tSize = tSize;
			return HeaderGetData<SCountingMemoryAllocation, void>(ps);
		}
		return ps;
	}
	else
	{
		pv = mpcAlloc->Realloc(pv, tAllocSize);
		if (pv)
		{
			mtSize += tSize;
		}
		return pv;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCountingAllocator::Free(void* pv)
{
	SCountingMemoryAllocation*	ps;
	size_t						tOffset;

	tOffset = mpcAlloc->SizeOffset();
	if (pv)
	{
		if (tOffset == 0)
		{
			ps = DataGetHeader<SCountingMemoryAllocation, void>(pv);
			mtSize -= ps->tSize;
			mpcAlloc->Free(ps);

		}
		else
		{
			ps = (SCountingMemoryAllocation*)RemapSinglePointer(pv, -(int)(tOffset));
			mtSize -= ps->tSize;
			mpcAlloc->Free(pv);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCountingAllocator::GetName(void)
{
	return mpcAlloc->GetName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCountingAllocator::IsLocal(void)
{
	return mpcAlloc->IsLocal();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCountingAllocator::SizeOffset(void)
{
	size_t	tOffset;

	tOffset = mpcAlloc->SizeOffset();
	if (tOffset == 0)
	{
		return sizeof(SCountingMemoryAllocation);
	}
	else
	{
		return tOffset;
	}
}

