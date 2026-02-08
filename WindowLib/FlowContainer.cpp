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
	Ptr<CComponent>	pComponent;
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
	else if (meDirection == CSD_Down)
	{
		if (meWrap == CSW_Wrap)
		{
			LayoutDownWrap(sPosition, sAreaSize);
		}
		else
		{
			LayoutDown(sPosition, sAreaSize);
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
	Ptr<CComponent>	pComponent;
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
SInt2 CFlowContainer::CalculateAreaForHorizontalLayout(int iWidth)
{
	Ptr<CComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	int						fYPos;
	int						iFirst;
	SInt2					sLocalAreaSize;
	SInt2					sArea;

	fYPos = 0;
	iFirst = 0;
	sLocalAreaSize.Init(0, 0);
	sArea.Init(0, 0);
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if ((sLocalAreaSize.x + sSize.x) > iWidth)
		{
			sArea.y = sLocalAreaSize.y + fYPos;
			if (sLocalAreaSize.x > sArea.x)
			{
				sArea.x = sLocalAreaSize.x;
			}

			fYPos += sLocalAreaSize.y;
			sLocalAreaSize.Init(0, 0);
			iFirst = i;
		}

		if (sLocalAreaSize.y < sSize.y)
		{
			sLocalAreaSize.y = sSize.y;
		}

		sLocalAreaSize.x += sSize.x;
	}

	sArea.y = sLocalAreaSize.y + fYPos;
	if (sLocalAreaSize.x > sArea.x)
	{
		sArea.x = sLocalAreaSize.x;
	}

	return sArea;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutRightWrap(SInt2 sPosition, SInt2 sAreaSize)
{
	Ptr<CComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	int						iFirst;
	SInt2					sLocalAreaSize;
	SInt2					sLocalPosition;
	SInt2					sRequiredArea;

	sRequiredArea = CalculateAreaForHorizontalLayout(sAreaSize.x);

	iFirst = 0;
	sLocalAreaSize.Init(0, 0);
	sLocalPosition.Init(0, 0);
	sLocalPosition.y = AlignComponentVertical(muAlignment.eVertical, sRequiredArea.y, sAreaSize.y);

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if ((sLocalAreaSize.x + sSize.x) > sAreaSize.x)
		{
			LayoutRight(sLocalPosition, sLocalAreaSize, iFirst, i);
			sLocalPosition.y += sLocalAreaSize.y;
			sLocalAreaSize.Init(0, 0);
			iFirst = i;
		}

		if (sLocalAreaSize.y < sSize.y)
		{
			sLocalAreaSize.y = sSize.y;
		}

		sLocalAreaSize.x += sSize.x;
	}

	LayoutRight(sLocalPosition, sLocalAreaSize, iFirst, i);
} 




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutDown(SInt2 sPosition, SInt2 sAreaSize)
{
	size	uiSize;

	uiSize = maChildren.Size();
	LayoutDown(sPosition, sAreaSize, 0, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutDown(SInt2 sPosition, SInt2 sAreaSize, size iStart, size iEnd)
{
	Ptr<CComponent>	pComponent;
	size					i;
	SInt2					sSize;
	int						fYPos;
	int						fXPos;
	SInt2					sOffset;

	fYPos = 0;
	for (i = iStart; i < iEnd; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (muAlignment.eVertical == CSV_Stretch)
		{
			sSize.x = sAreaSize.x;
		}

		fXPos = AlignComponentHorizontal(muAlignment.eHorizontal, sSize.x, sAreaSize.x);
		sOffset.Init(fXPos + sPosition.x, fYPos + sPosition.y);
		fYPos += sSize.y;

		pComponent->Layout(sOffset, sSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt2 CFlowContainer::CalculateAreaForVerticalLayout(int iHeight)
{
	Ptr<CComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	int						fXPos;
	int						iFirst;
	SInt2					sLocalAreaSize;
	SInt2					sArea;

	fXPos = 0;
	iFirst = 0;
	sLocalAreaSize.Init(0, 0);
	sArea.Init(0, 0);
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if ((sLocalAreaSize.y + sSize.y) > iHeight)
		{
			sArea.x = sLocalAreaSize.x + fXPos;
			if (sLocalAreaSize.y > sArea.y)
			{
				sArea.y = sLocalAreaSize.y;
			}

			fXPos += sLocalAreaSize.x;
			sLocalAreaSize.Init(0, 0);
			iFirst = i;
		}

		if (sLocalAreaSize.x < sSize.x)
		{
			sLocalAreaSize.x = sSize.x;
		}

		sLocalAreaSize.y += sSize.y;
	}

	sArea.x = sLocalAreaSize.x + fXPos;
	if (sLocalAreaSize.y > sArea.y)
	{
		sArea.y = sLocalAreaSize.y;
	}

	return sArea;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutDownWrap(SInt2 sPosition, SInt2 sAreaSize)
{
	Ptr<CComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	int						iFirst;
	SInt2					sLocalAreaSize;
	SInt2					sLocalPosition;
	SInt2					sRequiredArea;

	sRequiredArea = CalculateAreaForVerticalLayout(sAreaSize.y);

	iFirst = 0;
	sLocalAreaSize.Init(0, 0);
	sLocalPosition.Init(0, 0);
	sLocalPosition.x = AlignComponentHorizontal(muAlignment.eHorizontal, sRequiredArea.x, sAreaSize.x);

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if ((sLocalAreaSize.y + sSize.y) > sAreaSize.y)
		{
			LayoutDown(sLocalPosition, sLocalAreaSize, iFirst, i);
			sLocalPosition.x += sLocalAreaSize.x;
			sLocalAreaSize.Init(0, 0);
			iFirst = i;
		}

		if (sLocalAreaSize.x < sSize.x)
		{
			sLocalAreaSize.x = sSize.x;
		}

		sLocalAreaSize.y += sSize.y;
	}

	LayoutDown(sLocalPosition, sLocalAreaSize, iFirst, i);
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

