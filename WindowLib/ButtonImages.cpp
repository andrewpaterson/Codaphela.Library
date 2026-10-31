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
#include "SupportLib/ImageAccessorCreator.h"
#include "ButtonImages.h"


//The tile in a strip and the clockwise quarter turns for each EButtonBorderPiece.
static int gaiButtonTile[NUM_BUTTON_BORDER_PIECES] = {			4, 0, 5, 3, 1, 7, 2, 6 };
static int gaiButtonQuarterTurns[NUM_BUTTON_BORDER_PIECES] = {	0, 0, 0, 3, 1, 0, 2, 0 };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonImages::Init(Ptr<CImage> pNormal, Ptr<CImage> pMouseOver, Ptr<CImage> pSelected, Ptr<CImage> pPushed, ARGB32 sFaceColour)
{
	Init(pNormal, pMouseOver, pSelected, pPushed, sFaceColour, sFaceColour, sFaceColour, sFaceColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonImages::Init(Ptr<CImage> pNormal, Ptr<CImage> pMouseOver, Ptr<CImage> pSelected, Ptr<CImage> pPushed, ARGB32 sNormalFace, ARGB32 sMouseOverFace, ARGB32 sSelectedFace, ARGB32 sPushedFace)
{
	PreInit();

	maCels.Init();
	masFaceColours[BS_Normal] = sNormalFace;
	masFaceColours[BS_MouseOver] = sMouseOverFace;
	masFaceColours[BS_Focussed] = sSelectedFace;
	masFaceColours[BS_Pushed] = sPushedFace;
	miTileSize = (int32)pNormal->GetWidth();

	AddStrip(pNormal);
	AddStrip(pMouseOver);
	AddStrip(pSelected);
	AddStrip(pPushed);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonImages::Class(void)
{
	M_Embedded(maCels);
	U_UInt32Array(masFaceColours, NUM_BUTTON_STATES);
	U_Int32(miTileSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonImages::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButtonImages::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButtonImages::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonImages::AddStrip(Ptr<CImage> pStrip)
{
	size	i;

	for (i = 0; i < NUM_BUTTON_BORDER_PIECES; i++)
	{
		maCels.Add(CreateCel(pStrip, gaiButtonTile[i], gaiButtonQuarterTurns[i]));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An unturned tile is a cel of the strip.  A turned one is copied into an
// image of its own.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CButtonImages::CreateCel(Ptr<CImage> pStrip, int iTile, int iQuarterTurns)
{
	Ptr<CImage>		pTile;
	CImageAccessor*	pcSource;
	CImageAccessor*	pcDest;
	uint			x;
	uint			y;
	int				iTop;
	int				iLast;

	iTop = iTile * miTileSize;
	if (iQuarterTurns == 0)
	{
		return CropCel(OMalloc<CImageCel>(pStrip, 0, iTop, miTileSize, iTop + miTileSize));
	}

	iLast = miTileSize - 1;
	pTile = OMalloc<CImage>(miTileSize, miTileSize, pStrip);
	pcSource = CImageAccessorCreator::Create(&pStrip);
	pcDest = CImageAccessorCreator::Create(&pTile);
	for (y = 0; y < (uint)miTileSize; y++)
	{
		for (x = 0; x < (uint)miTileSize; x++)
		{
			if (iQuarterTurns == 1)
			{
				pcDest->Set(x, y, pcSource->Get(y, iTop + iLast - x));
			}
			else if (iQuarterTurns == 2)
			{
				pcDest->Set(x, y, pcSource->Get(iLast - x, iTop + iLast - y));
			}
			else
			{
				pcDest->Set(x, y, pcSource->Get(iLast - y, iTop + x));
			}
		}
	}
	pcSource->Kill();
	pcDest->Kill();
	return CropCel(OMalloc<CImageCel>(pTile));
}


//////////////////////////////////////////////////////////////////////////
//
// The cel keeps the full tile size but only its pixels that are not
// transparent are blitted, so an empty band, such as where a focus ring
// would be, costs nothing.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CButtonImages::CropCel(Ptr<CImageCel> pCel)
{
	pCel->CropTransparentBorders();
	return pCel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CButtonImages::GetCel(EButtonState eState, EButtonBorderPiece ePiece)
{
	return maCels.Get(eState * NUM_BUTTON_BORDER_PIECES + ePiece);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CButtonImages::GetFaceColour(EButtonState eState) { return masFaceColours[eState]; }
int32 CButtonImages::GetTileSize(void) { return miTileSize; }

