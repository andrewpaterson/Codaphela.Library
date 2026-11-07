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
#include "BorderLayoutHelper.h"
#include "Window.h"
#include "ImageBorderContainer.h"


//The cell of each EButtonBorderPiece.
static EBorderLayoutCell gaeImageBorderCell[NUM_BUTTON_BORDER_PIECES] = { BLC_TopLeft, BLC_Top, BLC_TopRight, BLC_Left, BLC_Right, BLC_BottomLeft, BLC_Bottom, BLC_BottomRight };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages, int32 iHorizontalPadding, int32 iVerticalPadding)
{
	CBorderLayoutHelper		cBorder;
	Ptr<CImageCanvasDraw>	pDraw;
	Ptr<CCanvas>			pCanvas;
	Ptr<CCanvas>			pBackground;
	size					i;
	int32					iTileSize;
	bool					bEdge;

	PreInit();

	CGridContainer::Init(pWindow);
	SetSizedByContent(true);
	SetHitAsWhole(true);
	mpImages = pImages;
	maBorderDraws.Init();
	meBorderState = BS_Normal;

	//As a button's: the corners are their tile's size and the edges are a tile thick and stretched along the face.  Every
	//piece is transparent so what is behind shows through the rounded corners and any empty band.
	cBorder.Init(this);
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
		cBorder.AddComponent(pCanvas, gaeImageBorderCell[i]);
	}

	mpFace = OMalloc<CFillContainer>(pWindow);
	mpFaceDraw = OMalloc<CClearCanvasDraw>(mpImages->GetFaceColour(BS_Normal));
	pBackground = OMalloc<CCanvas>(pWindow, mpFaceDraw);
	mpFace->AddComponent(pBackground);
	mpFace->SetAlignment(CSH_Stretch, CSV_Stretch);
	cBorder.AddComponent(mpFace, BLC_Center);
	cBorder.Kill();

	//The face's background covers the padding.
	mpPadded = OMalloc<CFillContainer>(pWindow);
	mpPadded->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpFace->AddComponent(mpPadded);
	SetPaddedInsets(iHorizontalPadding, iVerticalPadding);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpImages);
	M_Embedded(maBorderDraws);
	M_Pointer(mpFaceDraw);
	M_Pointer(mpFace);
	M_Pointer(mpPadded);
	U_2Int32(msPadding);
	U_Enum(meBorderState);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBorderContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBorderContainer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::SetBorderState(EButtonState eState)
{
	if (eState != meBorderState)
	{
		meBorderState = eState;
		UpdateBorder();

		//The border's draws changed, which their canvases can't see.
		Invalidate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The border as a field's (a text field's, a combo box's): the focus ring
// while focussed, the mouse over look while the pointer is over it, and
// normal otherwise.
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::UpdateBorderState(bool bFocussed)
{
	if (bFocussed)
	{
		SetBorderState(BS_Focussed);
	}
	else if (IsMouseOver())
	{
		SetBorderState(BS_MouseOver);
	}
	else
	{
		SetBorderState(BS_Normal);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the border with other images, like a button that has become the
// default.  They must have the same tile size.
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::SetImages(Ptr<CButtonImages> pImages)
{
	if (&mpImages != &pImages)
	{
		mpImages = pImages;
		UpdateBorder();
	}

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::UpdateBorder(void)
{
	size	i;

	for (i = 0; i < NUM_BUTTON_BORDER_PIECES; i++)
	{
		maBorderDraws.Get(i)->SetCel(mpImages->GetCel(meBorderState, (EButtonBorderPiece)i));
	}
	mpFaceDraw->SetColour(mpImages->GetFaceColour(meBorderState));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::SetPadding(int32 iHorizontal, int32 iVertical)
{
	SetPaddedInsets(iHorizontal, iVertical);
	UpdateFaceSize();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::SetPaddedInsets(int32 iHorizontal, int32 iVertical)
{
	SInsets		sInsets;

	msPadding.Init(iHorizontal, iVertical);
	sInsets.Init(iHorizontal, iVertical);
	mpPadded->SetInsets(&sInsets);
}


//////////////////////////////////////////////////////////////////////////
//
// The face's desired size is what the border's grid makes room for.
// Being stretched it still fills its cell.
//
//////////////////////////////////////////////////////////////////////////
void CImageBorderContainer::UpdateFaceSize(void)
{
	SInt32Vec2	sContent;

	sContent = GetContentSize();
	mpFace->SetDesiredSize(sContent.x + msPadding.x * 2, sContent.y + msPadding.y * 2);
}


//////////////////////////////////////////////////////////////////////////
//
// What the padded face needs, without the padding.  Nothing unless the
// container says otherwise.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CImageBorderContainer::GetContentSize(void)
{
	return SInt32Vec2(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFillContainer> CImageBorderContainer::GetFace(void) { return mpFace; }
Ptr<CFillContainer> CImageBorderContainer::GetPaddedFace(void) { return mpPadded; }
SInt32Vec2 CImageBorderContainer::GetPadding(void) { return msPadding; }
Ptr<CButtonImages> CImageBorderContainer::GetImages(void) { return mpImages; }
EButtonState CImageBorderContainer::GetBorderState(void) { return meBorderState; }

