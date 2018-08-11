#include "Chars.h"
#include "MemoryCacheAllocation.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCacheAllocation::Init(int iDataSize, int iDescriptorSize)
{
	mapEvictedCacheDescriptors.Init();
	miDataSize = iDataSize;
	miDescriptorSize = iDescriptorSize;
	
	mpsCacheDesc = NULL;
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
BOOL CMemoryCacheAllocation::HasOverlaps(void)
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
void CMemoryCacheAllocation::Dump(void)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	CChars						sz;
	char*						pvData;
	int							iLen;
	int							i;

	sz.Init();

	sz.Append("Evicted Data (");
	sz.Append(NumElements());
	sz.Append(")\n---------------\n");

	for (i = 0; i < NumElements(); i++)
	{
		psCacheDesc = Get(i);
		pvData = (char*)RemapSinglePointer(psCacheDesc, miDescriptorSize);
		iLen = psCacheDesc->iDataSize;

		sz.Append("(Ln:");
		sz.AppendHexHiLo(&iLen, 4);
		sz.Append(" Da:");
		sz.AppendHexHiLo(&psCacheDesc, 4);
		sz.Append(" Nx:");
		sz.AppendHexHiLo(&psCacheDesc->psNext, 4);
		sz.Append(" Fl:");
		sz.AppendHexHiLo(&psCacheDesc->iFlags, 4);
		sz.Append(") ");

		sz.AppendData(pvData, iLen, 80);
		sz.AppendNewLine();
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

