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
#include "StandardLib/Objects.h"
#include "RoundedCorners.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoundedCorners::Init(Ptr<CWindow> pWindow, ARGB32 sColour, int32 iRadius)
{
	Ptr<CCanvas>	pCorner;
	int				i;

	PreInit();

	CContainer::Init(pWindow);
	miRadius = iRadius;
	SetAlignment(CSH_Stretch, CSV_Stretch);
	maCorners.Init();
	for (i = 0; i < RC_NumCorners; i++)
	{
		pCorner = OMalloc<CCanvas>(pWindow, OMalloc<CRoundedCornerCanvasDraw>(sColour, (ERoundedCorner)i));
		pCorner->SetTransparent(true);
		pCorner->SetAlignment(CSH_Stretch, CSV_Stretch);
		maCorners.Add(pCorner);
		AddComponent(pCorner);
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoundedCorners::Class(void)
{
	CContainer::Class();

	M_Embedded(maCorners);
	U_Int32(miRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoundedCorners::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoundedCorners::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// It asks for no room: it goes over components that have it.
//
//////////////////////////////////////////////////////////////////////////
void CRoundedCorners::SetRequiredSize(void)
{
	msRequiredSize.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A canvas the radius across in each corner of the area.
//
//////////////////////////////////////////////////////////////////////////
void CRoundedCorners::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	int32		iRadius;
	int32		iRight;
	int32		iBottom;
	SInt32Vec2	sSize;

	SetActualBounds(sPosition, sAreaSize);
	SetContainerBounds(sPosition, sAreaSize);

	iRadius = miRadius;
	if (iRadius > sAreaSize.x / 2)
	{
		iRadius = sAreaSize.x / 2;
	}
	if (iRadius > sAreaSize.y / 2)
	{
		iRadius = sAreaSize.y / 2;
	}
	if (iRadius < 0)
	{
		iRadius = 0;
	}

	iRight = sPosition.x + sAreaSize.x - iRadius;
	iBottom = sPosition.y + sAreaSize.y - iRadius;
	sSize.Init(iRadius, iRadius);
	maCorners.Get(RC_TopLeft)->Layout(sPosition, sSize);
	maCorners.Get(RC_TopRight)->Layout(SInt32Vec2(iRight, sPosition.y), sSize);
	maCorners.Get(RC_BottomLeft)->Layout(SInt32Vec2(sPosition.x, iBottom), sSize);
	maCorners.Get(RC_BottomRight)->Layout(SInt32Vec2(iRight, iBottom), sSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The corners are only drawn, so the pointer finds what is under them.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CRoundedCorners::FindComponentAt(int x, int y)
{
	return NULL;
}


int32 CRoundedCorners::GetRadius(void) const { return miRadius; }
Ptr<CCanvas> CRoundedCorners::GetCorner(ERoundedCorner eCorner) { return maCorners.Get(eCorner); }

