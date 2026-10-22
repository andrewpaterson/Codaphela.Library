#ifndef __BUTTON_BAR_H__
#define __BUTTON_BAR_H__
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
#include "BaseLib/ArrayInt.h"
#include "Insets.h"
#include "Container.h"


//A row of buttons, like the OK and Cancel along the bottom of a dialog.  The buttons keep their desired sizes and are
//laid out left to right in the order they were added with the gap between each, inside the insets.  The row is placed
//by the bar's button alignment (right by default) and each button sits in the row's height by its own alignment.
//
//Like a button, a bar keeps the size its buttons need unless a desired size is set on an axis.  Stretch the bar
//horizontally so that it gets the width its buttons are aligned in.
class CButtonBar : public CContainer
{
CONSTRUCTABLE(CButtonBar);
DESTRUCTABLE(CButtonBar);
protected:
	SInsets						msInsets;
	int32						miGap;  //Between neighbouring buttons.
	EContainerStyleHorizontal	meButtonAlignment;  //Where the row of buttons sits; stretch is treated as left.

public:
	void						Init(Ptr<CWindow> pWindow);
	void						Class(void) override;
	void 						Free(void) override;

	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	void						SetInsets(SInsets* psInsets);
	void						SetInsets(int iTop, int iRight, int iBottom, int iLeft);
	void						SetGap(int32 iGap);
	int32						GetGap(void);
	void						SetButtonAlignment(EContainerStyleHorizontal eAlignment);
	EContainerStyleHorizontal	GetButtonAlignment(void);

	SInt32Vec2					GetDesiredSize(void) override;
	void						SetRequiredSize(void) override;
	void						Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

protected:
	SInt32Vec2					GetRowSize(CArrayInt* paiWidths);
};


#endif // __BUTTON_BAR_H__

