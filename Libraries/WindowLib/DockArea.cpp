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
#include "DockManager.h"
#include "DockGroup.h"
#include "AutoHide.h"
#include "DockGuides.h"
#include "DockFloatingWindow.h"
#include "DockArea.h"


//////////////////////////////////////////////////////////////////////////
//
// The main window's area (bMain) has the auto hide strips.
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, bool bMain)
{
	PreInit();

	CContainer::Init(pWindow);
	mpManager = pManager;
	mpRoot = NULL;
	mbMain = bMain;
	mpGuides = NULL;
	mpFloating = NULL;
	mcRootBounds.Init(0, 0, 0, 0);
	SetAlignment(CSH_Stretch, CSV_Stretch);
	mpBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pManager->GetColour(TC_DockBackground)));
	mpBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	if (bMain)
	{
		mpLeftStrip = OMalloc<CAutoHideStrip>(pWindow, pManager, DS_Left);
		mpRightStrip = OMalloc<CAutoHideStrip>(pWindow, pManager, DS_Right);
		mpTopStrip = OMalloc<CAutoHideStrip>(pWindow, pManager, DS_Top);
		mpBottomStrip = OMalloc<CAutoHideStrip>(pWindow, pManager, DS_Bottom);
	}
	else
	{
		mpLeftStrip = NULL;
		mpRightStrip = NULL;
		mpTopStrip = NULL;
		mpBottomStrip = NULL;
	}
	PlaceChildren();
	AddInputAction(CIC_CloseDocument, &pManager, &CDockManager::CloseActiveDocument);
	AddInputAction(CIC_HideToolWindow, &pManager, &CDockManager::HideActiveTool);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::Class(void)
{
	CContainer::Class();

	M_Pointer(mpManager);
	M_Pointer(mpRoot);
	U_Bool(mbMain);
	M_Pointer(mpBackground);
	M_Pointer(mpLeftStrip);
	M_Pointer(mpRightStrip);
	M_Pointer(mpTopStrip);
	M_Pointer(mpBottomStrip);
	M_Pointer(mpGuides);
	M_Pointer(mpFloating);
	U_Data(CInt32Rect, mcRootBounds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockArea::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockArea::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The background, the root, the strips with tabs, then the guides over
// everything.
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::PlaceChildren(void)
{
	RemoveAllComponents();
	AddComponent(mpBackground);
	if (mpRoot.IsNotNull())
	{
		AddComponent(mpRoot);
	}
	if (mbMain)
	{
		AddComponent(mpLeftStrip);
		AddComponent(mpRightStrip);
		AddComponent(mpTopStrip);
		AddComponent(mpBottomStrip);
	}
	if (mpGuides.IsNotNull())
	{
		AddComponent(mpGuides);
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::SetRoot(Ptr<CDockNode> pRoot)
{
	mpRoot = pRoot;
	PlaceChildren();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::StripsChanged(void)
{
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::SetRequiredSize(void)
{
	msRequiredSize.Init(DOCK_MINIMUM_SIZE, DOCK_MINIMUM_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
// The strips with tabs take their edges, the top and bottom strips
// between the left and right ones; the root fills the rest.
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	int32		iLeft;
	int32		iTop;
	int32		iRight;
	int32		iBottom;

	SetActualBounds(sPosition, sAreaSize);
	SetContainerBounds(sPosition, sAreaSize);
	mpBackground->Layout(sPosition, sAreaSize);

	iLeft = sPosition.x;
	iTop = sPosition.y;
	iRight = sPosition.x + sAreaSize.x;
	iBottom = sPosition.y + sAreaSize.y;
	if (mbMain)
	{
		LayoutStrip(mpLeftStrip, DS_Left, &iLeft, &iTop, &iRight, &iBottom);
		LayoutStrip(mpRightStrip, DS_Right, &iLeft, &iTop, &iRight, &iBottom);
		LayoutStrip(mpTopStrip, DS_Top, &iLeft, &iTop, &iRight, &iBottom);
		LayoutStrip(mpBottomStrip, DS_Bottom, &iLeft, &iTop, &iRight, &iBottom);
	}

	if (iRight < iLeft)
	{
		iRight = iLeft;
	}
	if (iBottom < iTop)
	{
		iBottom = iTop;
	}
	mcRootBounds.Init(iLeft, iTop, iRight, iBottom);
	if (mpRoot.IsNotNull())
	{
		mpRoot->Layout(SInt32Vec2(iLeft, iTop), SInt32Vec2(iRight - iLeft, iBottom - iTop));
	}
	if (mpGuides.IsNotNull())
	{
		mpGuides->Layout(sPosition, sAreaSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A strip with tabs takes its side's edge of what is left; one without
// takes nothing.
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::LayoutStrip(Ptr<CAutoHideStrip> pStrip, EDockSide eSide, int32* piLeft, int32* piTop, int32* piRight, int32* piBottom)
{
	int32		iThickness;

	if (!pStrip->HasTabs())
	{
		pStrip->Layout(SInt32Vec2((eSide == DS_Right) ? *piRight : *piLeft, (eSide == DS_Bottom) ? *piBottom : *piTop), SInt32Vec2(0, 0));
		return;
	}

	iThickness = AUTO_HIDE_STRIP_SIZE;
	switch (eSide)
	{
	case DS_Left:
		pStrip->Layout(SInt32Vec2(*piLeft, *piTop), SInt32Vec2(iThickness, *piBottom - *piTop));
		*piLeft += iThickness;
		break;
	case DS_Right:
		pStrip->Layout(SInt32Vec2(*piRight - iThickness, *piTop), SInt32Vec2(iThickness, *piBottom - *piTop));
		*piRight -= iThickness;
		break;
	case DS_Top:
		pStrip->Layout(SInt32Vec2(*piLeft, *piTop), SInt32Vec2(*piRight - *piLeft, iThickness));
		*piTop += iThickness;
		break;
	default:
		pStrip->Layout(SInt32Vec2(*piLeft, *piBottom - iThickness), SInt32Vec2(*piRight - *piLeft, iThickness));
		*piBottom -= iThickness;
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for an area without strips.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CAutoHideStrip> CDockArea::GetStrip(EDockSide eSide)
{
	switch (eSide)
	{
	case DS_Left:
		return mpLeftStrip;
	case DS_Right:
		return mpRightStrip;
	case DS_Top:
		return mpTopStrip;
	case DS_Bottom:
		return mpBottomStrip;
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Inside the strips, in window client space.
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::GetRootBounds(CInt32Rect* pcDest)
{
	pcDest->Init(&mcRootBounds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockArea::FindGroupAt(int x, int y)
{
	if (mpRoot.IsNotNull() && mpRoot->IsPointIn(x, y))
	{
		return mpRoot->FindGroupAt(x, y);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The guides are drawn over everything in the area.
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::ShowGuides(Ptr<CDockGuides> pGuides)
{
	if (mpGuides != pGuides)
	{
		mpGuides = pGuides;
		PlaceChildren();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockArea::HideGuides(void)
{
	if (mpGuides.IsNotNull())
	{
		mpGuides = NULL;
		PlaceChildren();
	}
}


Ptr<CDockNode>		CDockArea::GetRoot(void) { return mpRoot; }
bool				CDockArea::IsMain(void) { return mbMain; }
Ptr<CDockGuides>	CDockArea::GetGuides(void) { return mpGuides; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockArea::ReplaceNode(Ptr<CDockNode> pOld, Ptr<CDockNode> pNew)
{
	if (pOld != mpRoot)
	{
		return false;
	}
	pNew->SetDockSize(pOld->GetDockSize());
	SetRoot(pNew);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockArea::RemoveNode(Ptr<CDockNode> pNode)
{
	if (pNode != mpRoot)
	{
		return false;
	}
	SetRoot(NULL);
	return true;
}


void						CDockArea::SetFloating(Ptr<CDockFloatingWindow> pFloating) { mpFloating = pFloating; }
Ptr<CDockFloatingWindow>	CDockArea::GetFloating(void) { return mpFloating; }

