/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/Int2.h"
#include "FlowContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::Init(Ptr<CWindow> pWindow)
{
	CContainer::Init(pWindow);
	meWrap = CSW_Wrap;
	meDirection = CSD_Right;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::Free(void)
{
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetRequiredSize(void)
{
	Ptr<CBasicComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	SInt2					sTotalSize;

	sTotalSize.Init(0, 0);

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetBestSize();
		if (meDirection == CSD_Right)
		{
			sTotalSize.x += sSize.x;
			if (sTotalSize.y < sSize.y)
			{
				sTotalSize.y = sSize.y;
			}
		}
		else if (meDirection == CSD_Down)
		{
			sTotalSize.y += sSize.y;
			if (sTotalSize.x < sSize.x)
			{
				sTotalSize.x = sSize.x;
			}
		}
	}
	msRequiredSize = sTotalSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	Ptr<CBasicComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	int						fXPos;
	int						fYPos;
	SInt2					sOffset;

	SetBounds(sPosition, sAreaSize);

	fXPos = 0;
	fYPos = 0;
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetBestSize();
		if (meDirection == CSD_Right)
		{
			if ((meWrap == CSW_Wrap) && ((fXPos + sSize.x) > sAreaSize.x))
			{
				fXPos = 0;
				fYPos += sSize.y;
			}
			sOffset.Init(fXPos + sPosition.x, fYPos + sPosition.y);
			fXPos += sSize.x;
		}
		else if (meDirection == CSD_Down)
		{
			if ((meWrap == CSW_Wrap) && ((fYPos + sSize.y) > sAreaSize.y))
			{
				fYPos = 0;
				fXPos += sSize.x;
			}
			sOffset.Init(fXPos + sPosition.x, fYPos + sPosition.y);
			fYPos += sSize.y;
		}
		
		pComponent->Layout(sOffset, sSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetFlowStyle(EContainerStyleDirection eD, EContainerStyleWrap eW)
{
	meWrap = eW;
	meDirection = eD;
}

