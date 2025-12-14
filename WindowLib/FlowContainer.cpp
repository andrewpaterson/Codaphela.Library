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
	muAlignment.eVertical = CSV_Top;
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

	//This works out how much space the component needs, not where it needs that space.

	sTotalSize.Init(0, 0);

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if ((meDirection == CSD_Right) || (meDirection == CSD_Left))
		{
			sTotalSize.x += sSize.x;
			if (sTotalSize.y < sSize.y)
			{
				sTotalSize.y = sSize.y;
			}
		}
		else if ((meDirection == CSD_Down) || (meDirection == CSD_Up))
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
	SetBounds(sPosition, sAreaSize);

	if (meDirection == CSD_Right)
	{
		if (meWrap == CSW_Wrap)
		{
			LayoutRightWrap(sPosition, sAreaSize);
		}
		else
		{
			LayoutRight(sPosition, sAreaSize);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutRight(SInt2 sPosition, SInt2 sAreaSize)
{
	size	uiSize;

	uiSize = maChildren.Size();
	LayoutRight(sPosition, sAreaSize, 0, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutRight(SInt2 sPosition, SInt2 sAreaSize, size iStart, size iEnd)
{
	Ptr<CBasicComponent>	pComponent;
	size					i;
	SInt2					sSize;
	int						fXPos;
	int						fYPos;
	SInt2					sOffset;

	fXPos = 0;
	for (i = iStart; i < iEnd; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (muAlignment.eVertical == CSV_Stretch)
		{
			sSize.y = sAreaSize.y;
		}

		fYPos = AlignComponentVertical(muAlignment.eVertical, sSize.y, sAreaSize.y);
		sOffset.Init(fXPos + sPosition.x, fYPos + sPosition.y);
		fXPos += sSize.x;

		pComponent->Layout(sOffset, sSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutRightWrap(SInt2 sPosition, SInt2 sAreaSize)
{
	Ptr<CBasicComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	int						fXPos;
	int						fYPos;
	SInt2					sOffset;
	int						iCount;
	int						iFirst;
	SInt2					sLocalAreaSize;
	SInt2					sLocalPosition;

	fXPos = 0;
	fYPos = 0;
	iCount = 0;
	iFirst = 0;
	sLocalAreaSize.Init(0, 0);
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if ((fXPos + sSize.x) > sAreaSize.x)
		{
			sLocalAreaSize.x = fXPos;
			LayoutRight(sLocalPosition, sLocalAreaSize, iFirst, i);
			fXPos = 0;
			fYPos += sSize.y;
			iCount = 0;
		}

		if (sLocalAreaSize.y < sSize.y)
		{
			sLocalAreaSize.y = sSize.y;
		}

		iCount++;
		sOffset.Init(fXPos + sPosition.x, fYPos + sPosition.y);
		fXPos += sSize.x;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetFlowStyle(EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleHorizontal eA)
{
	meWrap = eW;
	meDirection = eD;
	muAlignment.eHorizontal = eA;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetFlowStyle(EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleVertical eA)
{
	meWrap = eW;
	meDirection = eD;
	muAlignment.eVertical = eA;
}

