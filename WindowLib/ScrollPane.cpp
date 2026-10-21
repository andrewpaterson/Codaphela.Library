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
#include "ScrollPane.h"


//////////////////////////////////////////////////////////////////////////
//
// The scroll bars' thumbs are drawn with pImages, which can be shared.
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::Init(Ptr<CWindow> pWindow, Ptr<CScrollBarImages> pImages)
{
	PreInit();

	CContainer::Init(pWindow);
	mpContent = NULL;
	meHorizontalPolicy = SPP_AsNeeded;
	meVerticalPolicy = SPP_AsNeeded;
	msScrollPosition.Init(0, 0);
	msContentSize.Init(0, 0);
	miMargin = 2;
	miWheelStep = 48;

	//The scroll bars are the last children so they are drawn over the content and hit before it.
	mpHorizontal = OMalloc<CScrollBar>(pWindow, Ptr<CScrollPane>(this), pImages, SBO_Horizontal);
	CContainer::AddComponent(mpHorizontal);
	mpVertical = OMalloc<CScrollBar>(pWindow, Ptr<CScrollPane>(this), pImages, SBO_Vertical);
	CContainer::AddComponent(mpVertical);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::Class(void)
{
	CContainer::Class();

	M_Pointer(mpContent);
	M_Pointer(mpHorizontal);
	M_Pointer(mpVertical);
	U_Enum(meHorizontalPolicy);
	U_Enum(meVerticalPolicy);
	U_2Int32(msScrollPosition);
	U_2Int32(msContentSize);
	U_Int32(miMargin);
	U_Int32(miWheelStep);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::Free(void)
{
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScrollPane::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScrollPane::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the content, if there was any.  The scroll position goes back
// to the top left.
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::SetContent(Ptr<CComponent> pContent)
{
	if (mpContent.IsNotNull())
	{
		RemoveComponent(mpContent);
	}

	mpContent = pContent;
	msScrollPosition.Init(0, 0);
	if (mpContent.IsNotNull())
	{
		//The content is the first child so it is drawn under the scroll bars.
		CContainer::AddComponent(mpContent);
		maChildren.Remove(mpContent);
		maChildren.Insert(0, mpContent);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A scroll pane holds one component, adding one sets the content.
//
//////////////////////////////////////////////////////////////////////////
bool CScrollPane::AddComponent(Ptr<CComponent> pComponent)
{
	SetContent(pComponent);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::SetPolicy(EScrollBarOrientation eOrientation, EScrollPanePolicy ePolicy)
{
	if (eOrientation == SBO_Horizontal)
	{
		meHorizontalPolicy = ePolicy;
	}
	else
	{
		meVerticalPolicy = ePolicy;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EScrollPanePolicy CScrollPane::GetPolicy(EScrollBarOrientation eOrientation)
{
	if (eOrientation == SBO_Horizontal)
	{
		return meHorizontalPolicy;
	}
	return meVerticalPolicy;
}


//////////////////////////////////////////////////////////////////////////
//
// How far the content is scrolled from its top left.  The content isn't
// moved until the scroll pane is next laid out.  Before the scroll pane
// has been laid out the content's size isn't known so the position is
// kept as asked for and limited to the maximum when it is used.  A word
// wrapped text area, say, only knows its height after a first layout.
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::SetScrollPosition(int32 x, int32 y)
{
	msScrollPosition.Init(x, y);
	ClampScrollPosition();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::SetScrollPosition(EScrollBarOrientation eOrientation, int32 iPosition)
{
	if (eOrientation == SBO_Horizontal)
	{
		SetScrollPosition(iPosition, msScrollPosition.y);
	}
	else
	{
		SetScrollPosition(msScrollPosition.x, iPosition);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The content's size less the view's, as last laid out.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CScrollPane::GetMaximumScrollPosition(void)
{
	SInt32Vec2	sMaximum;

	sMaximum.x = msContentSize.x - mcActualBounds.GetWidth();
	sMaximum.y = msContentSize.y - mcActualBounds.GetHeight();
	if (sMaximum.x < 0)
	{
		sMaximum.x = 0;
	}
	if (sMaximum.y < 0)
	{
		sMaximum.y = 0;
	}
	return sMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::ClampScrollPosition(void)
{
	SInt32Vec2	sMaximum;

	if (msScrollPosition.x < 0)
	{
		msScrollPosition.x = 0;
	}
	if (msScrollPosition.y < 0)
	{
		msScrollPosition.y = 0;
	}

	//Not laid out yet, the content's size isn't known.
	if ((msContentSize.x == 0) && (msContentSize.y == 0))
	{
		return;
	}

	sMaximum = GetMaximumScrollPosition();
	if (msScrollPosition.x > sMaximum.x)
	{
		msScrollPosition.x = sMaximum.x;
	}
	if (msScrollPosition.y > sMaximum.y)
	{
		msScrollPosition.y = sMaximum.y;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The position the content is scrolled to: the one asked for, no further
// than the maximum.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CScrollPane::GetScrollPosition(void)
{
	SInt32Vec2	sPosition;
	SInt32Vec2	sMaximum;

	sPosition = msScrollPosition;
	sMaximum = GetMaximumScrollPosition();
	if (sPosition.x > sMaximum.x)
	{
		sPosition.x = sMaximum.x;
	}
	if (sPosition.y > sMaximum.y)
	{
		sPosition.y = sMaximum.y;
	}
	return sPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// A scroll pane can be as small as its container makes it.
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::SetRequiredSize(void)
{
	msRequiredSize.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The length the content is laid out to on one axis.
//
//////////////////////////////////////////////////////////////////////////
int32 CScrollPane::GetContentLength(int32 iDesired, int32 iView, EScrollPanePolicy ePolicy)
{
	if ((ePolicy == SPP_Never) || (iDesired < iView))
	{
		return iView;
	}
	return iDesired;
}


//////////////////////////////////////////////////////////////////////////
//
// The view is the whole area.  The content is laid out first, moved by the
// scroll position, then the scroll bars over it.
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sDesired;
	SInt32Vec2	sScrollPosition;
	SInt32Vec2	sContentPosition;
	SInt32Vec2	sTrackPosition;
	SInt32Vec2	sTrackSize;
	int32		iThickness;
	bool		bHorizontal;
	bool		bVertical;

	SetContainerBounds(sPosition, sAreaSize);
	SetActualBounds(sPosition, sAreaSize);

	if (mpContent.IsNotNull())
	{
		sDesired = mpContent->GetDesiredSize();
		msContentSize.x = GetContentLength(sDesired.x, sAreaSize.x, meHorizontalPolicy);
		msContentSize.y = GetContentLength(sDesired.y, sAreaSize.y, meVerticalPolicy);
	}
	else
	{
		msContentSize = sAreaSize;
	}
	sScrollPosition = GetScrollPosition();

	if (mpContent.IsNotNull())
	{
		sContentPosition.x = sPosition.x - sScrollPosition.x;
		sContentPosition.y = sPosition.y - sScrollPosition.y;
		LayoutContent(sContentPosition);
	}

	mpHorizontal->SetRange(msContentSize.x, sAreaSize.x, sScrollPosition.x);
	mpVertical->SetRange(msContentSize.y, sAreaSize.y, sScrollPosition.y);
	bHorizontal = mpHorizontal->IsShown();
	bVertical = mpVertical->IsShown();

	//Each track is inset by the margin and leaves the corner to the other when both are shown.
	mpHorizontal->SetRequiredSize();
	iThickness = mpHorizontal->GetRequiredSize().y;

	sTrackPosition.Init(sPosition.x + miMargin, sPosition.y + sAreaSize.y - iThickness - miMargin);
	sTrackSize.Init(sAreaSize.x - miMargin * 2 - (bVertical ? iThickness + miMargin : 0), iThickness);
	mpHorizontal->Layout(sTrackPosition, sTrackSize);

	sTrackPosition.Init(sPosition.x + sAreaSize.x - iThickness - miMargin, sPosition.y + miMargin);
	sTrackSize.Init(iThickness, sAreaSize.y - miMargin * 2 - (bHorizontal ? iThickness + miMargin : 0));
	mpVertical->Layout(sTrackPosition, sTrackSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The content gets the whole content size on an axis unless it is smaller
// there and not stretched, then it keeps its size and is placed by its
// alignment.  Content bigger than the view on an axis it is never scrolled
// on is shrunk to fit.
//
//////////////////////////////////////////////////////////////////////////
void CScrollPane::LayoutContent(SInt32Vec2 sContentPosition)
{
	SInt32Vec2					sSize;
	SInt32Vec2					sPosition;
	EContainerStyleHorizontal	eHorizontal;
	EContainerStyleVertical		eVertical;

	sSize = mpContent->GetDesiredSize();
	eHorizontal = mpContent->GetHorizontalAlignment();
	eVertical = mpContent->GetVerticalAlignment();
	if ((sSize.x < 0) || (sSize.x > msContentSize.x) || (eHorizontal == CSH_Stretch))
	{
		sSize.x = msContentSize.x;
	}
	if ((sSize.y < 0) || (sSize.y > msContentSize.y) || (eVertical == CSV_Stretch))
	{
		sSize.y = msContentSize.y;
	}

	sPosition.x = sContentPosition.x + AlignComponentHorizontal(eHorizontal, sSize.x, msContentSize.x);
	sPosition.y = sContentPosition.y + AlignComponentVertical(eVertical, sSize.y, msContentSize.y);
	mpContent->Layout(sPosition, sSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The scroll pane uses the wheel on an axis it can scroll on, even when
// it is already scrolled to the end, so a scroll pane it is inside doesn't
// suddenly start scrolling instead.  Otherwise the wheel goes on to the
// scroll pane's parent.
//
//////////////////////////////////////////////////////////////////////////
bool CScrollPane::Wheel(int iRight, int iDown)
{
	SInt32Vec2	sPosition;
	SInt32Vec2	sMaximum;
	bool		bUsed;

	sPosition = GetScrollPosition();
	sMaximum = GetMaximumScrollPosition();
	bUsed = false;
	if ((iRight != 0) && (sMaximum.x > 0))
	{
		sPosition.x += iRight * miWheelStep;
		bUsed = true;
	}
	if ((iDown != 0) && (sMaximum.y > 0))
	{
		sPosition.y += iDown * miWheelStep;
		bUsed = true;
	}

	if (bUsed)
	{
		SetScrollPosition(sPosition.x, sPosition.y);
	}
	return bUsed;
}


//////////////////////////////////////////////////////////////////////////
//
// The content is only drawn inside the scroll pane.
//
//////////////////////////////////////////////////////////////////////////
bool CScrollPane::ClipsChildren(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CScrollBar> CScrollPane::GetScrollBar(EScrollBarOrientation eOrientation)
{
	if (eOrientation == SBO_Horizontal)
	{
		return mpHorizontal;
	}
	return mpVertical;
}


Ptr<CComponent> CScrollPane::GetContent(void) { return mpContent; }
void CScrollPane::SetMargin(int32 iMargin) { miMargin = iMargin; }
void CScrollPane::SetWheelStep(int32 iWheelStep) { miWheelStep = iWheelStep; }
int32 CScrollPane::GetWheelStep(void) { return miWheelStep; }
SInt32Vec2 CScrollPane::GetContentSize(void) { return msContentSize; }

