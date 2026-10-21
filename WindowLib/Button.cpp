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
#include "ClearCanvasDraw.h"
#include "BorderLayoutHelper.h"
#include "Window.h"
#include "Button.h"


//The cell of each EButtonBorderPiece.
static EBorderLayoutCell gaeButtonBorderCell[NUM_BUTTON_BORDER_PIECES] = { BLC_TopLeft, BLC_Top, BLC_TopRight, BLC_Left, BLC_Right, BLC_BottomLeft, BLC_Bottom, BLC_BottomRight };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages, Ptr<CFont> pFont, const char* szCaption)
{
	Init(pWindow, pImages, NULL, pFont, szCaption);
}


//////////////////////////////////////////////////////////////////////////
//
// The default images must have the same tile size as the images.
//
//////////////////////////////////////////////////////////////////////////
void CButton::Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages, Ptr<CFont> pFont, const char* szCaption)
{
	CBorderLayoutHelper		cBorder;
	Ptr<CImageCanvasDraw>	pDraw;
	Ptr<CCanvas>			pCanvas;
	Ptr<CCanvas>			pBackground;
	int						i;
	int32					iTileSize;
	bool					bEdge;

	PreInit();

	CGridContainer::Init(pWindow);
	mpImages = pImages;
	mpDefaultImages = pDefaultImages;
	maBorderDraws.Init();
	msPadding.Init(8, 2);
	mcListeners.Init();
	mcListeners.AddAllowedClass<CButtonListener>();
	meState = BS_Normal;
	mbMouseOver = false;
	mbFocussed = false;
	mbPressed = false;
	mbDefault = false;
	SetCanGetFocus(true);

	//The corners are their tile's size.  The edges are a tile thick and stretched along the face.  Every piece is
	//transparent: the corners are rounded and any piece can have an empty band, so what is behind shows through.
	cBorder.Init(Ptr<CGridContainer>(this));
	iTileSize = mpImages->GetTileSize();
	for (i = 0; i < NUM_BUTTON_BORDER_PIECES; i++)
	{
		bEdge = (i == BBP_Top) || (i == BBP_Left) || (i == BBP_Right) || (i == BBP_Bottom);
		pDraw = OMalloc<CImageCanvasDraw>(mpImages->GetCel(BS_Normal, (EButtonBorderPiece)i), bEdge);
		maBorderDraws.Add(pDraw);
		pCanvas = OMalloc<CCanvas>(pWindow, pDraw);
		pCanvas->SetTransparent(true);
		if ((i == BBP_Top) || (i == BBP_Bottom))
		{
			pCanvas->SetDesiredSize(-1, iTileSize);
		}
		else if ((i == BBP_Left) || (i == BBP_Right))
		{
			pCanvas->SetDesiredSize(iTileSize, -1);
		}
		cBorder.AddComponent(pCanvas, gaeButtonBorderCell[i]);
	}

	mpFace = OMalloc<CFillContainer>(pWindow);
	mpFaceDraw = OMalloc<CClearCanvasDraw>(mpImages->GetFaceColour(BS_Normal));
	pBackground = OMalloc<CCanvas>(pWindow, mpFaceDraw);
	mpFace->AddComponent(pBackground);
	mpCaption = OMalloc<CLabel>(pWindow, pFont, szCaption);
	mpFace->AddComponent(mpCaption);
	mpFace->SetAlignment(CSH_Stretch, CSV_Stretch);
	cBorder.AddComponent(mpFace, BLC_Center);
	cBorder.Kill();
	UpdateFaceSize();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpImages);
	M_Pointer(mpDefaultImages);
	M_Pointer(mpFaceDraw);
	M_Embedded(maBorderDraws);
	M_Pointer(mpFace);
	M_Pointer(mpCaption);
	U_2Int32(msPadding);
	U_Data(CListenerCall, mcListeners);
	U_Enum(meState);
	U_Bool(mbMouseOver);
	U_Bool(mbFocussed);
	U_Bool(mbPressed);
	U_Bool(mbDefault);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Free(void)
{
	mcListeners.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButton::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButton::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetCaption(const char* szCaption)
{
	mpCaption->SetText(szCaption);
	UpdateFaceSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetPadding(int32 iHorizontal, int32 iVertical)
{
	msPadding.Init(iHorizontal, iVertical);
	UpdateFaceSize();
}


//////////////////////////////////////////////////////////////////////////
//
// The face's desired size is what the grid makes room for.  Being
// stretched it still fills its cell.
//
//////////////////////////////////////////////////////////////////////////
void CButton::UpdateFaceSize(void)
{
	SInt32Vec2	sCaption;

	sCaption = mpCaption->GetDesiredSize();
	mpFace->SetDesiredSize(sCaption.x + msPadding.x * 2, sCaption.y + msPadding.y * 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Like a label a button keeps the size its parts need unless a desired
// size is set on an axis.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CButton::GetDesiredSize(void)
{
	return GetDesiredOrRequiredSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButton::AddListener(CButtonListener* pcListener)
{
	return mcListeners.AddListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButton::RemoveListener(CButtonListener* pcListener)
{
	return mcListeners.RemoveListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Click(void)
{
	mcListeners.CallListeners(&CButtonListener::ButtonClicked, this, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are in window client space.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CButton::FindComponentAt(int x, int y)
{
	if (IsPointIn(x, y))
	{
		return this;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A focussed button clicks itself on enter, not the default button.
//
//////////////////////////////////////////////////////////////////////////
bool CButton::WantsEnter(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Only the window calls this, see CWindow::SetDefaultButton.
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetDefault(bool bDefault)
{
	if (mbDefault != bDefault)
	{
		mbDefault = bDefault;
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The images the button is drawn with now.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CButton::GetImages(void)
{
	if (mbDefault && mpDefaultImages)
	{
		return mpDefaultImages;
	}
	return mpImages;
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a press.
//
//////////////////////////////////////////////////////////////////////////
void CButton::FocusGained(void)
{
	mbFocussed = true;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::FocusLost(void)
{
	mbFocussed = false;
	mbPressed = false;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::MovedInto(void)
{
	mbMouseOver = true;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::MovedOutof(void)
{
	mbMouseOver = false;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
// The press has already given the button focus.
//
//////////////////////////////////////////////////////////////////////////
void CButton::ActivateComponent(int x, int y)
{
	mbPressed = true;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
// The release goes to the button with focus wherever the pointer is.  It
// only clicks if the pointer is still over the button.
//
//////////////////////////////////////////////////////////////////////////
void CButton::ExecuteComponent(int x, int y)
{
	if (mbPressed)
	{
		mbPressed = false;
		UpdateState();
		if (mbMouseOver)
		{
			Click();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Space(bool bInsert)
{
	Click();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Enter(void)
{
	Click();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushed while pressed with the pointer over it, then mouse over, then
// selected while it has focus.
//
//////////////////////////////////////////////////////////////////////////
void CButton::UpdateState(void)
{
	EButtonState	eState;

	if (mbPressed && mbMouseOver)
	{
		eState = BS_Pushed;
	}
	else if (mbMouseOver)
	{
		eState = BS_MouseOver;
	}
	else if (mbFocussed)
	{
		eState = BS_Selected;
	}
	else
	{
		eState = BS_Normal;
	}

	if (eState != meState)
	{
		meState = eState;
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::UpdateLook(void)
{
	Ptr<CButtonImages>	pImages;
	int					i;

	pImages = GetImages();
	for (i = 0; i < NUM_BUTTON_BORDER_PIECES; i++)
	{
		maBorderDraws.Get(i)->SetCel(pImages->GetCel(meState, (EButtonBorderPiece)i));
	}
	mpFaceDraw->SetColour(pImages->GetFaceColour(meState));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CButton::GetCaption(void) { return mpCaption->GetText(); }
EButtonState CButton::GetState(void) { return meState; }
bool CButton::IsPressed(void) { return mbPressed; }
bool CButton::IsDefault(void) { return mbDefault; }

