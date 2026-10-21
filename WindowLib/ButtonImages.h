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
#ifndef __BUTTON_IMAGES_H__
#define __BUTTON_IMAGES_H__
#include "StandardLib/Array.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/ColourARGB32.h"


enum EButtonState
{
	BS_Normal,
	BS_MouseOver,
	BS_Selected,  //Has focus.
	BS_Pushed,

	NUM_BUTTON_STATES
};


//The border around a button's face, in the order CBorderLayoutHelper lays them out without the centre.
enum EButtonBorderPiece
{
	BBP_TopLeft,
	BBP_Top,
	BBP_TopRight,
	BBP_Left,
	BBP_Right,
	BBP_BottomLeft,
	BBP_Bottom,
	BBP_BottomRight,

	NUM_BUTTON_BORDER_PIECES
};


//The border pieces of a button in each of its states and the colour of its face.  Buttons can share them.
//
//Each state's border comes from a strip of eight square tiles, as wide as a tile, from the top down: the top, right,
//bottom and left edges then the top left, top right, bottom right and bottom left corners.  Every edge has its outside
//along its top so the right, bottom and left edges are turned clockwise by one, two and three quarters.
//
//The face can be one colour for every state or a colour for each.
//
//Every cel is a whole tile in size but its transparent borders are cropped, so a band left empty in some states, such
//as a focus ring's, reserves its space without being blitted.
class CButtonImages : public CObject
{
CONSTRUCTABLE(CButtonImages);
DESTRUCTABLE(CButtonImages);
protected:
	CArray<CImageCel>	maCels;  //NUM_BUTTON_BORDER_PIECES for each state in turn.
	ARGB32				masFaceColours[NUM_BUTTON_STATES];
	int32				miTileSize;

public:
	void			Init(Ptr<CImage> pNormal, Ptr<CImage> pMouseOver, Ptr<CImage> pSelected, Ptr<CImage> pPushed, ARGB32 sFaceColour);
	void			Init(Ptr<CImage> pNormal, Ptr<CImage> pMouseOver, Ptr<CImage> pSelected, Ptr<CImage> pPushed, ARGB32 sNormalFace, ARGB32 sMouseOverFace, ARGB32 sSelectedFace, ARGB32 sPushedFace);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	Ptr<CImageCel>	GetCel(EButtonState eState, EButtonBorderPiece ePiece);
	ARGB32			GetFaceColour(EButtonState eState);
	int32			GetTileSize(void);

protected:
	void			AddStrip(Ptr<CImage> pStrip);
	Ptr<CImageCel>	CreateCel(Ptr<CImage> pStrip, int iTile, int iQuarterTurns);
	Ptr<CImageCel>	CropCel(Ptr<CImageCel> pCel);
};


#endif // __BUTTON_IMAGES_H__

