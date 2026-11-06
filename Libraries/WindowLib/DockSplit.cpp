/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Window.h"
#include "DockManager.h"
#include "DockGroup.h"
#include "DockPane.h"
#include "DockSplit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSash::Init(Ptr<CWindow> pWindow, Ptr<CDockSplit> pSplit, bool bHorizontal, ARGB32 sColour)
{
	PreInit();

	CCanvas::Init(pWindow, OMalloc<CClearCanvasDraw>(sColour));
	mpSplit = pSplit;
	mbDragging = false;
	miPressed = 0;
	SetCanGetFocus(true);
	SetAlignment(CSH_Stretch, CSV_Stretch);
	SetHorizontal(bHorizontal);
	AddInputAction(CIC_ActivateComponent, this, &CDockSash::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CDockSash::ExecuteComponent);
	AddInputAction(CIC_ContDragSelect, this, &CDockSash::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CDockSash::StopDragSelect);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSash::Class(void)
{
	CCanvas::Class();

	M_Pointer(mpSplit);
	U_Bool(mbDragging);
	U_Int32(miPressed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockSash::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockSash::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A horizontal split's sashes are upright and moved left and right.
//
//////////////////////////////////////////////////////////////////////////
void CDockSash::SetHorizontal(bool bHorizontal)
{
	SetCursor(bHorizontal ? NC_SizeWestEast : NC_SizeNorthSouth);
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the sash.
//
//////////////////////////////////////////////////////////////////////////
void CDockSash::ActivateComponent(int x, int y)
{
	SInt32Vec2	sPosition;

	sPosition = GetPosition();
	miPressed = mpSplit->IsHorizontal() ? sPosition.x + x : sPosition.y + y;
	mbDragging = true;
	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSash::ExecuteComponent(int x, int y)
{
	if (mbDragging)
	{
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The split moves the sash by how far the pointer has come since it was
// pressed or last moved it.  x and y are relative to the sash.
//
//////////////////////////////////////////////////////////////////////////
void CDockSash::ContDragSelect(int x, int y)
{
	SInt32Vec2	sPosition;
	int32		iAlong;

	if (!mbDragging)
	{
		return;
	}
	sPosition = GetPosition();
	iAlong = mpSplit->IsHorizontal() ? sPosition.x + x : sPosition.y + y;
	if (iAlong != miPressed)
	{
		mpSplit->SashMoved(this, iAlong - miPressed);
		miPressed = iAlong;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The keys go back to the active pane.
//
//////////////////////////////////////////////////////////////////////////
void CDockSash::StopDragSelect(int x, int y)
{
	Ptr<CDockManager>	pManager;

	mbDragging = false;
	pManager = mpSplit->GetManager();
	if (pManager.IsNotNull())
	{
		pManager->RestoreFocus(mpWindow);
	}
}


bool CDockSash::IsDragging(void) { return mbDragging; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, bool bHorizontal, ARGB32 sSashColour)
{
	PreInit();

	CDockNode::Init(pWindow, pManager);
	mbHorizontal = bHorizontal;
	maNodes.Init();
	maSashes.Init();
	maiSizes.Init();
	msSashColour = sSashColour;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::Class(void)
{
	CDockNode::Class();

	U_Bool(mbHorizontal);
	M_Embedded(maNodes);
	M_Embedded(maSashes);
	U_Data(CArrayInt, maiSizes);
	U_Data(ARGB32, msSashColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::Free(void)
{
	maiSizes.Kill();
	CDockNode::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EDockNodeKind CDockSplit::GetKind(void)
{
	return DNK_Split;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockSplit::ContainsDocumentWell(void)
{
	size	i;

	for (i = 0; i < maNodes.Size(); i++)
	{
		if (maNodes.Get(i)->ContainsDocumentWell())
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Every node's panes in order.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::GetPanes(CArray<CDockPane>* paPanes)
{
	size	i;

	for (i = 0; i < maNodes.Size(); i++)
	{
		maNodes.Get(i)->GetPanes(paPanes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockSplit::FindGroupAt(int x, int y)
{
	size			i;
	Ptr<CDockNode>	pNode;

	for (i = 0; i < maNodes.Size(); i++)
	{
		pNode = maNodes.Get(i);
		if (pNode->IsPointIn(x, y))
		{
			return pNode->FindGroupAt(x, y);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockSplit::GetFirstGroup(void)
{
	size			i;
	Ptr<CDockGroup>	pGroup;

	for (i = 0; i < maNodes.Size(); i++)
	{
		pGroup = maNodes.Get(i)->GetFirstGroup();
		if (pGroup.IsNotNull())
		{
			return pGroup;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The node goes before the node at uiIndex, or after the others.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::InsertNode(Ptr<CDockNode> pNode, size uiIndex)
{
	if (uiIndex > maNodes.Size())
	{
		uiIndex = maNodes.Size();
	}
	maNodes.Insert(uiIndex, pNode);
	PlaceChildren();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::AddNode(Ptr<CDockNode> pNode)
{
	InsertNode(pNode, maNodes.Size());
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if the node isn't in the split.
//
//////////////////////////////////////////////////////////////////////////
bool CDockSplit::RemoveNode(Ptr<CDockNode> pNode)
{
	size	uiIndex;

	uiIndex = IndexOf(pNode);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	maNodes.RemoveAt(uiIndex);
	PlaceChildren();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// pNew takes pOld's place and its dock size.
//
//////////////////////////////////////////////////////////////////////////
bool CDockSplit::ReplaceNode(Ptr<CDockNode> pOld, Ptr<CDockNode> pNew)
{
	size	uiIndex;

	uiIndex = IndexOf(pOld);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	pNew->SetDockSize(pOld->GetDockSize());
	maNodes.RemoveAt(uiIndex);
	maNodes.Insert(uiIndex, pNew);
	PlaceChildren();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The nodes and a sash between each pair become the split's children, in
// order.  Sashes are kept and reused.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::PlaceChildren(void)
{
	size			i;
	Ptr<CDockSash>	pSash;
	Ptr<CWindow>	pWindow;

	RemoveAllComponents();
	while (maSashes.Size() + 1 < maNodes.Size())
	{
		pSash = OMalloc<CDockSash>(mpWindow, Ptr<CDockSplit>(this), mbHorizontal, msSashColour);
		maSashes.Add(pSash);
	}
	while ((maSashes.Size() > 0) && (maSashes.Size() + 1 > maNodes.Size()))
	{
		maSashes.RemoveAt(maSashes.Size() - 1);
	}
	for (i = 0; i < maNodes.Size(); i++)
	{
		if (i > 0)
		{
			AddComponent(maSashes.Get(i - 1));
		}
		AddComponent(maNodes.Get(i));
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::SetHorizontal(bool bHorizontal)
{
	size	i;

	mbHorizontal = bHorizontal;
	for (i = 0; i < maSashes.Size(); i++)
	{
		maSashes.Get(i)->SetHorizontal(bHorizontal);
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDockSplit::IndexOf(Ptr<CDockNode> pNode)
{
	size	i;

	for (i = 0; i < maNodes.Size(); i++)
	{
		if (maNodes.Get(i) == pNode)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CDockSplit::GetAlong(SInt32Vec2 sSize)
{
	return mbHorizontal ? sSize.x : sSize.y;
}


//////////////////////////////////////////////////////////////////////////
//
// Each node needs at least the minimum along the split, and the most any
// node needs across it.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::SetRequiredSize(void)
{
	int32	iAlong;

	iAlong = (int32)maNodes.Size() * DOCK_MINIMUM_SIZE;
	if (maNodes.Size() > 1)
	{
		iAlong += (int32)(maNodes.Size() - 1) * DOCK_SASH_SIZE;
	}
	if (mbHorizontal)
	{
		msRequiredSize.Init(iAlong, DOCK_MINIMUM_SIZE);
	}
	else
	{
		msRequiredSize.Init(DOCK_MINIMUM_SIZE, iAlong);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each node's size along the split from iAvailable, the space less the
// sashes.  The node with the document well takes what the others leave;
// without it every node is scaled by its dock size.  Nodes are kept to the
// minimum while there is room, the others giving way.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::WorkOutSizes(int32 iAvailable, CArrayInt* paiSizes)
{
	size	uiFill;

	paiSizes->Kill();
	paiSizes->Init();
	if (maNodes.Size() == 0)
	{
		return;
	}
	if (iAvailable < 0)
	{
		iAvailable = 0;
	}
	uiFill = StartSizes(iAvailable, paiSizes);
	if (uiFill != ARRAY_ELEMENT_NOT_FOUND)
	{
		FillSize(iAvailable, uiFill, paiSizes);
	}
	else
	{
		ScaleSizes(iAvailable, paiSizes);
	}
	KeepMinimums(paiSizes);
}


//////////////////////////////////////////////////////////////////////////
//
// Each node's dock size, or an even share for a node without one.
// Returns the index of the node with the document well, if any.
//
//////////////////////////////////////////////////////////////////////////
size CDockSplit::StartSizes(int32 iAvailable, CArrayInt* paiSizes)
{
	size	i;
	size	uiFill;
	int32	iSize;
	int32	iShare;

	iShare = iAvailable / (int32)maNodes.Size();
	uiFill = ARRAY_ELEMENT_NOT_FOUND;
	for (i = 0; i < maNodes.Size(); i++)
	{
		iSize = maNodes.Get(i)->GetDockSize();
		paiSizes->Add((iSize > 0) ? iSize : iShare);
		if ((uiFill == ARRAY_ELEMENT_NOT_FOUND) && maNodes.Get(i)->ContainsDocumentWell())
		{
			uiFill = i;
		}
	}
	return uiFill;
}


//////////////////////////////////////////////////////////////////////////
//
// The node at uiFill takes what the others leave.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::FillSize(int32 iAvailable, size uiFill, CArrayInt* paiSizes)
{
	size	i;
	int32	iOthers;

	iOthers = 0;
	for (i = 0; i < paiSizes->NumElements(); i++)
	{
		if (i != uiFill)
		{
			iOthers += paiSizes->GetValue(i);
		}
	}
	paiSizes->SetValue(uiFill, iAvailable - iOthers);
}


//////////////////////////////////////////////////////////////////////////
//
// Every node is scaled by its size to fill iAvailable; the last takes
// what rounding leaves.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::ScaleSizes(int32 iAvailable, CArrayInt* paiSizes)
{
	size	i;
	size	uiNumNodes;
	int32	iTotal;
	int32	iUsed;
	int64	iScaled;

	uiNumNodes = paiSizes->NumElements();
	iTotal = 0;
	for (i = 0; i < uiNumNodes; i++)
	{
		iTotal += paiSizes->GetValue(i);
	}
	if (iTotal <= 0)
	{
		return;
	}
	iUsed = 0;
	for (i = 0; i < uiNumNodes; i++)
	{
		if (i + 1 == uiNumNodes)
		{
			paiSizes->SetValue(i, iAvailable - iUsed);
		}
		else
		{
			iScaled = ((int64)paiSizes->GetValue(i) * iAvailable) / iTotal;
			paiSizes->SetValue(i, (int32)iScaled);
			iUsed += (int32)iScaled;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Nodes under the minimum take from the largest others while they can.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::KeepMinimums(CArrayInt* paiSizes)
{
	size	i;
	size	uiLargest;
	int32	iNeeded;
	int32	iSpare;

	for (i = 0; i < paiSizes->NumElements(); i++)
	{
		iNeeded = DOCK_MINIMUM_SIZE - paiSizes->GetValue(i);
		while (iNeeded > 0)
		{
			uiLargest = FindLargestOther(paiSizes, i);
			if (uiLargest == ARRAY_ELEMENT_NOT_FOUND)
			{
				break;
			}
			iSpare = paiSizes->GetValue(uiLargest) - DOCK_MINIMUM_SIZE;
			if (iSpare > iNeeded)
			{
				iSpare = iNeeded;
			}
			paiSizes->SetValue(uiLargest, paiSizes->GetValue(uiLargest) - iSpare);
			paiSizes->SetValue(i, paiSizes->GetValue(i) + iSpare);
			iNeeded -= iSpare;
		}
		if (paiSizes->GetValue(i) < 0)
		{
			paiSizes->SetValue(i, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The largest node other than uiExcept that is over the minimum,
// ARRAY_ELEMENT_NOT_FOUND if none is.
//
//////////////////////////////////////////////////////////////////////////
size CDockSplit::FindLargestOther(CArrayInt* paiSizes, size uiExcept)
{
	size	i;
	size	uiLargest;
	int32	iLargest;

	uiLargest = ARRAY_ELEMENT_NOT_FOUND;
	iLargest = DOCK_MINIMUM_SIZE;
	for (i = 0; i < paiSizes->NumElements(); i++)
	{
		if ((i != uiExcept) && (paiSizes->GetValue(i) > iLargest))
		{
			iLargest = paiSizes->GetValue(i);
			uiLargest = i;
		}
	}
	return uiLargest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size			i;
	int32			iAvailable;
	int32			iAlong;
	int32			iSize;

	SetActualBounds(sPosition, sAreaSize);
	SetContainerBounds(sPosition, sAreaSize);

	iAvailable = GetAlong(sAreaSize);
	if (maNodes.Size() > 1)
	{
		iAvailable -= (int32)(maNodes.Size() - 1) * DOCK_SASH_SIZE;
	}
	WorkOutSizes(iAvailable, &maiSizes);

	iAlong = 0;
	for (i = 0; i < maNodes.Size(); i++)
	{
		if (i > 0)
		{
			LayoutAlong(maSashes.Get(i - 1), sPosition, sAreaSize, iAlong, DOCK_SASH_SIZE);
			iAlong += DOCK_SASH_SIZE;
		}
		iSize = maiSizes.GetValue(i);
		LayoutAlong(maNodes.Get(i), sPosition, sAreaSize, iAlong, iSize);
		iAlong += iSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Lays a node or sash out iAlong pixels into the split, iSize along
// and the split's full size across.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::LayoutAlong(Ptr<CComponent> pComponent, SInt32Vec2 sPosition, SInt32Vec2 sAreaSize, int32 iAlong, int32 iSize)
{
	if (mbHorizontal)
	{
		pComponent->Layout(SInt32Vec2(sPosition.x + iAlong, sPosition.y), SInt32Vec2(iSize, sAreaSize.y));
	}
	else
	{
		pComponent->Layout(SInt32Vec2(sPosition.x, sPosition.y + iAlong), SInt32Vec2(sAreaSize.x, iSize));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The sash's nodes on each side take their sizes from the last layout,
// iDelta moving from one to the other but neither going under the
// minimum.  Every node's dock size becomes its laid out size so the
// others don't move.
//
//////////////////////////////////////////////////////////////////////////
void CDockSplit::SashMoved(Ptr<CDockSash> pSash, int32 iDelta)
{
	size	uiSash;
	size	i;
	int32	iBefore;
	int32	iAfter;

	for (uiSash = 0; uiSash < maSashes.Size(); uiSash++)
	{
		if (maSashes.Get(uiSash) == pSash)
		{
			break;
		}
	}
	if ((uiSash >= maSashes.Size()) || (maiSizes.NumElements() != maNodes.Size()))
	{
		return;
	}

	iBefore = maiSizes.GetValue(uiSash);
	iAfter = maiSizes.GetValue(uiSash + 1);
	if (iBefore + iDelta < DOCK_MINIMUM_SIZE)
	{
		iDelta = DOCK_MINIMUM_SIZE - iBefore;
	}
	if (iAfter - iDelta < DOCK_MINIMUM_SIZE)
	{
		iDelta = iAfter - DOCK_MINIMUM_SIZE;
	}
	if ((iBefore + iDelta < DOCK_MINIMUM_SIZE) || (iAfter - iDelta < DOCK_MINIMUM_SIZE))
	{
		return;
	}

	for (i = 0; i < maNodes.Size(); i++)
	{
		maNodes.Get(i)->SetDockSize(maiSizes.GetValue(i));
	}
	maNodes.Get(uiSash)->SetDockSize(iBefore + iDelta);
	maNodes.Get(uiSash + 1)->SetDockSize(iAfter - iDelta);
	maiSizes.SetValue(uiSash, iBefore + iDelta);
	maiSizes.SetValue(uiSash + 1, iAfter - iDelta);
	InvalidateLayout();
	mpManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size				CDockSplit::NumNodes(void) { return maNodes.Size(); }
Ptr<CDockNode>		CDockSplit::GetNode(size uiIndex) { return maNodes.Get(uiIndex); }
bool				CDockSplit::IsHorizontal(void) { return mbHorizontal; }
Ptr<CDockSash>		CDockSplit::GetSash(size uiIndex) { return maSashes.Get(uiIndex); }
int32				CDockSplit::GetNodeSize(size uiIndex) { return (uiIndex < maiSizes.NumElements()) ? maiSizes.GetValue(uiIndex) : 0; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDockSplit::CountPanes(void)
{
	size	i;
	size	uiCount;

	uiCount = 0;
	for (i = 0; i < maNodes.Size(); i++)
	{
		uiCount += maNodes.Get(i)->CountPanes();
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane at uiIndex counting through the nodes in order.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockSplit::GetTreePane(size uiIndex)
{
	size			i;
	size			uiCount;
	Ptr<CDockNode>	pNode;

	for (i = 0; i < maNodes.Size(); i++)
	{
		pNode = maNodes.Get(i);
		uiCount = pNode->CountPanes();
		if (uiIndex < uiCount)
		{
			return pNode->GetTreePane(uiIndex);
		}
		uiIndex -= uiCount;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Something docked on eSide of a node in the split would be next to it
// in the split: left and right in a horizontal split, top and bottom in a
// vertical one.
//
//////////////////////////////////////////////////////////////////////////
bool CDockSplit::IsAlong(EDockSide eSide)
{
	return mbHorizontal == IsHorizontalDockSide(eSide);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockSplit::IsSameDirection(Ptr<CDockSplit> pOther)
{
	return pOther.IsNotNull() && (pOther->IsHorizontal() == mbHorizontal);
}

