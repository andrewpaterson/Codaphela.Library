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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsPointerNear(int x, int y, SInt32Vec2* psPosition, int32 iPixels)
{
	return (abs(x - psPosition->x) <= iPixels) && (abs(y - psPosition->y) <= iPixels);
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing has been released on yet.
//
//////////////////////////////////////////////////////////////////////////
void SDoubleClick::Init(void)
{
	iLastKey = -1;
	sClicks.Init();
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
	if (iKey != iLastKey)
	{
		sClicks.Forget();
	}
	iLastKey = iKey;
	if (sClicks.Pressed(x, y) == 2)
	{
		Forget();
		return true;
	}
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
	iLastKey = -1;
	sClicks.Forget();
}


//////////////////////////////////////////////////////////////////////////
//
// No press yet.
//
//////////////////////////////////////////////////////////////////////////
void SClickCount::Init(void)
{
	iCount = 0;
	iLastTime = 0;
	sLastPosition.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Button one was pressed at x, y.  Returns 1 for a single click, 2 for a
// double, 3 for a triple; a fourth press starts again at 1.
//
//////////////////////////////////////////////////////////////////////////
int32 SClickCount::Pressed(int x, int y)
{
	return Pressed(x, y, GetDebugTickCount());
}


//////////////////////////////////////////////////////////////////////////
//
// As Pressed(x, y) at iMilliseconds, for tests.
//
//////////////////////////////////////////////////////////////////////////
int32 SClickCount::Pressed(int x, int y, int64 iMilliseconds)
{
	if ((iCount > 0) && (iCount < 3) && (iMilliseconds - iLastTime <= DOUBLE_CLICK_MILLISECONDS) && IsPointerNear(x, y, &sLastPosition, DOUBLE_CLICK_PIXELS))
	{
		iCount++;
	}
	else
	{
		iCount = 1;
	}

	iLastTime = iMilliseconds;
	sLastPosition.Init(x, y);
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The next press is a single click, as after focus is lost.
//
//////////////////////////////////////////////////////////////////////////
void SClickCount::Forget(void)
{
	iCount = 0;
}

