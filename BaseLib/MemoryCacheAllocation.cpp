#include "Chars.h"
#include "MemoryCacheAllocation.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCacheAllocation::Init(int uiSize)
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
SMemoryCacheDescriptor* CMemoryCacheAllocation::Get(int iIndex)
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
void CMemoryCacheAllocation::Dump(int iDescriptorSize)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	CChars						sz;
	char*						pvData;
	size						iLen;
	int							i;

	sz.Init();

	sz.Append("Evicted Data (");
	sz.Append(NumElements());
	sz.Append(")\n---------------\n");

	for (i = 0; i < NumElements(); i++)
	{
		psCacheDesc = Get(i);
		pvData = (char*)RemapSinglePointer(psCacheDesc, iDescriptorSize);
		iLen = psCacheDesc->uiSize;

		sz.Append("(Ln:");
		sz.AppendHexHiLo(&iLen, 4);
		sz.Append(" Da:");
		sz.AppendHexHiLo(&psCacheDesc, 4);
		sz.Append(" Nx:");
		sz.AppendHexHiLo(&psCacheDesc->psNext, 4);
		sz.Append(") ");

		sz.AppendData(pvData, iLen, 80);
		sz.AppendNewLine();
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

