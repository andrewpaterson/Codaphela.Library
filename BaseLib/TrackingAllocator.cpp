#include "TrackingAllocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrackingAllocator::Init(CMallocator* pcAlloc)
{
	mpcAlloc = pcAlloc;
	mmpiSizes.Init(1024);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrackingAllocator::Kill(void)
{
	mmpiSizes.Kill();
	mpcAlloc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CTrackingAllocator::AllocatedSize(void)
{
	size_t			tTotal;
	SMapIterator	sIter;
	BOOL			bResult;
	size_t*			ptCurrent;

	tTotal = 0;
	bResult = mmpiSizes.StartIteration(&sIter, NULL, (void**)&ptCurrent);
	while (bResult)
	{
		tTotal += (*ptCurrent);
		bResult = mmpiSizes.Iterate(&sIter, NULL, (void**)&ptCurrent);
	}
	return tTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CTrackingAllocator::Malloc(size_t tSize)
{
	void*	pv;

	pv = mpcAlloc->Malloc(tSize);
	mmpiSizes.Put(pv, tSize);

	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CTrackingAllocator::Realloc(void* pv, size_t tSize)
{
	void*	pvNew;

	pvNew = mpcAlloc->Realloc(pv, tSize);
	if (pvNew != pv)
	{
		mmpiSizes.Remove(pv);
	}
	mmpiSizes.Put(pvNew, tSize);

	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrackingAllocator::Free(void* pv)
{
	mpcAlloc->Free(pv);
	mmpiSizes.Remove(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CTrackingAllocator::GetName(void)
{
	return mpcAlloc->GetName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTrackingAllocator::IsLocal(void)
{
	return mpcAlloc->IsLocal();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CTrackingAllocator::SizeOffset(void)
{
	return 0;
}

