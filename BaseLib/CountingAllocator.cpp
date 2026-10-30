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
lsize CCountingAllocator::AllocatedUserSize(void)
{
	return mtUserSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CCountingAllocator::AllocatedSystemSize(void)
{
	return mtSystemSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CCountingAllocator::Malloc(lsize uiSize)
{
	SCountingMemoryAllocation*	ps;
	void*						pv;
	lsize						tOffset;
	lsize						tAllocSize;
	lsize						tSystemSize;
	
	tOffset = mpcAlloc->SizeOffset();
	if (tOffset == 0)
	{
		tAllocSize = uiSize + sizeof(SCountingMemoryAllocation);
		tSystemSize = tAllocSize;
	}
	else
	{
		tAllocSize = uiSize;
		tSystemSize = uiSize + tOffset;
	}

	pv = mpcAlloc->Malloc(tAllocSize);
	if (pv)
	{
		mtUserSize += uiSize;
		mtSystemSize += tSystemSize;

		if (tOffset == 0)
		{
			ps = (SCountingMemoryAllocation*)pv;
			ps->uiSize = uiSize;
			return HeaderGetData<SCountingMemoryAllocation, void>(ps);
		}
	}

	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CCountingAllocator::Realloc(void* pv, lsize uiSize)
{
	SCountingMemoryAllocation*	ps;
	lsize						tOffset;
	lsize						tAllocSize;
	lsize						tSystemSize;

	tOffset = mpcAlloc->SizeOffset();
	if (tOffset == 0)
	{
		tAllocSize = uiSize + sizeof(SCountingMemoryAllocation);
		tSystemSize = tAllocSize;
	}
	else
	{
		tAllocSize = uiSize;
		tSystemSize = uiSize + tOffset;
	}

	if (pv)
	{
		if (tOffset == 0)
		{
			ps = DataGetHeader<SCountingMemoryAllocation, void>(pv);
			mtUserSize -= ps->uiSize;
			mtSystemSize -= (ps->uiSize + sizeof(SCountingMemoryAllocation));
		}
		else
		{
			ps = (SCountingMemoryAllocation*)RemapSinglePointer(pv, -(ptrdiff_t)tOffset);
			mtUserSize -= ps->uiSize;
			mtSystemSize -= (ps->uiSize + tOffset);
		}
	}

	if (tOffset == 0)
	{
		ps = (SCountingMemoryAllocation*)mpcAlloc->Realloc(DataGetHeader<SCountingMemoryAllocation, void>(pv), tAllocSize);
		if (ps)
		{
			mtUserSize += uiSize;
			mtSystemSize += tSystemSize;

			ps->uiSize = uiSize;
			return HeaderGetData<SCountingMemoryAllocation, void>(ps);
		}
		return ps;
	}
	else
	{
		pv = mpcAlloc->Realloc(pv, tAllocSize);
		if (pv)
		{
			mtUserSize += uiSize;
			mtSystemSize += tSystemSize;
		}
		return pv;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCountingAllocator::Free(void* pv)
{
	SCountingMemoryAllocation*	ps;
	lsize						tOffset;

	tOffset = mpcAlloc->SizeOffset();
	if (pv)
	{
		if (tOffset == 0)
		{
			ps = DataGetHeader<SCountingMemoryAllocation, void>(pv);
			mtUserSize -= ps->uiSize;
			mtSystemSize -= (ps->uiSize + sizeof(SCountingMemoryAllocation));
			return mpcAlloc->Free(ps);
		}
		else
		{
			ps = (SCountingMemoryAllocation*)RemapSinglePointer(pv, -(ptrdiff_t)tOffset);
			mtUserSize -= ps->uiSize;
			mtSystemSize -= (ps->uiSize + tOffset);
			return mpcAlloc->Free(pv);
		}
	}
	return false;
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
size CCountingAllocator::ClassSize(void)
{
	return mpcAlloc->ClassSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCountingAllocator::IsLocal(void)
{
	return mpcAlloc->IsLocal();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCountingAllocator::SizeOffset(void)
{
	size	tOffset;

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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCountingAllocator::ShortName(void)
{
	return ClassName();
}

