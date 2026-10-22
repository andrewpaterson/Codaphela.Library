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
#include "Component.h"
#include "FillContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Init(Ptr<CWindow> pWindow)
{
	CContainer::Init(pWindow);
	msInsets.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Free(void)
{
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Class(void)
{
	CContainer::Class();
	U_Data(SInsets, msInsets);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFillContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFillContainer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetRequiredSize(void)
{
	size				i;
	Ptr<CComponent>		pComponent;
	size				uiSize;
	SInt32Vec2			sSize;
	SInt32Vec2			sLargest;

	//The components are on top of each other so the largest of them on each axis decides the size.
	sLargest.Init(0, 0);

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		sLargest.Maximise(sSize);
	}

	sLargest.x += msInsets.GetLeftAndRight();
	sLargest.y += msInsets.GetTopAndBottom();
	msRequiredSize = sLargest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size				i;
	SInt32Vec2			sOrigin;
	SInt32Vec2			sArea;
	Ptr<CComponent>		pComponent;
	size				uiSize;

	sOrigin = sPosition;
	sArea = sAreaSize;
	msInsets.Inset(&sOrigin, &sArea);

	SetContainerBounds(sOrigin, sArea);

	//Every component is laid out into the same area.
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		LayoutComponentInArea(pComponent, sOrigin, sArea);
	}

	SetActualBounds(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetInsets(SInsets* psInsets)
{
	memcpy(&msInsets, psInsets, sizeof(SInsets));
}

