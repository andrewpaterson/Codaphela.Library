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
#include "BaseLib/Numbers.h"
#include "WorldLib/World.h"
#include "Viewport.h"
#include "Drawable.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDrawable::Init(CViewport* pcViewport)
{
	CComponent::Init(pcViewport);
	mpcGraphicsViewport = mpcWorld->CreateViewport();
	msOldSize.Init(MAX_INT, MAX_INT);
	msOldPosition.Init(MAX_INT, MAX_INT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDrawable::Kill(void)
{
	mpcWorld->ReleaseViewport(mpcGraphicsViewport);
	CComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDrawable::Draw(void)
{
	SContainerBounds	sBounds;

	GetContainerBounds(&sBounds);
	mpcGraphicsViewport->Set(sBounds.msTopLeft.x, sBounds.msTopLeft.y, sBounds.msSize.x, sBounds.msSize.y);
	return CComponent::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDrawable::SetActualSize(int fWidth, int fHeight)
{
	msOldSize = msActualSize;
	msActualSize.Init(fWidth, fHeight);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDrawable::SetActualSize(SInt2 sSize)
{
	msOldSize = msActualSize;
	msActualSize = sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDrawable::SetPosition(int x, int y)
{
	msOldPosition = msPosition;
	msPosition.Init(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDrawable::SetPosition(SInt2 sPosition)
{
	msOldPosition = msPosition;
	msPosition = sPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDrawable::GetPosition(SInt2* psPosition)
{
	psPosition->Init(msPosition.x, msPosition.y);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDrawable::HasSizeChanged(void)
{
	if (msOldSize == msActualSize)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDrawable::HasPositionChanged(void)
{
	if (msOldPosition == msPosition)
	{
		return false;
	}
	return true;
}

