#include "Chars.h"
#include "MemoryCacheAllocation.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCacheAllocation::Init(uint32 uiSize)
{
	mapEvictedCacheDescriptors.Init();
	muiSize = uiSize;
	
	mpsDescriptor = NULL;
	miCachedSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCacheAllocation::Kill(void)
{
	mapEvictedCacheDescriptors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryCacheAllocation::HasOverlaps(void)
{
	return mapEvictedCacheDescriptors.IsNotEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CMemoryCacheAllocation::Get(size iIndex)
{
	return (SMemoryCacheDescriptor*)mapEvictedCacheDescriptors.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMemoryCacheAllocation::NumElements(void)
{
	return mapEvictedCacheDescriptors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVoidPtr* CMemoryCacheAllocation::GetEvictedArray(void)
{
	return &mapEvictedCacheDescriptors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCacheAllocation::Dump(size iDescriptorSize)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	CChars						sz;
	char*						pvData;
	size						iLen;
	size						i;
	size						iNumElements;
	SMemoryCacheDescriptor*		psNext;
	SMemoryCacheDescriptor*		psPrev;

	sz.Init();

	sz.Append("Evicted Data (");
	sz.Append(NumElements());
	sz.Append(")\n---------------\n");

	iNumElements = NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psCacheDesc = Get(i);
		pvData = (char*)RemapSinglePointer(psCacheDesc, iDescriptorSize);
		iLen = psCacheDesc->GetSize();
		psNext = psCacheDesc->GetNext();
		psPrev = psCacheDesc->GetPrev();

		sz.Append("(Ln:");
		sz.AppendHexHiLo(&iLen, 4);
		sz.Append(" Da:");
		sz.AppendHexHiLo(&psCacheDesc, 4);
		sz.Append(" Nx:");
		sz.AppendHexHiLo(&psNext, 4);
		sz.Append(" Pv:");
		sz.AppendHexHiLo(&psPrev, 4);
		sz.Append(") ");

		sz.AppendData(pvData, iLen, 80);
		sz.AppendNewLine();
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

