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
#include "BaseLib/Int32Vec2.h"
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
void CFlowContainer::Class(void)
{
	CContainer::Class();
	U_Enum(meWrap);
	U_Enum(meDirection);
	U_Data(UContainerAlignment, muAlignment);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetRequiredSize(void)
{
	Ptr<CComponent>		pComponent;
	size				uiSize;
	size				i;
	SInt32Vec2			sSize;
	SInt32Vec2			sTotalSize;
	bool				bHorizontal;

	//This works out how much space the components need on a single unwrapped line, not where they need that space.
	//Components without a desired size (-1) don't add to the required size.

	bHorizontal = IsHorizontal();
	sTotalSize.Init(0, 0);

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (sSize.x < 0)
		{
			sSize.x = 0;
		}
		if (sSize.y < 0)
		{
			sSize.y = 0;
		}

		MainAxis(sTotalSize, bHorizontal) += MainAxis(sSize, bHorizontal);
		if (CrossAxis(sTotalSize, bHorizontal) < CrossAxis(sSize, bHorizontal))
		{
			CrossAxis(sTotalSize, bHorizontal) = CrossAxis(sSize, bHorizontal);
		}
	}
	msRequiredSize = sTotalSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SetContainerBounds(sPosition, sAreaSize);

	if (meDirection == CSD_Unknown)
	{
		return;
	}

	if (meWrap == CSW_Wrap)
	{
		LayoutWrap(sPosition, sAreaSize);
	}
	else
	{
		LayoutLine(sPosition, sAreaSize, 0, maChildren.Size());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFlowContainer::IsHorizontal(void)
{
	return (meDirection == CSD_Right) || (meDirection == CSD_Left);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFlowContainer::IsReversed(void)
{
	return (meDirection == CSD_Left) || (meDirection == CSD_Up);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFlowContainer::IsCrossStretch(void)
{
	if (IsHorizontal())
	{
		return muAlignment.eVertical == CSV_Stretch;
	}
	else
	{
		return muAlignment.eHorizontal == CSH_Stretch;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFlowContainer::AlignCross(int iComponentSize, int iAvailableSize)
{
	if (IsHorizontal())
	{
		return AlignComponentVertical(muAlignment.eVertical, iComponentSize, iAvailableSize);
	}
	else
	{
		return AlignComponentHorizontal(muAlignment.eHorizontal, iComponentSize, iAvailableSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutLine(SInt32Vec2 sPosition, SInt32Vec2 sLineSize, size iStart, size iEnd)
{
	Ptr<CComponent>		pComponent;
	size				i;
	SInt32Vec2			sSize;
	SInt32Vec2			sOffset;
	int					iMainPos;
	int					iMainOffset;
	bool				bHorizontal;
	bool				bReversed;
	bool				bStretch;

	bHorizontal = IsHorizontal();
	bReversed = IsReversed();
	bStretch = IsCrossStretch();

	iMainPos = 0;
	for (i = iStart; i < iEnd; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (bStretch)
		{
			CrossAxis(sSize, bHorizontal) = CrossAxis(sLineSize, bHorizontal);
		}

		if (bReversed)
		{
			iMainOffset = MainAxis(sLineSize, bHorizontal) - iMainPos - MainAxis(sSize, bHorizontal);
		}
		else
		{
			iMainOffset = iMainPos;
		}

		MainAxis(sOffset, bHorizontal) = MainAxis(sPosition, bHorizontal) + iMainOffset;
		CrossAxis(sOffset, bHorizontal) = CrossAxis(sPosition, bHorizontal) + AlignCross(CrossAxis(sSize, bHorizontal), CrossAxis(sLineSize, bHorizontal));
		iMainPos += MainAxis(sSize, bHorizontal);

		pComponent->Layout(sOffset, sSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CFlowContainer::CalculateWrappedArea(int iMainLength)
{
	Ptr<CComponent>		pComponent;
	size				uiSize;
	size				i;
	size				iFirst;
	SInt32Vec2			sSize;
	SInt32Vec2			sArea;
	int					iLineLength;
	int					iLineThickness;
	int					iLongestLine;
	int					iCrossPos;
	bool				bHorizontal;

	bHorizontal = IsHorizontal();

	iFirst = 0;
	iLineLength = 0;
	iLineThickness = 0;
	iLongestLine = 0;
	iCrossPos = 0;
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (((iLineLength + MainAxis(sSize, bHorizontal)) > iMainLength) && (i > iFirst))
		{
			if (iLineLength > iLongestLine)
			{
				iLongestLine = iLineLength;
			}

			iCrossPos += iLineThickness;
			iLineLength = 0;
			iLineThickness = 0;
			iFirst = i;
		}

		if (iLineThickness < CrossAxis(sSize, bHorizontal))
		{
			iLineThickness = CrossAxis(sSize, bHorizontal);
		}

		iLineLength += MainAxis(sSize, bHorizontal);
	}

	if (iLineLength > iLongestLine)
	{
		iLongestLine = iLineLength;
	}

	MainAxis(sArea, bHorizontal) = iLongestLine;
	CrossAxis(sArea, bHorizontal) = iCrossPos + iLineThickness;
	return sArea;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutWrap(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	Ptr<CComponent>		pComponent;
	size				uiSize;
	size				i;
	size				iFirst;
	SInt32Vec2			sSize;
	SInt32Vec2			sLinePosition;
	SInt32Vec2			sLineSize;
	SInt32Vec2			sRequiredArea;
	int					iLineLength;
	int					iLineThickness;
	bool				bHorizontal;

	bHorizontal = IsHorizontal();
	sRequiredArea = CalculateWrappedArea(MainAxis(sAreaSize, bHorizontal));

	sLinePosition = sPosition;
	CrossAxis(sLinePosition, bHorizontal) += AlignCross(CrossAxis(sRequiredArea, bHorizontal), CrossAxis(sAreaSize, bHorizontal));

	//Each line spans the whole main axis so that reversed directions start from the far edge.
	MainAxis(sLineSize, bHorizontal) = MainAxis(sAreaSize, bHorizontal);

	iFirst = 0;
	iLineLength = 0;
	iLineThickness = 0;
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (((iLineLength + MainAxis(sSize, bHorizontal)) > MainAxis(sAreaSize, bHorizontal)) && (i > iFirst))
		{
			CrossAxis(sLineSize, bHorizontal) = iLineThickness;
			LayoutLine(sLinePosition, sLineSize, iFirst, i);

			CrossAxis(sLinePosition, bHorizontal) += iLineThickness;
			iLineLength = 0;
			iLineThickness = 0;
			iFirst = i;
		}

		if (iLineThickness < CrossAxis(sSize, bHorizontal))
		{
			iLineThickness = CrossAxis(sSize, bHorizontal);
		}

		iLineLength += MainAxis(sSize, bHorizontal);
	}

	CrossAxis(sLineSize, bHorizontal) = iLineThickness;
	LayoutLine(sLinePosition, sLineSize, iFirst, uiSize);
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

