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
#ifndef __WIDGET_H__
#define __WIDGET_H__
#include "Component.h"
#include "Block.h"
#include "Border.h"


class CWidget : public CComponent
{
public:
	BASE_FUNCTIONS(CWidget);

	CBlock*		mpcBackground;
	CBorder*	mpcBorder;

	void 		Init(CViewport* pcViewport, BOOL bDefaultBorder = TRUE, BOOL bDefaultBackground = TRUE);
	void 		Kill(void);
	BOOL 		Draw(void);
	void		Layout(SInt2 sPosition, SInt2 sAreaSize);

	CBorder*	GetBorder(void);
	CBlock*		GetBackground(void);
	void		GetUsableArea(SInt2* psPosition, SInt2* psSize);
	SInsets		GetBorderWidths(void);
};


#endif // __WIDGET_H__

