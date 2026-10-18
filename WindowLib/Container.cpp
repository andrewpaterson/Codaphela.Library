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
	mcBounds.Init();
	mcBounds.SetPos(-1, -1);
	mcBounds.SetSize(-1, -1);
	msRequiredSize.Init(-1, -1);

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
	U_Data(CRectangle, mcBounds);
	U_2Int32(msRequiredSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CContainer::GetContainerBounds(CRectangle* pcDest)
{
	pcDest->Init(&mcBounds);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::SetContainerBounds(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	CRectangle	cParentBounds;

	//The container's bounds are where it was laid out, but never outside its parent container's bounds.
	mcBounds.Init(sPosition.x, sPosition.y, sPosition.x + sAreaSize.x, sPosition.y + sAreaSize.y);
	if (mpParent.IsNotNull())
	{
		if (mpParent->GetContainerBounds(&cParentBounds))
		{
			mcBounds.ClipTo(&cParentBounds);
		}
	}
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
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CContainer::GetRequiredSize(void)
{
	return msRequiredSize;
}

