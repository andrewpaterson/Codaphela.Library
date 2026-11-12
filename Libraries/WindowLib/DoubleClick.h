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
#ifndef __DOUBLE_CLICK_H__
#define __DOUBLE_CLICK_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Int32Vec2.h"


#define DOUBLE_CLICK_MILLISECONDS	500
#define DOUBLE_CLICK_PIXELS			4


//The pointer at x, y is within iPixels of psPosition, along and down: near enough to be the same place.
bool IsPointerNear(int x, int y, SInt32Vec2* psPosition, int32 iPixels);


//Counts the presses of button one that make a double or triple click, as a text's word and line selection does: each
//press within half a second and a few pixels of the last counts on from it, up to three.
struct SClickCount
{
	int32		iCount;  //The last press's count, 0 for none.
	int64		iLastTime;  //When, in milliseconds.
	SInt32Vec2	sLastPosition;  //Where, relative to whatever the caller measures from.

	void	Init(void);
	int32	Pressed(int x, int y);
	int32	Pressed(int x, int y, int64 iMilliseconds);
	void	Forget(void);
};


//Tells a double click from two clicks: button one released twice on the same thing in half a second within a few
//pixels (counted as SClickCount counts presses).  The thing is a key the caller chooses, like a list's row or a tab's
//index, so nothing is held.
struct SDoubleClick
{
	int32		iLastKey;  //What button one was last released on, -1 for nothing.
	SClickCount	sClicks;  //The releases on it; positions relative to whatever the caller measures from.

	void	Init(void);
	bool	Released(int32 iKey, int x, int y);
	void	Forget(void);
};


#endif // __DOUBLE_CLICK_H__

