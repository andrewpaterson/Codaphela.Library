#include <stdlib.h>
#include "BaseLib/FreeList.h"


//Only what CRedBlackTree uses.  Each element is malloced after a header that links
//it into a list (mpsNotFullHead is the head) so Kill can free them all.
struct SFreeListStubHeader
{
	SFreeListStubHeader*	psPrev;
	SFreeListStubHeader*	psNext;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(size uiElementSize, size uiAlignment)
{
	muiElementSize = (uint16)uiElementSize;
	muiAlignment = (uint8)uiAlignment;
	muiNumElements = 0;
	mpsNotFullHead = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Kill(void)
{
	SFreeListStubHeader*	psHeader;
	SFreeListStubHeader*	psNext;

	psHeader = (SFreeListStubHeader*)mpsNotFullHead;
	while (psHeader)
	{
		psNext = psHeader->psNext;
		free(psHeader);
		psHeader = psNext;
	}
	mpsNotFullHead = NULL;
	muiNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::Add(SFNode** ppsNode)
{
	SFreeListStubHeader*	psHeader;

	psHeader = (SFreeListStubHeader*)malloc(sizeof(SFreeListStubHeader) + muiElementSize);
	psHeader->psPrev = NULL;
	psHeader->psNext = (SFreeListStubHeader*)mpsNotFullHead;
	if (psHeader->psNext)
	{
		psHeader->psNext->psPrev = psHeader;
	}
	mpsNotFullHead = (SFNode*)psHeader;
	muiNumElements++;
	return &psHeader[1];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::Remove(void* pvData)
{
	SFreeListStubHeader*	psHeader;

	psHeader = &((SFreeListStubHeader*)pvData)[-1];
	if (psHeader->psPrev)
	{
		psHeader->psPrev->psNext = psHeader->psNext;
	}
	else
	{
		mpsNotFullHead = (SFNode*)psHeader->psNext;
	}
	if (psHeader->psNext)
	{
		psHeader->psNext->psPrev = psHeader->psPrev;
	}
	free(psHeader);
	muiNumElements--;
	return true;
}

