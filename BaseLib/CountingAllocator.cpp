#include "DataMacro.h"
#include "CountingAllocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCountingAllocator::Init(CMallocator* pcAlloc)
{
	mpcAlloc = pcAlloc;
	mtUserSize = 0;
	mtSystemSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCountingAllocator::Kill(void)
{
	mtUserSize = 0;
	mtSystemSize = 0;
	mpcAlloc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCountingAllocator::AllocatedUserSize(void)
{
	return mtUserSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCountingAllocator::AllocatedSystemSize(void)
{
	return mtSystemSize;
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
	size_t						tSystemSize;
	
	tOffset = mpcAlloc->SizeOffset();
	if (tOffset == 0)
	{
		tAllocSize = tSize + sizeof(SCountingMemoryAllocation);
		tSystemSize = tAllocSize;
	}
	else
	{
		tAllocSize = tSize;
		tSystemSize = tSize + tOffset;
	}

	pv = mpcAlloc->Malloc(tAllocSize);
	if (pv)
	{
		mtUserSize += tSize;
		mtSystemSize += tSystemSize;

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
	size_t						tSystemSize;

	tOffset = mpcAlloc->SizeOffset();
	if (tOffset == 0)
	{
		tAllocSize = tSize + sizeof(SCountingMemoryAllocation);
		tSystemSize = tAllocSize;
	}
	else
	{
		tAllocSize = tSize;
		tSystemSize = tSize + tOffset;
	}

	if (pv)
	{
		if (tOffset == 0)
		{
			ps = DataGetHeader<SCountingMemoryAllocation, void>(pv);
			mtUserSize -= ps->tSize;
			mtSystemSize -= (ps->tSize + sizeof(SCountingMemoryAllocation));
		}
		else
		{
			ps = (SCountingMemoryAllocation*)RemapSinglePointer(pv, -(ptrdiff_t)tOffset);
			mtUserSize -= ps->tSize;
			mtSystemSize -= (ps->tSize + tOffset);
		}
	}

	if (tOffset == 0)
	{
		ps = (SCountingMemoryAllocation*)mpcAlloc->Realloc(DataGetHeader<SCountingMemoryAllocation, void>(pv), tAllocSize);
		if (ps)
		{
			mtUserSize += tSize;
			mtSystemSize += tSystemSize;

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
			mtUserSize += tSize;
			mtSystemSize += tSystemSize;
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
			mtUserSize -= ps->tSize;
			mtSystemSize -= (ps->tSize + sizeof(SCountingMemoryAllocation));
			mpcAlloc->Free(ps);
		}
		else
		{
			ps = (SCountingMemoryAllocation*)RemapSinglePointer(pv, -(ptrdiff_t)tOffset);
			mtUserSize -= ps->tSize;
			mtSystemSize -= (ps->tSize + tOffset);
			mpcAlloc->Free(pv);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCountingAllocator::ClassName(void)
{
	return mpcAlloc->ClassName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CCountingAllocator::ClassSize(void)
{
	return mpcAlloc->ClassSize();
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

