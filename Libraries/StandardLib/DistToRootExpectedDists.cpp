#include "BaseObject.h"
#include "BinaryHeap.h"
#include "DistToRootExpectedDists.h"


#define EXPECTED_DISTS_LINEAR_SEARCH	16


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::Init(void)
{
	macDists.Init();
	macHeap.Init();
	mpuiTable = NULL;
	muiTableSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::Kill(void)
{
	SafeFree(mpuiTable);
	muiTableSize = 0;
	macHeap.Kill();
	macDists.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each object is in the expected dists once (unless it has been removed);
// adding it again only ever lowers its expected dist.
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::Add(CBaseObject* pcObject, int iExpectedDist)
{
	//This method is never called with an iExpectedDist less than ROOT_DIST_TO_ROOT

	SDistToRoot*	psDistToRoot;
	size			uiIndex;

	uiIndex = FindIndex(pcObject);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		psDistToRoot = macDists.Get(uiIndex);
		if (psDistToRoot->iExpectedDist > iExpectedDist)
		{
			psDistToRoot->iExpectedDist = iExpectedDist;
			PushHeap(iExpectedDist, uiIndex);
		}
		return;
	}

	uiIndex = macDists.NumElements();
	psDistToRoot = macDists.Add();
	psDistToRoot->iExpectedDist = iExpectedDist;
	psDistToRoot->pcObject = pcObject;
	psDistToRoot->bRemoved = false;
	PushHeap(iExpectedDist, uiIndex);
	PutTable(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the object's (not removed) expected dist.
//
//////////////////////////////////////////////////////////////////////////
size CDistToRootExpectedDists::FindIndex(CBaseObject* pcObject)
{
	size			i;
	size			uiDists;
	size			uiSlot;
	size			uiEntry;
	SDistToRoot*	psDistToRoot;

	if (mpuiTable == NULL)
	{
		uiDists = macDists.NumElements();
		for (i = 0; i < uiDists; i++)
		{
			psDistToRoot = macDists.Get(i);
			if ((psDistToRoot->pcObject == pcObject) && !psDistToRoot->bRemoved)
			{
				return i;
			}
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	uiSlot = TableSlot(pcObject);
	for (;;)
	{
		uiEntry = mpuiTable[uiSlot];
		if (uiEntry == 0)
		{
			return ARRAY_ELEMENT_NOT_FOUND;
		}
		psDistToRoot = macDists.Get(uiEntry - 1);
		if ((psDistToRoot->pcObject == pcObject) && !psDistToRoot->bRemoved)
		{
			return uiEntry - 1;
		}
		uiSlot = (uiSlot + 1) & (muiTableSize - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where the object's entries start in the table.
//
//////////////////////////////////////////////////////////////////////////
size CDistToRootExpectedDists::TableSlot(CBaseObject* pcObject)
{
	return (size)((((ptr)pcObject) >> 4) & (muiTableSize - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the expected dist at uiIndex in the first free slot from its
// object's.
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::InsertTable(size uiIndex)
{
	size	uiSlot;

	uiSlot = TableSlot(macDists.Get(uiIndex)->pcObject);
	while (mpuiTable[uiSlot] != 0)
	{
		uiSlot = (uiSlot + 1) & (muiTableSize - 1);
	}
	mpuiTable[uiSlot] = uiIndex + 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Removed entries stay in the table (an object removed and added again is
// in it twice); the table is only built once a linear search gets slow.
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::PutTable(size uiIndex)
{
	size	uiDists;

	uiDists = macDists.NumElements();
	if (mpuiTable == NULL)
	{
		if (uiDists > EXPECTED_DISTS_LINEAR_SEARCH)
		{
			GrowTable();
		}
	}
	else if ((uiDists * 2) > muiTableSize)
	{
		GrowTable();
	}
	else
	{
		InsertTable(uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Rebuilds the table, at least four times the number of expected dists.
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::GrowTable(void)
{
	size	uiSize;
	size	uiDists;
	size	i;

	uiDists = macDists.NumElements();
	uiSize = 64;
	while (uiSize < (uiDists * 4))
	{
		uiSize *= 2;
	}

	SafeFree(mpuiTable);
	mpuiTable = (size*)malloc((lsize)uiSize * sizeof(size));
	memset(mpuiTable, 0, (lsize)uiSize * sizeof(size));
	muiTableSize = uiSize;

	for (i = 0; i < uiDists; i++)
	{
		InsertTable(i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool HeapNodeLess(SDistToRootHeapNode* psLeft, SDistToRootHeapNode* psRight)
{
	if (psLeft->iExpectedDist != psRight->iExpectedDist)
	{
		return psLeft->iExpectedDist < psRight->iExpectedDist;
	}
	return psLeft->uiIndex < psRight->uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::PushHeap(int iExpectedDist, size uiIndex)
{
	SDistToRootHeapNode		sNode;

	sNode.iExpectedDist = iExpectedDist;
	sNode.uiIndex = uiIndex;
	BinaryHeapPush(&macHeap, &sNode, HeapNodeLess);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::PopHeap(void)
{
	SDistToRootHeapNode		sNode;

	BinaryHeapPop(&macHeap, &sNode, HeapNodeLess);
}


//////////////////////////////////////////////////////////////////////////
//
// A node is stale when its expected dist was removed or lowered since.
//
//////////////////////////////////////////////////////////////////////////
bool CDistToRootExpectedDists::IsHeapNodeStale(SDistToRootHeapNode* psNode)
{
	SDistToRoot*	psDistToRoot;

	psDistToRoot = macDists.Get(psNode->uiIndex);
	return psDistToRoot->bRemoved || (psDistToRoot->iExpectedDist != psNode->iExpectedDist);
}


//////////////////////////////////////////////////////////////////////////
//
// The lowest expected dist; of equal ones the first added.
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistToRootExpectedDists::GetLowest(void)
{
	SDistToRootHeapNode*	psNode;

	while (macHeap.NumElements() > 0)
	{
		psNode = macHeap.Get(0);
		if (!IsHeapNodeStale(psNode))
		{
			return macDists.Get(psNode->uiIndex);
		}
		PopHeap();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Marked rather than removed: the heap and the table refer to entries by
// index.  The heap skips it when it reaches it.
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::Remove(SDistToRoot* psDistToRoot)
{
	if ((psDistToRoot == NULL) || psDistToRoot->bRemoved)
	{
		return;
	}
	psDistToRoot->bRemoved = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::Print(CChars* psz, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	SDistToRoot*	psDistToRoot;
	bool			bFirst;

	bFirst = true;
	iNum = macDists.NumElements();
	for (i = 0; i < iNum; i++)
	{
		psDistToRoot = macDists.Get(i);
		if (psDistToRoot->bRemoved)
		{
			continue;
		}

		if (bCommaSeparate)
		{
			if (!bFirst)
			{
				psz->Append(", ");
			}
		}
		else
		{
			psz->Append("  ");
		}
		bFirst = false;

		psDistToRoot->pcObject->PrintObject(psz, false);

		if (!bCommaSeparate)
		{
			psz->AppendNewLine();
		}
	}
}

