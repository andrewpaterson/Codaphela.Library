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
#include "StandardLib/ClassDefines.h"
#include "Container.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::Init(Ptr<CWindow> pWindow)
{
	PreInit();

	CComponent::Init(pWindow);
	mcContainerBounds.Init(-1, -1, -1, -1);
	msRequiredSize.Init(-1, -1);
	mbClipsChildren = true;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::Free(void)
{
	CComponent::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::Class(void)
{
	CComponent::Class();
	U_Data(CInt32Rect, mcContainerBounds);
	U_2Int32(msRequiredSize);
	U_Bool(mbClipsChildren);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CContainer::GetContainerBounds(CInt32Rect* pcDest)
{
	pcDest->Init(&mcContainerBounds);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::SetContainerBounds(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	CInt32Rect	cParentBounds;

	//The container's bounds are where it was laid out, but never outside its parent container's bounds.
	mcContainerBounds.Init(sPosition.x, sPosition.y, sPosition.x + sAreaSize.x, sPosition.y + sAreaSize.y);
	if (mpParent.IsNotNull())
	{
		if (mpParent->GetContainerBounds(&cParentBounds))
		{
			mcContainerBounds.ClipTo(&cParentBounds);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A component that overflows the area it was laid out in, like a fixed
// size component in a shrunk grid cell, is cut off at the container's
// edge rather than drawn over whatever is next to the container.
//
//////////////////////////////////////////////////////////////////////////
bool CContainer::ClipsChildren(void)
{
	return mbClipsChildren;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::SetClipsChildren(bool bClipsChildren)
{
	mbClipsChildren = bClipsChildren;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	// This intentionally does not call CComponent::Layout 
	// because a container must fully implement it's own Layout.
}


//////////////////////////////////////////////////////////////////////////
//
// For a container that keeps the size its components need unless a
// desired size is set on an axis, like a button.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CContainer::GetDesiredOrRequiredSize(void)
{
	SInt32Vec2	sSize;

	sSize = msDesiredSize;
	if ((sSize.x < 0) || (sSize.y < 0))
	{
		SetRequiredSize();
		if (sSize.x < 0)
		{
			sSize.x = msRequiredSize.x;
		}
		if (sSize.y < 0)
		{
			sSize.y = msRequiredSize.y;
		}
	}
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::SetActualBounds(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	//Called once the container has laid out its components.  Like any component its actual bounds are the area it was
	//given by its parent, insets included and not clipped.  The inset and clipped area is mcContainerBounds.
	SetPosition(sPosition);
	SetActualSize(sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CContainer::GetRequiredSize(void)
{
	return msRequiredSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::LayoutComponentInArea(Ptr<CComponent> pComponent, SInt32Vec2 sAreaPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2					sSize;
	SInt32Vec2					sPosition;
	EContainerStyleHorizontal	eHorizontal;
	EContainerStyleVertical		eVertical;

	//A component with a desired size of -1 on an axis is variable sized and takes the whole area on that axis, as does a
	//component stretched on that axis.  Otherwise it keeps its desired size and is placed by its own alignment.
	sSize = pComponent->GetDesiredSize();
	eHorizontal = pComponent->GetHorizontalAlignment();
	eVertical = pComponent->GetVerticalAlignment();
	if ((sSize.x < 0) || (eHorizontal == CSH_Stretch))
	{
		sSize.x = sAreaSize.x;
	}
	if ((sSize.y < 0) || (eVertical == CSV_Stretch))
	{
		sSize.y = sAreaSize.y;
	}

	sPosition.x = sAreaPosition.x + AlignComponentHorizontal(eHorizontal, sSize.x, sAreaSize.x);
	sPosition.y = sAreaPosition.y + AlignComponentVertical(eVertical, sSize.y, sAreaSize.y);

	pComponent->Layout(sPosition, sSize);
}

