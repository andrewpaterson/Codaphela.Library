#include "MemoryHeader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryFreeListParams* SMemoryFreeListParams::Init(unsigned int iFreeListSize, int iPrevSize, int iChunkSize)
{
	iMaxListSize = iFreeListSize;
	iMinListSize = iPrevSize + 1;
	iMaxElementSize = iFreeListSize - sizeof(SMemoryAllocation);
	if (iPrevSize >= sizeof(SMemoryAllocation) + 1)
	{
		iMinElementSize = iPrevSize - sizeof(SMemoryAllocation) + 1;
	}
	else
	{
		iMinElementSize = 1;
	}

	this->iChunkSize = iChunkSize;
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFreeListDesc::Init(unsigned int iStride, int iAlignment, int iOffset)
{
	this->iStride = iStride;
	this->iAlignment = iAlignment;
	this->iOffset = iOffset;
	this->pcFreeList = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFreeListDesc::Init(CFreeList* pcFreeList, int iStride, int iAlignment, int iOffset)
{
	this->iStride = iStride;
	this->iAlignment = iAlignment;
	this->iOffset = iOffset;
	this->pcFreeList = pcFreeList;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareFreeListDesc(const void* arg1, const void* arg2)
{
	SFreeListDesc*	ps1;
	SFreeListDesc*	ps2;

	ps1 = (SFreeListDesc*)arg1;
	ps2 = (SFreeListDesc*)arg2;

	if ((ps1->iStride) < (ps2->iStride))
	{
		return -1;
	}
	else if ((ps1->iStride) > (ps2->iStride))
	{
		return 1;
	}

	if ((ps1->iAlignment) < (ps2->iAlignment))
	{
		return -1;
	}
	else if ((ps1->iAlignment) > (ps2->iAlignment))
	{
		return 1;
	}

	if ((ps1->iOffset) < (ps2->iOffset))
	{
		return 1;
	}
	else if ((ps1->iOffset) > (ps2->iOffset))
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareFreeListParam(const void* arg1, const void* arg2)
{
	unsigned int		uiElementSize;
	SMemoryFreeListParams*	psParams;

	uiElementSize = *((unsigned int*)arg1);
	psParams = (SMemoryFreeListParams*)arg2;

	if (uiElementSize < psParams->iMinElementSize)
	{
		return -1;
	}
	else if (uiElementSize > psParams->iMaxElementSize)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SMemory::Init(void)
{
	bValid = FALSE;;
	pvMem = NULL;
	uiAllocCount = 0;
	uiSize = 0;

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SMemory::Set(SMemoryAllocation* psAllocation)
{
	bValid = TRUE;
	pvMem = RemapSinglePointer(psAllocation, sizeof(SMemoryAllocation));
	uiAllocCount = psAllocation->uiAllocCount;
	uiSize = psAllocation->uiSize;
}

