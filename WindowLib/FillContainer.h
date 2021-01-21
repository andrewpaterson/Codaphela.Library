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
#ifndef __FILL_CONTAINER_H__
#define __FILL_CONTAINER_H__
#include "Container.h"
#include "Border.h"


enum EFillStyleHorizontal
{
	FSH_Stretch,
	FSH_Left,
	FSH_Center,
	FSH_Right,
};


enum EFillStyleVertical
{
	FSV_Stretch,
	FSV_Top,
	FSV_Center,
	FSV_Bottom,
};


class CFillContainer : public CContainer
{
public:
	CONSTRUCTABLE(CFillContainer);

	EFillStyleHorizontal	meHorizontal;
	EFillStyleVertical		meVertical;
	SInsets					msInsets;

	void 	Init(CViewport* pcViewport);
	void 	Kill(void);

	void 	Layout(SInt2 sPosition, SInt2 sAreaSize);
	void 	SetFillStyle(EFillStyleHorizontal eH, EFillStyleVertical eV);
	void 	SetInsets(SInsets* psInsets);
};


#endif // __FILL_CONTAINER_H__

