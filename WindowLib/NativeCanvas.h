#ifndef __NATIVE_CANVAS_H__
#define __NATIVE_CANVAS_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/Chars.h"
#include "BasicNativeComponent.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Rectangle.h"


class CNativeCanvas : public CBasicNativeComponent
{
CONSTRUCTABLE(CNativeCanvas);
friend class CCanvas;
protected:
	CCanvas*		mpcCanvas;

public:
			void	Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory);
			void	Kill(void);

protected:
	virtual bool	CreateNativeCanvas(void) =0;
	virtual uint8*	GetPixelData(void) =0;

	virtual void	CopyCanvas(CNativeCanvas* pcSourceCanvas) =0;

	virtual void	DrawBox(CRectangle* pcRect, bool bFilled, ARGB32 sColour) =0;
	virtual void	DrawPixel(int32 iX, int32 iY, ARGB32 sColour) =0;
};


#endif // __NATIVE_CANVAS_H__

