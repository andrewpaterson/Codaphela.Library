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
	meHorizontal = CSH_Left;
	meVertical = CSV_Top;
	msInsets.Init();
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
	U_Enum(meHorizontal);
	U_Enum(meVertical);
	U_Data(SInsets, msInsets);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFlowContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFlowContainer::Load(CObjectReader* pcFile)
{
	return false;
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
	int32				iMainTotal;
	int32				iCrossTotal;
	int32				iCrossSize;

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

		// Main-axis accumulation
		iMainTotal = ContainerStyleMainAxis(&sTotalSize, bHorizontal);
		iMainTotal += ContainerStyleMainAxis(&sSize, bHorizontal);
		if (bHorizontal)
		{
			sTotalSize.x = iMainTotal;
		}
		else
		{
			sTotalSize.y = iMainTotal;
		}

		// Cross-axis maximum
		iCrossTotal = ContainerStyleCrossAxis(&sTotalSize, bHorizontal);
		iCrossSize = ContainerStyleCrossAxis(&sSize, bHorizontal);
		if (iCrossTotal < iCrossSize)
		{
			if (bHorizontal)
			{
				sTotalSize.y = iCrossSize;
			}
			else
			{
				sTotalSize.x = iCrossSize;
			}
		}
	}
	sTotalSize.x += msInsets.GetLeftAndRight();
	sTotalSize.y += msInsets.GetTopAndBottom();
	msRequiredSize = sTotalSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sOrigin;
	SInt32Vec2	sArea;

	sOrigin = sPosition;
	sArea = sAreaSize;
	msInsets.Inset(&sOrigin, &sArea);

	SetContainerBounds(sOrigin, sArea);

	if (meDirection != CSD_Unknown)
	{
		if (meWrap == CSW_Wrap)
		{
			LayoutWrap(sOrigin, sArea);
		}
		else
		{
			LayoutLine(sOrigin, sArea, 0, maChildren.Size());
		}
	}

	SetActualBounds(sPosition, sAreaSize);
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
int CFlowContainer::AlignCross(int iComponentSize, int iAvailableSize)
{
	if (IsHorizontal())
	{
		return AlignComponentVertical(meVertical, iComponentSize, iAvailableSize);
	}
	else
	{
		return AlignComponentHorizontal(meHorizontal, iComponentSize, iAvailableSize);
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
	SInt32Vec2			sAreaPosition;
	SInt32Vec2			sArea;
	int					iMainPos;
	int					iMainSize;
	int					iMainOffset;
	bool				bHorizontal;
	bool				bReversed;

	//Each component gets an area as long as its desired main axis size and as thick as the line.  Its own alignment
	//places it across the line, or stretches it to the line's thickness.
	bHorizontal = IsHorizontal();
	bReversed = IsReversed();
	iMainPos = 0;

	for (i = iStart; i < iEnd; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		iMainSize = ContainerStyleMainAxis(&sSize, bHorizontal);
		if (iMainSize < 0)
		{
			iMainSize = 0;
		}

		// Main-axis offset (reversed or normal)
		if (bReversed)
		{
			iMainOffset = ContainerStyleMainAxis(&sLineSize, bHorizontal) - iMainPos - iMainSize;
		}
		else
		{
			iMainOffset = iMainPos;
		}

		if (bHorizontal)
		{
			sAreaPosition.Init(sPosition.x + iMainOffset, sPosition.y);
			sArea.Init(iMainSize, sLineSize.y);
		}
		else
		{
			sAreaPosition.Init(sPosition.x, sPosition.y + iMainOffset);
			sArea.Init(sLineSize.x, iMainSize);
		}

		iMainPos += iMainSize;

		LayoutComponentInArea(pComponent, sAreaPosition, sArea);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFlowContainer::FindLineEnd(size iFirst, int iMainLength, int* piLineLength, int* piLineThickness)
{
	Ptr<CComponent>		pComponent;
	size				uiSize;
	size				i;
	SInt32Vec2			sSize;
	int					iLineLength;
	int					iLineThickness;
	bool				bHorizontal;

	// A line takes components until the next would make it longer than iMainLength,
	// but always takes at least one.
	// Returns the index of the first component on the next line.
	bHorizontal = IsHorizontal();
	iLineLength = 0;
	iLineThickness = 0;
	uiSize = maChildren.Size();

	for (i = iFirst; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();

		if (((iLineLength + ContainerStyleMainAxis(&sSize, bHorizontal)) > iMainLength) && (i > iFirst))
		{
			break;
		}

		if (iLineThickness < ContainerStyleCrossAxis(&sSize, bHorizontal))
		{
			iLineThickness = ContainerStyleCrossAxis(&sSize, bHorizontal);
		}

		iLineLength += ContainerStyleMainAxis(&sSize, bHorizontal);
	}

	*piLineLength = iLineLength;
	*piLineThickness = iLineThickness;
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CFlowContainer::CalculateWrappedArea(int iMainLength)
{
	size		uiSize;
	size		iFirst;
	SInt32Vec2	sArea;
	int			iLineLength;
	int			iLineThickness;
	int			iLongestLine;
	int			iCrossPos;
	bool		bHorizontal;

	bHorizontal = IsHorizontal();
	iLongestLine = 0;
	iCrossPos = 0;
	uiSize = maChildren.Size();
	iFirst = 0;

	while (iFirst < uiSize)
	{
		iFirst = FindLineEnd(iFirst, iMainLength, &iLineLength, &iLineThickness);
		if (iLineLength > iLongestLine)
		{
			iLongestLine = iLineLength;
		}
		iCrossPos += iLineThickness;
	}

	// Write the final results (helpers are now read-only)
	if (bHorizontal)
	{
		sArea.x = iLongestLine;
		sArea.y = iCrossPos;
	}
	else
	{
		sArea.x = iCrossPos;
		sArea.y = iLongestLine;
	}

	return sArea;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::LayoutWrap(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size		uiSize;
	size		iFirst;
	size		iEnd;
	SInt32Vec2	sLinePosition;
	SInt32Vec2	sLineSize;
	SInt32Vec2	sRequiredArea;
	int			iLineLength;
	int			iLineThickness;
	bool		bHorizontal;
	int32		iCrossPos;
	int32		iMainLen;

	bHorizontal = IsHorizontal();

	// CalculateWrappedArea takes the main-axis length
	sRequiredArea = CalculateWrappedArea(ContainerStyleMainAxis(&sAreaSize, bHorizontal));

	sLinePosition = sPosition;

	// Align the starting cross-axis position
	iCrossPos = ContainerStyleCrossAxis(&sLinePosition, bHorizontal) + AlignCross(ContainerStyleCrossAxis(&sRequiredArea, bHorizontal), ContainerStyleCrossAxis(&sAreaSize, bHorizontal));
	if (bHorizontal)
	{
		sLinePosition.y = iCrossPos;
	}
	else
	{
		sLinePosition.x = iCrossPos;
	}

	iMainLen = ContainerStyleMainAxis(&sAreaSize, bHorizontal);
	if (bHorizontal)
	{
		sLineSize.x = iMainLen;
	}
	else
	{
		sLineSize.y = iMainLen;
	}

	uiSize = maChildren.Size();
	iFirst = 0;

	while (iFirst < uiSize)
	{
		iEnd = FindLineEnd(iFirst, ContainerStyleMainAxis(&sAreaSize, bHorizontal), &iLineLength, &iLineThickness);

		// Set the line's cross-axis thickness
		if (bHorizontal)
		{
			sLineSize.y = iLineThickness;
		}
		else
		{
			sLineSize.x = iLineThickness;
		}

		LayoutLine(sLinePosition, sLineSize, iFirst, iEnd);

		// Advance the line position on the cross axis
		iCrossPos = ContainerStyleCrossAxis(&sLinePosition, bHorizontal) + iLineThickness;
		if (bHorizontal)
		{
			sLinePosition.y = iCrossPos;
		}
		else
		{
			sLinePosition.x = iCrossPos;
		}

		iFirst = iEnd;
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
	meHorizontal = eA;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetFlowStyle(EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleVertical eA)
{
	meWrap = eW;
	meDirection = eD;
	meVertical = eA;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlowContainer::SetInsets(SInsets* psInsets)
{
	memcpy(&msInsets, psInsets, sizeof(SInsets));
}

