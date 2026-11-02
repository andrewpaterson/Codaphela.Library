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
#include <stdlib.h>
#include "BaseLib/Timer.h"
#include "DoubleClick.h"


#define DOUBLE_CLICK_MILLISECONDS	500
#define DOUBLE_CLICK_PIXELS			4


//////////////////////////////////////////////////////////////////////////
//
// Nothing has been released on yet.
//
//////////////////////////////////////////////////////////////////////////
void SDoubleClick::Init(void)
{
	miLastKey = -1;
	miLastTime = 0;
	msLastPosition.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Button one was released on iKey (0 or more) at x, y.  Returns true if
// that makes a double click.  A release that doesn't may be the first of
// one.
//
//////////////////////////////////////////////////////////////////////////
bool SDoubleClick::Released(int32 iKey, int x, int y)
{
	int64	iNow;

	iNow = GetDebugTickCount();
	if ((miLastKey == iKey) && (iNow - miLastTime <= DOUBLE_CLICK_MILLISECONDS) && (abs(x - msLastPosition.x) <= DOUBLE_CLICK_PIXELS) && (abs(y - msLastPosition.y) <= DOUBLE_CLICK_PIXELS))
	{
		miLastKey = -1;
		return true;
	}

	miLastKey = iKey;
	miLastTime = iNow;
	msLastPosition.Init(x, y);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The next release can't finish a double click, as when the keys have
// changed what they mean.
//
//////////////////////////////////////////////////////////////////////////
void SDoubleClick::Forget(void)
{
	miLastKey = -1;
}

