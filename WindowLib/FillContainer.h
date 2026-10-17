#ifndef __FILL_CONTAINER_H__
#define __FILL_CONTAINER_H__
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
#include "ContainerStyles.h"
#include "Container.h"
#include "Insets.h"


//Overlay lays every component into the whole area (like a single GridBagLayout cell).
//Horizontal and Vertical split the area into equal slots, one per component (like SWT FillLayout).
enum EFillContainerSplit
{
	FCS_Overlay,
	FCS_Horizontal,
	FCS_Vertical,
};


class CFillContainer : public CContainer
{
CONSTRUCTABLE(CFillContainer);
DESTRUCTABLE(CFillContainer);
protected:
	EContainerStyleHorizontal	meHorizontal;
	EContainerStyleVertical		meVertical;
	EFillContainerSplit			meSplit;
	int32						miSpacing;
	SInsets						msInsets;
	SInt32Vec2					msRequiredSize;

public:
	void			Init(Ptr<CWindow> pWindow);
	void			Class(void) override;
	void			Free(void) override;

	void			SetRequiredSize(void) override;
	SInt32Vec2	GetRequiredSize(void);
	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

	void			SetFillStyle(EContainerStyleHorizontal eH, EContainerStyleVertical eV);
	void			SetSplit(EFillContainerSplit eSplit, int32 iSpacing);
	void			SetInsets(SInsets* psInsets);

protected:
	void			LayoutComponent(Ptr<CComponent> pComponent, SInt32Vec2 sSlotPosition, SInt32Vec2 sSlotSize);
};


#endif // __FILL_CONTAINER_H__

