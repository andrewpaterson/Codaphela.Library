#include "MemoryHeader.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFreeListDesc::Init(unsigned int iStride)
{
	this->iStride = iStride;
	this->pcFreeList = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFreeListDesc::Init(CFreeList* pcFreeList, int iStride)
{
	this->iStride = iStride;
	this->pcFreeList = pcFreeList;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SAlignedFreeListDesc::Init(unsigned int iStride, int iAlignment, int iOffset)
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
void SAlignedFreeListDesc::Init(CFreeList* pcFreeList, int iStride, int iAlignment, int iOffset)
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
int CompareAlignedFreeListDesc(const void* arg1, const void* arg2)
{
	SAlignedFreeListDesc*	ps1;
	SAlignedFreeListDesc*	ps2;

	ps1 = (SAlignedFreeListDesc*)arg1;
	ps2 = (SAlignedFreeListDesc*)arg2;

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

	return 0;
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
void SMemory::Set(SGeneralMemoryAllocation* psAllocation)
{
	bValid = TRUE;
	pvMem = RemapSinglePointer(psAllocation, sizeof(SGeneralMemoryAllocation));
	uiAllocCount = psAllocation->uiAllocCount;
	uiSize = psAllocation->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SMemory::Set(SDataMemoryAllocation* psAllocation)
{
	bValid = TRUE;
	pvMem = RemapSinglePointer(psAllocation, sizeof(SDataMemoryAllocation));
	uiAllocCount = 0;
	uiSize = psAllocation->uiSize;
}

