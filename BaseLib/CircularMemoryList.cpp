#include "PointerFunctions.h"
#include "PointerRemapper.h"
#include "Chars.h"
#include "CircularMemoryList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Init(void* pvCache, size_t uiCacheSize, int iDescriptorSize)
{
	miDescriptorSize = iDescriptorSize;
	muiCacheSize = uiCacheSize;
	mpvCache = pvCache;

	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Kill(void)
{
	mpvCache = NULL;
	mpsTail = NULL;
	mpsHead = NULL;
	muiCacheSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Remap(void* pvNewCache, size_t uiCacheSize)
{
	ptrdiff_t					uiAbsDiff;
	SMemoryCacheDescriptor*		psDescriptor;
	SMemoryCacheDescriptor*		psNext;


	if (mpsHead)
	{
		if (pvNewCache > mpvCache)
		{
			uiAbsDiff = (size_t)pvNewCache - (size_t)mpvCache;
			psDescriptor = mpsHead;
			do
			{
				psNext = psDescriptor->psNext;

				psDescriptor->psNext = (SMemoryCacheDescriptor*)RemapSinglePointer(psDescriptor->psNext, uiAbsDiff);
				psDescriptor->psPrev = (SMemoryCacheDescriptor*)RemapSinglePointer(psDescriptor->psPrev, uiAbsDiff);

				psDescriptor = psNext;
			} while (psDescriptor != mpsHead);

			mpsHead = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsHead, uiAbsDiff);
			mpsTail = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsTail, uiAbsDiff);
		}
		else
		{
			uiAbsDiff = (size_t)mpvCache - (size_t)pvNewCache;
			psDescriptor = mpsHead;
			do
			{
				psNext = psDescriptor->psNext;

				psDescriptor->psNext = (SMemoryCacheDescriptor*)RemapSinglePointer(psDescriptor->psNext, -uiAbsDiff);
				psDescriptor->psPrev = (SMemoryCacheDescriptor*)RemapSinglePointer(psDescriptor->psPrev, -uiAbsDiff);

				psDescriptor = psNext;
			} while (psDescriptor != mpsHead);

			mpsHead = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsHead, -uiAbsDiff);
			mpsTail = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsTail, -uiAbsDiff);
		}
	}

	if (uiCacheSize > muiCacheSize)
	{
		memcpy(pvNewCache, mpvCache, muiCacheSize);
		memset(RemapSinglePointer(pvNewCache, muiCacheSize), 0, uiCacheSize - muiCacheSize);
	}
	else
	{
		memcpy(pvNewCache, mpvCache, uiCacheSize);
	}

	mpvCache = pvNewCache;
	muiCacheSize = uiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Clear(void)
{
	mpsTail = NULL;
	mpsHead = NULL;
	memset(mpvCache, 0, muiCacheSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Deallocate(void* pvData)
{
	SMemoryCacheDescriptor* psDescriptor;

	if (pvData)
	{
		psDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(pvData, -miDescriptorSize);
		Deallocate(psDescriptor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Deallocate(SMemoryCacheDescriptor* psDescriptor)
{
	if ((psDescriptor == mpsHead) && (psDescriptor == mpsTail))
	{
		mpsHead = NULL;
		mpsTail = NULL;
		return;
	}

	if (psDescriptor != mpsHead)
	{
		psDescriptor->psPrev->psNext = psDescriptor->psNext;
	}
	else
	{
		psDescriptor->psPrev->psNext = psDescriptor->psNext;
		mpsHead = psDescriptor->psNext;
	}

	if (psDescriptor != mpsTail)
	{
		psDescriptor->psNext->psPrev = psDescriptor->psPrev;
	}
	else
	{
		psDescriptor->psNext->psPrev = psDescriptor->psPrev;
		mpsTail = psDescriptor->psPrev;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCircularMemoryList::GetCacheSize(void)
{
	return muiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCircularMemoryList::GetAllocatedSize(void)
{
	SMemoryCacheDescriptor* psIter;
	size_t					tSize;

	tSize = 0;
	psIter = StartIteration();
	while (psIter)
	{
		tSize += psIter->uiSize + miDescriptorSize;
		psIter = Iterate(psIter);
	}
	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetFirst(void)
{
	return mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetNext(SMemoryCacheDescriptor* psCurrent)
{
	if (!psCurrent)
	{
		return NULL;
	}

	psCurrent = psCurrent->psNext;

	return psCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetPrev(SMemoryCacheDescriptor* psCurrent)
{
	if (!psCurrent)
	{
		return NULL;
	}

	psCurrent = psCurrent->psPrev;

	return psCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetLast(void)
{
	return mpsTail;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircularMemoryList::NumElements(void)
{
	SMemoryCacheDescriptor*		psDescriptor;
	int							iNum;

	if (IsEmpty())
	{
		return 0;
	}

	iNum = 0;
	psDescriptor = StartIteration();
	while (psDescriptor)
	{
		iNum++;
		psDescriptor = Iterate(psDescriptor);
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircularMemoryList::GetDescriptorSize(void)
{
	return miDescriptorSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CCircularMemoryList::GetData(SMemoryCacheDescriptor* psDescriptor)
{
	return RemapSinglePointer(psDescriptor, miDescriptorSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetDescriptor(void* pvData)
{
	return (SMemoryCacheDescriptor*)RemapSinglePointer(pvData, -miDescriptorSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircularMemoryList::NumElements(int iSize)
{
	SMemoryCacheDescriptor* psDescriptor;
	int							iNum;

	if (IsEmpty())
	{
		return 0;
	}

	iNum = 0;
	psDescriptor = StartIteration();
	while (psDescriptor)
	{
		if (psDescriptor->uiSize == iSize)
		{
			iNum++;
		}
		psDescriptor = Iterate(psDescriptor);
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCircularMemoryList::IsEmpty(void)
{
	return mpsTail == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::StartIteration(void)
{
	return mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::Iterate(SMemoryCacheDescriptor* psCurrent)
{
	if (!psCurrent)
	{
		return NULL;
	}

	psCurrent = psCurrent->psNext;
	if (psCurrent != mpsHead)
	{
		return psCurrent;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Dump(void)
{
	SMemoryCacheDescriptor* psDescriptor;
	CChars						sz;
	char* pvData;
	int							iLen;

	sz.Init();

	sz.Append("Data Cache (");
	sz.Append(NumElements());
	sz.Append(")\n---------------\n");

	psDescriptor = StartIteration();
	while (psDescriptor)
	{
		pvData = (char*)RemapSinglePointer(psDescriptor, miDescriptorSize);
		iLen = psDescriptor->uiSize;

		sz.Append("(Ln:");
		sz.AppendHexHiLo(&iLen, 4);
		sz.Append(" Da:");
		sz.AppendHexHiLo(&psDescriptor, 4);
		sz.Append(" Nx:");
		sz.AppendHexHiLo(&psDescriptor->psNext, 4);
		sz.Append(") ");

		sz.AppendData(pvData, iLen, 80);
		sz.AppendNewLine();
		psDescriptor = Iterate(psDescriptor);
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

