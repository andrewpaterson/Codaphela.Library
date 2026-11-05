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
#ifndef __IMAGE_BORDER_CONTAINER_H__
#define __IMAGE_BORDER_CONTAINER_H__
#include "GridContainer.h"
#include "FillContainer.h"
#include "ImageCanvasDraw.h"
#include "ClearCanvasDraw.h"
#include "ButtonImages.h"


//A face inside a border of image pieces, drawn from CButtonImages in one of its states; what a button, a text field and
//a combo box are made of.  It is a grid of the eight border pieces, laid out by CBorderLayoutHelper, around a
//fill container: the face.  The face is filled with the state's face colour and whatever is added to it is drawn over
//that.
//
//What goes on the face is added to the padded face, the face less its padding.  The face's desired size is the
//content's size (GetContentSize) with the padding round it; call UpdateFaceSize when the content's size changes.
//
//Like a button it is hit as a whole, so it and not its parts has the pointer over it and takes focus, and it keeps the
//size its parts need unless a desired size is set on an axis.
class CImageBorderContainer : public CGridContainer
{
CONSTRUCTABLE(CImageBorderContainer);
DESTRUCTABLE(CImageBorderContainer);
protected:
	Ptr<CButtonImages>			mpImages;
	CArray<CImageCanvasDraw>	maBorderDraws;  //One for each EButtonBorderPiece.
	Ptr<CClearCanvasDraw>		mpFaceDraw;
	Ptr<CFillContainer>			mpFace;
	Ptr<CFillContainer>			mpPadded;  //The face less its padding.
	SInt32Vec2					msPadding;  //On each side of the content, horizontally and vertically.
	EButtonState				meBorderState;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages, int32 iHorizontalPadding, int32 iVerticalPadding);
	void				Class(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CFillContainer>	GetFace(void);
	Ptr<CFillContainer>	GetPaddedFace(void);
	void				SetPadding(int32 iHorizontal, int32 iVertical);
	SInt32Vec2			GetPadding(void);
	void				SetImages(Ptr<CButtonImages> pImages);
	Ptr<CButtonImages>	GetImages(void);
	void				SetBorderState(EButtonState eState);
	EButtonState		GetBorderState(void);


protected:
	void				UpdateBorder(void);
	void				SetPaddedInsets(int32 iHorizontal, int32 iVertical);
	void				UpdateFaceSize(void);
	virtual SInt32Vec2	GetContentSize(void);
};


#endif // __IMAGE_BORDER_CONTAINER_H__

