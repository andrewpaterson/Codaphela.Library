/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __DRAWABLE_H__
#define __DRAWABLE_H__
#include "WorldLib/GraphicsViewport.h"
#include "Component.h"


class CDrawable : public CComponent
{
public:
	SInt2				msOldSize;
	SInt2	 			msOldPosition;
	CViewportInstance*	mpcGraphicsViewport;

	void 	Init(CViewport* pcViewport);
	void 	Kill(void);
	BOOL 	Draw(void);

	void 	SetActualSize(int fWidth, int fHeight);
	void	SetActualSize(SInt2 sSize);
	void 	SetPosition(int x, int y);
	void	SetPosition(SInt2 sPosition);
	void 	GetPosition(SInt2* psPosition);
	BOOL 	HasSizeChanged(void);
	BOOL 	HasPositionChanged(void);
};


#endif // __DRAWABLE_H__

