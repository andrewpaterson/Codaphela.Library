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
#include "Popup.h"


//////////////////////////////////////////////////////////////////////////
//
// A new popup has no insets and is drawn with nothing behind its content.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::Init(Ptr<CWindow> pWindow, Ptr<CComponent> pContent, SInt32Vec2 sPosition, Ptr<CComponent> pOwner)
{
	PreInit();

	CContainer::Init(pWindow);
	mpContent = pContent;
	mpOwner = pOwner;
	msOpenPosition = sPosition;
	msInsets.Init();
	msBackground = 0;
	msBorder = 0;
	mbBorder = false;
	AddComponent(pContent);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPopup::Class(void)
{
	CContainer::Class();

	M_Pointer(mpContent);
	M_Pointer(mpOwner);
	U_2Int32(msOpenPosition);
	U_Data(SInsets, msInsets);
	U_UInt32(msBackground);
	U_UInt32(msBorder);
	U_Bool(mbBorder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPopup::Free(void)
{
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPopup::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPopup::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The border, if there is one, is drawn in the outside pixel of the
// insets so they should be at least one pixel.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::SetLook(ARGB32 sBackground, ARGB32 sBorder, SInsets* psInsets)
{
	msBackground = sBackground;
	msBorder = sBorder;
	mbBorder = true;
	memcpy(&msInsets, psInsets, sizeof(SInsets));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPopup::SetLook(ARGB32 sBackground, SInsets* psInsets)
{
	msBackground = sBackground;
	mbBorder = false;
	memcpy(&msInsets, psInsets, sizeof(SInsets));
}


//////////////////////////////////////////////////////////////////////////
//
// A popup is always the size it needs, however big the window is.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CPopup::GetDesiredSize(void)
{
	return GetDesiredOrRequiredSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPopup::SetRequiredSize(void)
{
	SInt32Vec2	sSize;

	sSize = mpContent->GetDesiredSize();
	if (sSize.x < 0)
	{
		sSize.x = 0;
	}
	if (sSize.y < 0)
	{
		sSize.y = 0;
	}
	sSize.x += msInsets.GetLeftAndRight();
	sSize.y += msInsets.GetTopAndBottom();
	msRequiredSize = sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The area is the whole window.  The popup ignores where it would put it
// and goes where it was opened, kept inside the area.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sSize;
	SInt32Vec2	sPopupPosition;
	SInt32Vec2	sContentPosition;
	SInt32Vec2	sContentSize;

	sSize = GetDesiredSize();
	sPopupPosition = msOpenPosition;
	if (sPopupPosition.x + sSize.x > sPosition.x + sAreaSize.x)
	{
		sPopupPosition.x = sPosition.x + sAreaSize.x - sSize.x;
	}
	if (sPopupPosition.y + sSize.y > sPosition.y + sAreaSize.y)
	{
		sPopupPosition.y = sPosition.y + sAreaSize.y - sSize.y;
	}
	if (sPopupPosition.x < sPosition.x)
	{
		sPopupPosition.x = sPosition.x;
	}
	if (sPopupPosition.y < sPosition.y)
	{
		sPopupPosition.y = sPosition.y;
	}

	sContentPosition = sPopupPosition;
	sContentSize = sSize;
	msInsets.Inset(&sContentPosition, &sContentSize);
	SetContainerBounds(sContentPosition, sContentSize);

	mpContent->Layout(sContentPosition, sContentSize);

	SetActualBounds(sPopupPosition, sSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The background and border go straight onto the window's canvas, under
// the content.
//
//////////////////////////////////////////////////////////////////////////
bool CPopup::Draw(void)
{
	Ptr<CCanvas>	pWindowCanvas;
	CInt32Rect		cRect;

	pWindowCanvas = mpWindow->GetCanvas();
	cRect.Init(mcActualBounds.GetLeft(), mcActualBounds.GetTop(), mcActualBounds.GetRight() - 1, mcActualBounds.GetBottom() - 1);
	if ((cRect.GetRight() >= cRect.GetLeft()) && (cRect.GetBottom() >= cRect.GetTop()))
	{
		if (mbBorder)
		{
			pWindowCanvas->DrawBox(&cRect, true, msBorder);
			cRect.Init(cRect.GetLeft() + 1, cRect.GetTop() + 1, cRect.GetRight() - 1, cRect.GetBottom() - 1);
		}
		if ((cRect.GetRight() >= cRect.GetLeft()) && (cRect.GetBottom() >= cRect.GetTop()))
		{
			pWindowCanvas->DrawBox(&cRect, true, msBackground);
		}
	}
	return DrawChildren();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CPopup::GetContent(void) { return mpContent; }
Ptr<CComponent> CPopup::GetOwner(void) { return mpOwner; }

