#include "BaseObject.h"
#include "Objects.h"
#include "DistCalculatorParameters.h"


#define EXPECTED_DISTS_LINEAR_SEARCH	16


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Init(void)
{
	muiWalk = gcObjects.StartWalk();
	macExpectedDists.Init();
	macExpectedDistsHeap.Init();
	mpuiExpectedDistsTable = NULL;
	muiExpectedDistsTableSize = 0;
	macClearCandidates.Init();
	mapcTos.Init();
	mapcDetachedFromRoot.Init();
	mapcCompletelyDetached.Init();
	mapcTouched.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Kill(void)
{
	mapcTouched.Kill();
	mapcCompletelyDetached.Kill();
	mapcDetachedFromRoot.Kill();
	mapcTos.Kill();
	macClearCandidates.Kill();
	SafeFree(mpuiExpectedDistsTable);
	muiExpectedDistsTableSize = 0;
	macExpectedDistsHeap.Kill();
	macExpectedDists.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each object is in the expected dists once (unless it has been removed);
// adding it again only ever lowers its expected dist.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddExpectedDist(CBaseObject* pcObject, int iExpectedDist)
{
	//This method is never called with an iExpectedDist less than ROOT_DIST_TO_ROOT

	SDistToRoot*	psDistToRoot;
	size			uiIndex;

	uiIndex = FindExpectedDistIndex(pcObject);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		psDistToRoot = macExpectedDists.Get(uiIndex);
		if (psDistToRoot->iExpectedDist > iExpectedDist)
		{
			psDistToRoot->iExpectedDist = iExpectedDist;
			PushExpectedDistsHeap(iExpectedDist, uiIndex);
		}
		return;
	}

	uiIndex = macExpectedDists.NumElements();
	psDistToRoot = macExpectedDists.Add();
	psDistToRoot->iExpectedDist = iExpectedDist;
	psDistToRoot->pcObject = pcObject;
	psDistToRoot->bRemoved = false;
	PushExpectedDistsHeap(iExpectedDist, uiIndex);
	PutExpectedDistsTable(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the object's (not removed) expected dist.
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::FindExpectedDistIndex(CBaseObject* pcObject)
{
	size			i;
	size			uiDists;
	size			uiSlot;
	size			uiEntry;
	SDistToRoot*	psDistToRoot;

	if (mpuiExpectedDistsTable == NULL)
	{
		uiDists = macExpectedDists.NumElements();
		for (i = 0; i < uiDists; i++)
		{
			psDistToRoot = macExpectedDists.Get(i);
			if ((psDistToRoot->pcObject == pcObject) && !psDistToRoot->bRemoved)
			{
				return i;
			}
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	uiSlot = ExpectedDistsTableSlot(pcObject);
	for (;;)
	{
		uiEntry = mpuiExpectedDistsTable[uiSlot];
		if (uiEntry == 0)
		{
			return ARRAY_ELEMENT_NOT_FOUND;
		}
		psDistToRoot = macExpectedDists.Get(uiEntry - 1);
		if ((psDistToRoot->pcObject == pcObject) && !psDistToRoot->bRemoved)
		{
			return uiEntry - 1;
		}
		uiSlot = (uiSlot + 1) & (muiExpectedDistsTableSize - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where the object's entries start in the table.
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::ExpectedDistsTableSlot(CBaseObject* pcObject)
{
	return (size)((((ptr)pcObject) >> 4) & (muiExpectedDistsTableSize - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the expected dist at uiIndex in the first free slot from its
// object's.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::InsertExpectedDistsTable(size uiIndex)
{
	size	uiSlot;

	uiSlot = ExpectedDistsTableSlot(macExpectedDists.Get(uiIndex)->pcObject);
	while (mpuiExpectedDistsTable[uiSlot] != 0)
	{
		uiSlot = (uiSlot + 1) & (muiExpectedDistsTableSize - 1);
	}
	mpuiExpectedDistsTable[uiSlot] = uiIndex + 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Removed entries stay in the table (an object removed and added again is
// in it twice); the table is only built once a linear search gets slow.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PutExpectedDistsTable(size uiIndex)
{
	size	uiDists;

	uiDists = macExpectedDists.NumElements();
	if (mpuiExpectedDistsTable == NULL)
	{
		if (uiDists > EXPECTED_DISTS_LINEAR_SEARCH)
		{
			GrowExpectedDistsTable();
		}
	}
	else if ((uiDists * 2) > muiExpectedDistsTableSize)
	{
		GrowExpectedDistsTable();
	}
	else
	{
		InsertExpectedDistsTable(uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Rebuilds the table, at least four times the number of expected dists.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::GrowExpectedDistsTable(void)
{
	size	uiSize;
	size	uiDists;
	size	i;

	uiDists = macExpectedDists.NumElements();
	uiSize = 64;
	while (uiSize < (uiDists * 4))
	{
		uiSize *= 2;
	}

	SafeFree(mpuiExpectedDistsTable);
	mpuiExpectedDistsTable = (size*)malloc(uiSize * sizeof(size));
	memset(mpuiExpectedDistsTable, 0, uiSize * sizeof(size));
	muiExpectedDistsTableSize = uiSize;

	for (i = 0; i < uiDists; i++)
	{
		InsertExpectedDistsTable(i);
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
void CDistCalculatorParameters::PushExpectedDistsHeap(int iExpectedDist, size uiIndex)
{
	SDistToRootHeapNode*	psNode;
	SDistToRootHeapNode		sNode;
	size					uiChild;
	size					uiParent;

	sNode.iExpectedDist = iExpectedDist;
	sNode.uiIndex = uiIndex;

	uiChild = macExpectedDistsHeap.NumElements();
	macExpectedDistsHeap.Add();
	while (uiChild > 0)
	{
		uiParent = (size)((uiChild - 1) / 2);
		psNode = macExpectedDistsHeap.Get(uiParent);
		if (!HeapNodeLess(&sNode, psNode))
		{
			break;
		}
		*macExpectedDistsHeap.Get(uiChild) = *psNode;
		uiChild = uiParent;
	}
	*macExpectedDistsHeap.Get(uiChild) = sNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PopExpectedDistsHeap(void)
{
	SDistToRootHeapNode		sLast;
	SDistToRootHeapNode*	psChild;
	size					uiNum;
	size					uiParent;
	size					uiChild;

	uiNum = macExpectedDistsHeap.NumElements() - 1;
	sLast = *macExpectedDistsHeap.Get(uiNum);
	macExpectedDistsHeap.RemoveTail();
	if (uiNum == 0)
	{
		return;
	}

	uiParent = 0;
	for (;;)
	{
		uiChild = (size)((uiParent * 2) + 1);
		if (uiChild >= uiNum)
		{
			break;
		}
		psChild = macExpectedDistsHeap.Get(uiChild);
		if ((uiChild + 1 < uiNum) && HeapNodeLess(macExpectedDistsHeap.Get(uiChild + 1), psChild))
		{
			uiChild++;
			psChild = macExpectedDistsHeap.Get(uiChild);
		}
		if (!HeapNodeLess(psChild, &sLast))
		{
			break;
		}
		*macExpectedDistsHeap.Get(uiParent) = *psChild;
		uiParent = uiChild;
	}
	*macExpectedDistsHeap.Get(uiParent) = sLast;
}


//////////////////////////////////////////////////////////////////////////
//
// A node is stale when its expected dist was removed or lowered since.
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculatorParameters::IsExpectedDistsHeapNodeStale(SDistToRootHeapNode* psNode)
{
	SDistToRoot*	psDistToRoot;

	psDistToRoot = macExpectedDists.Get(psNode->uiIndex);
	return psDistToRoot->bRemoved || (psDistToRoot->iExpectedDist != psNode->iExpectedDist);
}


//////////////////////////////////////////////////////////////////////////
//
// The lowest expected dist; of equal ones the first added.
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistCalculatorParameters::GetLowestExpectedDist(void)
{
	SDistToRootHeapNode*	psNode;

	while (macExpectedDistsHeap.NumElements() > 0)
	{
		psNode = macExpectedDistsHeap.Get(0);
		if (!IsExpectedDistsHeapNodeStale(psNode))
		{
			return macExpectedDists.Get(psNode->uiIndex);
		}
		PopExpectedDistsHeap();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Marked rather than removed: the heap and the table refer to entries by
// index.  The heap skips it when it reaches it.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::RemoveExpectedDist(SDistToRoot* psDistToRoot)
{
	if ((psDistToRoot == NULL) || psDistToRoot->bRemoved)
	{
		return;
	}
	psDistToRoot->bRemoved = true;
}


//////////////////////////////////////////////////////////////////////////
//
// An empty array for one object's pointer tos.  The same one each time, so
// walking many objects doesn't allocate for each.
//
//////////////////////////////////////////////////////////////////////////
CArrayEmbeddedObjectPtr* CDistCalculatorParameters::StartPointerTos(void)
{
	mapcTos.FakeSetUsedElements(0);
	return &mapcTos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddClearCandidate(CBaseObject* pcObject, int iClearedFromDistToRoot)
{
	SDistToRootClearCandidate*	psCandidate;

	psCandidate = macClearCandidates.Add();
	psCandidate->pcObject = pcObject;
	psCandidate->iClearedFromDistToRoot = iClearedFromDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
// The last candidate added; false when there are none.
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculatorParameters::PopClearCandidate(CBaseObject** ppcObject, int* piClearedFromDistToRoot)
{
	SDistToRootClearCandidate	sCandidate;

	if (!macClearCandidates.Pop(&sCandidate))
	{
		return false;
	}
	*ppcObject = sCandidate.pcObject;
	*piClearedFromDistToRoot = sCandidate.iClearedFromDistToRoot;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddDetachedFromRoot(CBaseObject* pcObject)
{
	mapcDetachedFromRoot.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumDetachedFromRoot(void)
{
	return mapcDetachedFromRoot.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetDetachedFromRoot(size iIndex)
{
	return mapcDetachedFromRoot.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddCompletelyDetached(CBaseObject* pcObject)
{
	mapcCompletelyDetached.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumCompletelyDetached(void)
{
	return mapcCompletelyDetached.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetCompletelyDetached(size iIndex)
{
	return mapcCompletelyDetached.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::RemoveCompletelyDetached(size iIndex)
{
	mapcCompletelyDetached.RemoveAt(iIndex, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::CopyRootDetachedToCompletelyDetached(void)
{
	//Poor mans copy for now.  Should resize and memcpy rather.
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = mapcDetachedFromRoot.NumElements();
	for (i = 0; i < iNum; i++)
	{
		pcBaseObject = *mapcDetachedFromRoot.Get(i);
		mapcCompletelyDetached.Add(pcBaseObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CDistCalculatorParameters::GetCompletelyDetachedArray(void)
{
	return &mapcCompletelyDetached;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddTouched(CBaseObject* pcObject)
{
	mapcTouched.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumTouched(void)
{
	return mapcTouched.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetTouched(size iIndex)
{
	return mapcTouched.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayBaseObjectPtrE32* pcArray, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		if (!bCommaSeparate)
		{
			psz->Append("  ");
		}

		pcBaseObject = *pcArray->Get(i);
		pcBaseObject->PrintObject(psz, false);
		
		if (bCommaSeparate)
		{
			if (i != iNum - 1)
			{
				psz->Append(", ");
			}
		}
		else
		{
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayBaseObjectPtr* pcArray, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		if (!bCommaSeparate)
		{
			psz->Append("  ");
		}

		pcBaseObject = *pcArray->Get(i);
		pcBaseObject->PrintObject(psz, false);

		if (bCommaSeparate)
		{
			if (i != iNum - 1)
			{
				psz->Append(", ");
			}
		}
		else
		{
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayDistToRoot* pcArray, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	SDistToRoot*	psDistToRoot;
	bool			bFirst;

	bFirst = true;
	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		psDistToRoot = pcArray->Get(i);
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


//////////////////////////////////////////////////////////////////////////
//
// What this calculation found the object to be (a WALK_STATE_...), or 0
// if it hasn't reached it.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDistCalculatorParameters::GetWalkState(CBaseObject* pcObject)
{
	uint32	uiStamp;

	uiStamp = pcObject->GetWalkStamp();
	if ((uiStamp & ~WALK_STAMP_STATE_MASK) != muiWalk)
	{
		return 0;
	}
	return uiStamp & WALK_STAMP_STATE_MASK;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::SetWalkState(CBaseObject* pcObject, uint32 uiState)
{
	pcObject->SetWalkStamp(muiWalk | uiState);
}


//////////////////////////////////////////////////////////////////////////
//
// The object is detached from the root but can be reached from the stack.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::MarkReachedFromStack(CBaseObject* pcObject)
{
	SetWalkState(pcObject, WALK_STATE_REACHED_FROM_STACK);
}


//////////////////////////////////////////////////////////////////////////
//
// (Heap froms are only ever objects allocated in Objects: pointers and
// collections in an object on the stack are stack froms.)
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculatorParameters::IsReachedFromStack(CBaseObject* pcObject)
{
	return pcObject->GetWalkStamp() == (muiWalk | WALK_STATE_REACHED_FROM_STACK);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Print(CChars* psz, bool bCommaSeparate)
{
	psz->Append("--------- DistCalculatorParameters ---------");
	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("      ");
	}
	psz->Append("Touched [");
	psz->Append(mapcTouched.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcTouched, bCommaSeparate);

	psz->AppendNewLine();

	psz->Append("ExpectedDists [");
	psz->Append(macExpectedDists.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &macExpectedDists, bCommaSeparate);
	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("     ");
	}
	psz->Append("Detached [");
	psz->Append(mapcDetachedFromRoot.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcDetachedFromRoot, bCommaSeparate);
	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("  ");
	}
	psz->Append("C. Detached [");
	psz->Append(mapcCompletelyDetached.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcCompletelyDetached, bCommaSeparate);
	psz->AppendNewLine();

	psz->Append("--------------------------------------------");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Dump(void)
{
	CChars sz;

	sz.Init();
	Print(&sz, false);
	sz.DumpKill();
}
