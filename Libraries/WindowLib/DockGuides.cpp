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
// bDocuments if it holds documents.
//
//////////////////////////////////////////////////////////////////////////
void CDockGuides::Build(int x, int y, bool bDocuments, SInt32Vec2 sDraggedSize)
{
	CInt32Rect			cRoot;
	CInt32Rect			cBounds;
	Ptr<CDockGroup>		pGroup;
	Ptr<CDocumentWell>	pWell;
	bool				bInWell;
	int					iHalf;
	CDockNode*			pcCompass;

	iHalf = DOCK_GUIDE_SIZE / 2;
	mpArea->GetRootBounds(&cRoot);
	pWell = mpManager->GetWell();

	pGroup = mpArea->FindGroupAt(x, y);
	bInWell = false;
	pcCompass = NULL;
	if (mpArea->IsMain() && pWell->IsPointIn(x, y))
	{
		if (pWell->GetRoot().IsNull())
		{
			pcCompass = &pWell;
		}
		else if (pGroup.IsNotNull())
		{
			pcCompass = &pGroup;
			bInWell = true;
		}
	}
	else if (pGroup.IsNotNull())
	{
		pcCompass = &pGroup;
	}

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
		AddGuide(cRoot.GetLeft() + DOCK_GUIDE_INSET, cRoot.GetTop() + cRoot.GetHeight() / 2 - iHalf, DTK_AreaEdge, DS_Left, NULL, false);
		AddGuide(cRoot.GetLeft() + cRoot.GetWidth() - DOCK_GUIDE_INSET - DOCK_GUIDE_SIZE, cRoot.GetTop() + cRoot.GetHeight() / 2 - iHalf, DTK_AreaEdge, DS_Right, NULL, false);
		AddGuide(cRoot.GetLeft() + cRoot.GetWidth() / 2 - iHalf, cRoot.GetTop() + DOCK_GUIDE_INSET, DTK_AreaEdge, DS_Top, NULL, false);
		AddGuide(cRoot.GetLeft() + cRoot.GetWidth() / 2 - iHalf, cRoot.GetTop() + cRoot.GetHeight() - DOCK_GUIDE_INSET - DOCK_GUIDE_SIZE, DTK_AreaEdge, DS_Bottom, NULL, false);
	}

	if (pcCompass == NULL)
	{
		Invalidate();
		return;
	}

	if (pcCompass == &pWell)
	{
		//The empty well: its centre takes documents and tools alike; tools can dock beside it.
		pWell->GetActualBounds(&cBounds);
		AddCompass(&cBounds, DTK_WellOuter, DTK_WellCentre, pcCompass, !bDocuments, true, 1, true);
	}
	else if (bInWell)
	{
		pGroup->GetActualBounds(&cBounds);
		AddCompass(&cBounds, DTK_Group, DTK_Group, pcCompass, true, true, 1, false);
		if (!bDocuments)
		{
			AddCompass(&cBounds, DTK_WellOuter, DTK_WellOuter, &pWell, true, false, 2, true);
		}
	}
	else
	{
		//A tool group, in the main area or a floating one: documents only go with documents.
		pGroup->GetActualBounds(&cBounds);
		if (!bDocuments || pGroup->IsDocumentGroup())
		{
			AddCompass(&cBounds, DTK_Group, DTK_Group, pcCompass, true, true, 1, false);
		}
	}
	Invalidate();
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
	iLength = 6;
	for (i = 0; i < iLength; i++)
	{
		switch (eSide)
		{
		case DS_Left:
			DrawBoxClipped(pCanvas, cx - 3 + i, cy - i, cx - 2 + i, cy + i + 1, true, sColour);
			break;
		case DS_Right:
			DrawBoxClipped(pCanvas, cx + 2 - i, cy - i, cx + 3 - i, cy + i + 1, true, sColour);
			break;
		case DS_Top:
			DrawBoxClipped(pCanvas, cx - i, cy - 3 + i, cx + i + 1, cy - 2 + i, true, sColour);
			break;
		default:
			DrawBoxClipped(pCanvas, cx - i, cy + 2 - i, cx + i + 1, cy + 3 - i, true, sColour);
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
			DrawBoxClipped(pCanvas, iLeft + 8, iTop + 8, iRight - 8, iBottom - 8, false, sArrow);
			DrawBoxClipped(pCanvas, iLeft + 8, iTop + 8, iRight - 8, iTop + 11, true, sArrow);
		}
		else
		{
			if (psGuide->bOuter || (psGuide->eKind == DTK_AreaEdge))
			{
				//Beside everything: a bar along the side the arrow points to.
				switch (psGuide->eSide)
				{
				case DS_Left:
					DrawBoxClipped(pCanvas, iLeft + 5, iTop + 7, iLeft + 7, iBottom - 7, true, sArrow);
					break;
				case DS_Right:
					DrawBoxClipped(pCanvas, iRight - 7, iTop + 7, iRight - 5, iBottom - 7, true, sArrow);
					break;
				case DS_Top:
					DrawBoxClipped(pCanvas, iLeft + 7, iTop + 5, iRight - 7, iTop + 7, true, sArrow);
					break;
				default:
					DrawBoxClipped(pCanvas, iLeft + 7, iBottom - 7, iRight - 7, iBottom - 5, true, sArrow);
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

