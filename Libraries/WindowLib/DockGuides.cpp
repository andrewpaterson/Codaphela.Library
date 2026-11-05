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
#include "DockArea.h"
#include "DockGroup.h"
#include "DocumentWell.h"
#include "DockGuides.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGuidesDraw::Init(CDockGuides* pcGuides)
{
	PreInit();
	CCanvasDraw::Init();
	mpcGuides = pcGuides;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGuidesDraw::Class(void)
{
	CCanvasDraw::Class();
	U_Pointer(mpcGuides);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockGuidesDraw::Draw(Ptr<CCanvas> pCanvas)
{
	if (mpcGuides && pCanvas->IsValid())
	{
		mpcGuides->DrawGuides(pCanvas);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The guides cover the area they are over.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, Ptr<CDockArea> pArea)
{
	PreInit();

	CCanvas::Init(pWindow, OMalloc<CDockGuidesDraw>(this));
	SetTransparent(true);
	SetAlignment(CSH_Stretch, CSV_Stretch);
	mpManager = pManager;
	mpArea = pArea;
	masGuides.Init();
	muiHover = MAX_SIZE;
	mcPreview.Init(0, 0, 0, 0);
	mbPreview = false;
	mpcCompassNode = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::Class(void)
{
	CCanvas::Class();

	M_Pointer(mpManager);
	M_Pointer(mpArea);
	U_Data(CArrayDockGuide, masGuides);
	U_Size(muiHover);
	U_Data(CInt32Rect, mcPreview);
	U_Bool(mbPreview);
	U_Pointer(mpcCompassNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::Free(void)
{
	masGuides.Kill();
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockGuides::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockGuides::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CDockGuides::GetDesiredSize(void)
{
	return SInt32Vec2(-1, -1);
}


//////////////////////////////////////////////////////////////////////////
//
// A guide with its top left at x, y.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::AddGuide(int x, int y, EDockTargetKind eKind, EDockSide eSide, CDockNode* pcNode, bool bOuter)
{
	SDockGuide*	psGuide;

	psGuide = masGuides.Add();
	psGuide->cRect.Init(x, y, x + DOCK_GUIDE_SIZE, y + DOCK_GUIDE_SIZE);
	psGuide->eKind = eKind;
	psGuide->eSide = eSide;
	psGuide->pcNode = pcNode;
	psGuide->bOuter = bOuter;
}


//////////////////////////////////////////////////////////////////////////
//
// A compass centred on pcBounds: iRing 1 is next to the centre, 2 the
// ring outside it.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::AddCompass(CInt32Rect* pcBounds, EDockTargetKind eSideKind, EDockTargetKind eCentreKind, CDockNode* pcNode, bool bSides, bool bCentre, int iRing, bool bOuter)
{
	int		cx;
	int		cy;
	int		iHalf;
	int		iStep;

	iHalf = DOCK_GUIDE_SIZE / 2;
	cx = pcBounds->GetLeft() + pcBounds->GetWidth() / 2;
	cy = pcBounds->GetTop() + pcBounds->GetHeight() / 2;
	iStep = DOCK_GUIDE_STEP * iRing;
	if (bCentre)
	{
		AddGuide(cx - iHalf, cy - iHalf, eCentreKind, DS_Centre, pcNode, false);
	}
	if (bSides)
	{
		AddGuide(cx - iHalf - iStep, cy - iHalf, eSideKind, DS_Left, pcNode, bOuter);
		AddGuide(cx - iHalf + iStep, cy - iHalf, eSideKind, DS_Right, pcNode, bOuter);
		AddGuide(cx - iHalf, cy - iHalf - iStep, eSideKind, DS_Top, pcNode, bOuter);
		AddGuide(cx - iHalf, cy - iHalf + iStep, eSideKind, DS_Bottom, pcNode, bOuter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The guides for something dragged to x, y in window client space;
// bDocuments if it holds documents.  They are only built again when the
// pointer goes over another group.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::Build(int x, int y, bool bDocuments, SInt32Vec2 sDraggedSize)
{
	CDockNode*	pcCompass;
	bool		bInWell;

	pcCompass = FindCompassNode(x, y, &bInWell);
	if ((pcCompass == mpcCompassNode) && (masGuides.NumElements() > 0))
	{
		return;
	}
	mpcCompassNode = pcCompass;
	masGuides.ReInit();
	muiHover = MAX_SIZE;
	mbPreview = false;

	//Tools dock along the main area's edges.
	if (mpArea->IsMain() && !bDocuments)
	{
		AddEdgeMarkers();
	}
	if (pcCompass)
	{
		AddCompassFor(pcCompass, bInWell, bDocuments);
	}
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// What the compass is over at x, y: the group there, or the empty well.
// *pbInWell if it is a group in the well.  NULL over neither.
//
//////////////////////////////////////////////////////////////////////////
CDockNode* CDockGuides::FindCompassNode(int x, int y, bool* pbInWell)
{
	Ptr<CDockGroup>		pGroup;
	Ptr<CDocumentWell>	pWell;

	*pbInWell = false;
	pWell = mpManager->GetWell();
	pGroup = mpArea->FindGroupAt(x, y);
	if (mpArea->IsMain() && pWell->IsPointIn(x, y))
	{
		if (pWell->GetRoot().IsNull())
		{
			return &pWell;
		}
		*pbInWell = pGroup.IsNotNull();
	}
	return pGroup.IsNotNull() ? &pGroup : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A marker in the middle of each of the area's edges.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::AddEdgeMarkers(void)
{
	CInt32Rect	cRoot;
	int			iHalf;
	int			iLeft;
	int			iTop;
	int			iRight;
	int			iBottom;

	iHalf = DOCK_GUIDE_SIZE / 2;
	mpArea->GetRootBounds(&cRoot);
	iLeft = cRoot.GetLeft() + DOCK_GUIDE_INSET;
	iTop = cRoot.GetTop() + DOCK_GUIDE_INSET;
	iRight = cRoot.GetLeft() + cRoot.GetWidth() - DOCK_GUIDE_INSET - DOCK_GUIDE_SIZE;
	iBottom = cRoot.GetTop() + cRoot.GetHeight() - DOCK_GUIDE_INSET - DOCK_GUIDE_SIZE;
	AddGuide(iLeft, cRoot.GetTop() + cRoot.GetHeight() / 2 - iHalf, DTK_AreaEdge, DS_Left, NULL, false);
	AddGuide(iRight, cRoot.GetTop() + cRoot.GetHeight() / 2 - iHalf, DTK_AreaEdge, DS_Right, NULL, false);
	AddGuide(cRoot.GetLeft() + cRoot.GetWidth() / 2 - iHalf, iTop, DTK_AreaEdge, DS_Top, NULL, false);
	AddGuide(cRoot.GetLeft() + cRoot.GetWidth() / 2 - iHalf, iBottom, DTK_AreaEdge, DS_Bottom, NULL, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The compass over pcNode.  Over the empty well its centre takes
// documents and tools alike, and tools can dock beside it.  Over a group
// in the well a second ring docks tools beside the whole well.  Over any
// other group documents only go with documents.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::AddCompassFor(CDockNode* pcNode, bool bInWell, bool bDocuments)
{
	CInt32Rect		cBounds;
	CDockGroup*		pcGroup;

	pcNode->GetActualBounds(&cBounds);
	if (pcNode->IsDocumentWell())
	{
		AddCompass(&cBounds, DTK_WellOuter, DTK_WellCentre, pcNode, !bDocuments, true, 1, true);
		return;
	}
	pcGroup = dynamic_cast<CDockGroup*>(pcNode);
	if (bInWell)
	{
		AddCompass(&cBounds, DTK_Group, DTK_Group, pcNode, true, true, 1, false);
		if (!bDocuments)
		{
			AddCompass(&cBounds, DTK_WellOuter, DTK_WellOuter, &mpManager->GetWell(), true, false, 2, true);
		}
	}
	else if (!bDocuments || pcGroup->IsDocumentGroup())
	{
		AddCompass(&cBounds, DTK_Group, DTK_Group, pcNode, true, true, 1, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The guide at x, y in window client space; psTarget says what dropping
// there does.  Returns false over no guide.
//
//////////////////////////////////////////////////////////////////////////
bool CDockGuides::Hit(int x, int y, SDockTarget* psTarget, SInt32Vec2 sDraggedSize)
{
	size		i;
	SDockGuide*	psGuide;
	size		uiHover;

	uiHover = MAX_SIZE;
	for (i = 0; i < masGuides.NumElements(); i++)
	{
		psGuide = masGuides.Get(i);
		if (psGuide->cRect.ContainsPoint(x, y))
		{
			uiHover = i;
			break;
		}
	}
	if (uiHover != muiHover)
	{
		muiHover = uiHover;
		Invalidate();
	}

	psTarget->Init();
	if (uiHover == MAX_SIZE)
	{
		mbPreview = false;
		return false;
	}
	psGuide = masGuides.Get(uiHover);
	psTarget->eKind = psGuide->eKind;
	psTarget->eSide = psGuide->eSide;
	psTarget->pcArea = &mpArea;
	psTarget->pcNode = psGuide->pcNode;
	PreviewFor(psGuide, sDraggedSize, &psTarget->cPreview);
	mcPreview.Init(&psTarget->cPreview);
	mbPreview = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The part of pcBounds on eSide iSize pixels across, all of it for the
// centre.
//
//////////////////////////////////////////////////////////////////////////
void SideRectangle(CInt32Rect* pcBounds, EDockSide eSide, int32 iSize, CInt32Rect* pcDest)
{
	int32	iLeft;
	int32	iTop;
	int32	iRight;
	int32	iBottom;

	iLeft = pcBounds->GetLeft();
	iTop = pcBounds->GetTop();
	iRight = iLeft + pcBounds->GetWidth();
	iBottom = iTop + pcBounds->GetHeight();
	switch (eSide)
	{
	case DS_Left:
		pcDest->Init(iLeft, iTop, iLeft + iSize, iBottom);
		break;
	case DS_Right:
		pcDest->Init(iRight - iSize, iTop, iRight, iBottom);
		break;
	case DS_Top:
		pcDest->Init(iLeft, iTop, iRight, iTop + iSize);
		break;
	case DS_Bottom:
		pcDest->Init(iLeft, iBottom - iSize, iRight, iBottom);
		break;
	default:
		pcDest->Init(pcBounds);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Docked beside something it takes its own size, but no more than half.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::PreviewFor(SDockGuide* psGuide, SInt32Vec2 sDraggedSize, CInt32Rect* pcDest)
{
	CInt32Rect	cBounds;
	int32		iSize;
	int32		iHalf;

	if (psGuide->eKind == DTK_AreaEdge)
	{
		mpArea->GetRootBounds(&cBounds);
	}
	else
	{
		psGuide->pcNode->GetActualBounds(&cBounds);
	}
	if (psGuide->eSide == DS_Centre)
	{
		pcDest->Init(&cBounds);
		return;
	}
	if (IsHorizontalDockSide(psGuide->eSide))
	{
		iSize = sDraggedSize.x;
		iHalf = cBounds.GetWidth() / 2;
	}
	else
	{
		iSize = sDraggedSize.y;
		iHalf = cBounds.GetHeight() / 2;
	}
	if ((iSize <= 0) || (iSize > iHalf))
	{
		iSize = iHalf;
	}
	SideRectangle(&cBounds, psGuide->eSide, iSize, pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
// The rectangle includes its right and bottom edges, as DrawBox's does,
// less the guides' own top left: the canvas is the area.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::DrawBoxClipped(Ptr<CCanvas> pCanvas, int iLeft, int iTop, int iRight, int iBottom, bool bFilled, ARGB32 sColour)
{
	CInt32Rect	cRect;
	SInt32Vec2	sPosition;

	sPosition = GetPosition();
	if ((iRight <= iLeft) || (iBottom <= iTop))
	{
		return;
	}
	cRect.Init(iLeft - sPosition.x, iTop - sPosition.y, iRight - 1 - sPosition.x, iBottom - 1 - sPosition.y);
	pCanvas->DrawBox(&cRect, bFilled, sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle pointing to eSide in the middle of the guide.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::DrawArrow(Ptr<CCanvas> pCanvas, CInt32Rect* pcRect, EDockSide eSide, ARGB32 sColour)
{
	int		cx;
	int		cy;
	int		i;
	int		iLength;

	cx = pcRect->GetLeft() + pcRect->GetWidth() / 2;
	cy = pcRect->GetTop() + pcRect->GetHeight() / 2;
	//A line a pixel thick for each step back from the tip, one pixel longer each side than the last.
	iLength = DOCK_GUIDE_ARROW_LENGTH;
	for (i = 0; i < iLength; i++)
	{
		switch (eSide)
		{
		case DS_Left:
			DrawBoxClipped(pCanvas, cx - DOCK_GUIDE_ARROW_TIP + i, cy - i, cx - DOCK_GUIDE_ARROW_TIP + 1 + i, cy + i + 1, true, sColour);
			break;
		case DS_Right:
			DrawBoxClipped(pCanvas, cx + DOCK_GUIDE_ARROW_TIP - 1 - i, cy - i, cx + DOCK_GUIDE_ARROW_TIP - i, cy + i + 1, true, sColour);
			break;
		case DS_Top:
			DrawBoxClipped(pCanvas, cx - i, cy - DOCK_GUIDE_ARROW_TIP + i, cx + i + 1, cy - DOCK_GUIDE_ARROW_TIP + 1 + i, true, sColour);
			break;
		default:
			DrawBoxClipped(pCanvas, cx - i, cy + DOCK_GUIDE_ARROW_TIP - 1 - i, cx + i + 1, cy + DOCK_GUIDE_ARROW_TIP - i, true, sColour);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The preview under the guides, then each guide: a square, an arrow on a
// side guide and a small window on a centre guide.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::DrawGuides(Ptr<CCanvas> pCanvas)
{
	size		i;
	SDockGuide*	psGuide;
	ARGB32		sFace;
	ARGB32		sBorder;
	ARGB32		sArrow;
	CInt32Rect*	pcRect;
	int			iLeft;
	int			iTop;
	int			iRight;
	int			iBottom;

	if (mbPreview)
	{
		iLeft = mcPreview.GetLeft();
		iTop = mcPreview.GetTop();
		iRight = iLeft + mcPreview.GetWidth();
		iBottom = iTop + mcPreview.GetHeight();
		DrawBoxClipped(pCanvas, iLeft, iTop, iRight, iBottom, true, mpManager->GetColour(DC_Preview));
		DrawBoxClipped(pCanvas, iLeft, iTop, iRight, iBottom, false, mpManager->GetColour(DC_PreviewBorder));
		DrawBoxClipped(pCanvas, iLeft + 1, iTop + 1, iRight - 1, iBottom - 1, false, mpManager->GetColour(DC_PreviewBorder));
	}

	sBorder = mpManager->GetColour(DC_GuideBorder);
	sArrow = mpManager->GetColour(DC_GuideArrow);
	for (i = 0; i < masGuides.NumElements(); i++)
	{
		psGuide = masGuides.Get(i);
		pcRect = &psGuide->cRect;
		sFace = mpManager->GetColour((i == muiHover) ? DC_GuideHover : DC_Guide);
		iLeft = pcRect->GetLeft();
		iTop = pcRect->GetTop();
		iRight = iLeft + pcRect->GetWidth();
		iBottom = iTop + pcRect->GetHeight();
		DrawBoxClipped(pCanvas, iLeft, iTop, iRight, iBottom, true, sFace);
		DrawBoxClipped(pCanvas, iLeft, iTop, iRight, iBottom, false, sBorder);
		if (psGuide->eSide == DS_Centre)
		{
			DrawBoxClipped(pCanvas, iLeft + DOCK_GUIDE_ICON_INSET, iTop + DOCK_GUIDE_ICON_INSET, iRight - DOCK_GUIDE_ICON_INSET, iBottom - DOCK_GUIDE_ICON_INSET, false, sArrow);
			DrawBoxClipped(pCanvas, iLeft + DOCK_GUIDE_ICON_INSET, iTop + DOCK_GUIDE_ICON_INSET, iRight - DOCK_GUIDE_ICON_INSET, iTop + DOCK_GUIDE_ICON_INSET + DOCK_GUIDE_ICON_TITLE, true, sArrow);
		}
		else
		{
			if (psGuide->bOuter || (psGuide->eKind == DTK_AreaEdge))
			{
				//Beside everything: a bar along the side the arrow points to.
				switch (psGuide->eSide)
				{
				case DS_Left:
					DrawBoxClipped(pCanvas, iLeft + DOCK_GUIDE_BAR_INSET, iTop + DOCK_GUIDE_BAR_END, iLeft + DOCK_GUIDE_BAR_INSET + DOCK_GUIDE_BAR_WIDTH, iBottom - DOCK_GUIDE_BAR_END, true, sArrow);
					break;
				case DS_Right:
					DrawBoxClipped(pCanvas, iRight - DOCK_GUIDE_BAR_INSET - DOCK_GUIDE_BAR_WIDTH, iTop + DOCK_GUIDE_BAR_END, iRight - DOCK_GUIDE_BAR_INSET, iBottom - DOCK_GUIDE_BAR_END, true, sArrow);
					break;
				case DS_Top:
					DrawBoxClipped(pCanvas, iLeft + DOCK_GUIDE_BAR_END, iTop + DOCK_GUIDE_BAR_INSET, iRight - DOCK_GUIDE_BAR_END, iTop + DOCK_GUIDE_BAR_INSET + DOCK_GUIDE_BAR_WIDTH, true, sArrow);
					break;
				default:
					DrawBoxClipped(pCanvas, iLeft + DOCK_GUIDE_BAR_END, iBottom - DOCK_GUIDE_BAR_INSET - DOCK_GUIDE_BAR_WIDTH, iRight - DOCK_GUIDE_BAR_END, iBottom - DOCK_GUIDE_BAR_INSET, true, sArrow);
					break;
				}
			}
			DrawArrow(pCanvas, pcRect, psGuide->eSide, sArrow);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first guide of that kind, side and node, NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
SDockGuide* CDockGuides::FindGuide(EDockTargetKind eKind, EDockSide eSide, CDockNode* pcNode, bool bOuter)
{
	size		i;
	SDockGuide*	psGuide;

	for (i = 0; i < masGuides.NumElements(); i++)
	{
		psGuide = masGuides.Get(i);
		if ((psGuide->eKind == eKind) && (psGuide->eSide == eSide) && ((pcNode == NULL) || (psGuide->pcNode == pcNode)) && (psGuide->bOuter == bOuter))
		{
			return psGuide;
		}
	}
	return NULL;
}


size		CDockGuides::NumGuides(void) { return masGuides.NumElements(); }
SDockGuide*	CDockGuides::GetGuide(size uiIndex) { return masGuides.Get(uiIndex); }

