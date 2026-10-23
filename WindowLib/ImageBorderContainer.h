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
	EButtonState				meBorderState;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CFillContainer>	GetFace(void);
	void				SetImages(Ptr<CButtonImages> pImages);
	Ptr<CButtonImages>	GetImages(void);
	void				SetBorderState(EButtonState eState);
	EButtonState		GetBorderState(void);

	SInt32Vec2			GetDesiredSize(void) override;
	Ptr<CComponent>		FindComponentAt(int x, int y) override;

protected:
	void				UpdateBorder(void);
};


#endif // __IMAGE_BORDER_CONTAINER_H__

