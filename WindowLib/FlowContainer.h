#ifndef __FLOW_CONTAINER_H__
#define __FLOW_CONTAINER_H__
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


class CFlowContainer : public CContainer
{
public:
	EContainerStyleWrap			meWrap;
	EContainerStyleDirection	meDirection;
	UContainerAlignment			muAlignment;

	void	Init(Ptr<CWindow> pWindow);
	void	Free(void);

	void	SetRequiredSize(void) override;
	void	Layout(SInt2 sPosition, SInt2 sAreaSize) override;

	void	SetFlowStyle(EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleHorizontal eA);
	void	SetFlowStyle(EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleVertical eA);

protected:
	void	LayoutRight(SInt2 sPosition, SInt2 sAreaSize);
	void	LayoutRightWrap(SInt2 sPosition, SInt2 sAreaSize);
	void	LayoutRight(SInt2 sPosition, SInt2 sAreaSize, size iStart, size iEnd);
	SInt2	CalculateAreaForHorizontalLayout(int width);
};


#endif // __FLOW_CONTAINER_H__

