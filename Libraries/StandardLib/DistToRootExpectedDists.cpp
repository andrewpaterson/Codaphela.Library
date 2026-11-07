/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
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
	mcIndices.Init();
	mbIndexed = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::Kill(void)
{
	mcIndices.Kill();
	mbIndexed = false;
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
	AddIndex(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the object's (not removed) expected dist.
//
//////////////////////////////////////////////////////////////////////////
size CDistToRootExpectedDists::FindIndex(CBaseObject* pcObject)
{
	size						i;
	size						uiDists;
	SDistToRoot*				psDistToRoot;
	SPointerLocationsIterator	sIter;
	ptr							uiIndex;
	bool						bExists;

	//An object removed and added again has more than one index: the one not removed is it.
	if (!mbIndexed)
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

	bExists = mcIndices.StartIteration(&sIter, pcObject, &uiIndex);
	while (bExists)
	{
		if (!macDists.Get((size)uiIndex)->bRemoved)
		{
			return (size)uiIndex;
		}
		bExists = mcIndices.Iterate(&sIter, &uiIndex);
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// Few objects are found by looking through them.  Once there are more than
// EXPECTED_DISTS_LINEAR_SEARCH each index is kept by its object, removed
// ones too (an object removed and added again has two).
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootExpectedDists::AddIndex(size uiIndex)
{
	size	i;
	size	uiDists;

	if (mbIndexed)
	{
		mcIndices.Add(macDists.Get(uiIndex)->pcObject, uiIndex);
		return;
	}

	uiDists = macDists.NumElements();
	if (uiDists > EXPECTED_DISTS_LINEAR_SEARCH)
	{
		for (i = 0; i < uiDists; i++)
		{
			mcIndices.Add(macDists.Get(i)->pcObject, i);
		}
		mbIndexed = true;
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

