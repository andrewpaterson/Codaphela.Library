#ifndef __ICON_COMPONENT_H__
#define __ICON_COMPONENT_H__
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
#include "Canvas.h"
#include "ImageCanvasDraw.h"


//A transparent canvas showing one image cel, an icon say.  It can be given a fixed width or height, wider or taller
//than its cel, so that icons of different sizes, or no icon at all, take the same space and what is next to them lines
//up.  The cel is centred in the canvas unless given another alignment.  An axis without a fixed size is the cel's size,
//or nothing without a cel.
class CIconComponent : public CCanvas
{
CONSTRUCTABLE(CIconComponent);
DESTRUCTABLE(CIconComponent);
protected:
	Ptr<CImageCanvasDraw>	mpCelDraw;

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CImageCel> pCel, int32 iWidth = -1, int32 iHeight = -1);
	void			Class(void) override;
	void 			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetCel(Ptr<CImageCel> pCel);
	Ptr<CImageCel>	GetCel(void);
	void			SetCelAlignment(EContainerStyleHorizontal eHorizontal, EContainerStyleVertical eVertical);

	SInt32Vec2		GetDesiredSize(void) override;
};


#endif // __ICON_COMPONENT_H__

