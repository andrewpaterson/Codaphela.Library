/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Container.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CContainer::GetContainerBounds(SContainerBounds* psDest)
{
	memcpy(psDest, &msBounds, sizeof(SContainerBounds));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CContainer::SetBounds(SInt2 sPosition, SInt2 sAreaSize)
{
	SContainerBounds	sParentBounds;
	int					iThisRight;
	int					iThisBottom;
	int					iThisLeft;
	int					iThisTop;	
	int					iParentRight;
	int					iParentBottom;

	iThisRight = (int)(sPosition.x + sAreaSize.x);
	iThisBottom = (int)(sPosition.y + sAreaSize.y);
	iThisLeft = (int)sPosition.x;
	iThisTop = (int)sPosition.y;
	if (mpcParent)
	{
		mpcParent->GetContainerBounds(&sParentBounds);
		iParentRight = sParentBounds.msSize.x + sParentBounds.msTopLeft.x;
		iParentBottom = sParentBounds.msSize.y + sParentBounds.msTopLeft.y;

		if (iThisRight > iParentRight)
		{
			iThisRight = iParentRight;
		}
		if (iThisBottom > iParentBottom)
		{
			iThisBottom = iParentBottom;
		}

		if (iThisLeft < sParentBounds.msTopLeft.x)
		{
			iThisLeft = sParentBounds.msTopLeft.x;
		}
		if (iThisTop < sParentBounds.msTopLeft.y)
		{
			iThisTop = sParentBounds.msTopLeft.y;
		}
	}

	msBounds.msSize.x = iThisRight - iThisLeft;
	msBounds.msSize.y = iThisBottom - iThisTop;
	msBounds.msTopLeft.x = iThisLeft;
	msBounds.msTopLeft.y = iThisTop;
}

