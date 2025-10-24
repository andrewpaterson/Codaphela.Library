#ifndef __CANVAS_H__
#define __CANVAS_H__
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
#include "StandardLib/Unknown.h"
#include "BasicComponent.h"


class CNativeCanvas;
class CCanvas : public CBasicComponent
{
CONSTRUCTABLE(CCanvas);
protected:
	CNativeCanvas*	mpcNativeCanvas;
	EColourFormat	meFormat;
	int32			miWidth;
	int32			miHeight;

public:
	void			Init(CNativeWindowFactory* pcWindowFactory);
	void			Init(EColourFormat eFormat, int32 iWidth, int32 iHeight, CNativeWindowFactory* pcWindowFactory);
	void			Kill(void);

	EColourFormat	GetFormat(void);
	int32			GetWidth(void);
	int32			GetHeight(void);

	uint8*			GetPixelData(void);

	CNativeCanvas*	GetNativeCanvas(void);
};


#endif // __CANVAS_H__

