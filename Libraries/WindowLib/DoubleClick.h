#ifndef __DOUBLE_CLICK_H__
#define __DOUBLE_CLICK_H__
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
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Int32Vec2.h"


//Tells a double click from two clicks: button one released twice on the same thing in half a second within a few
//pixels.  The thing is a key the caller chooses, like a list's row or a tab's index, so nothing is held.
struct SDoubleClick
{
	int32		miLastKey;  //What button one was last released on, -1 for nothing.
	int64		miLastTime;  //When, in milliseconds.
	SInt32Vec2	msLastPosition;  //Where, relative to whatever the caller measures from.

	void	Init(void);
	bool	Released(int32 iKey, int x, int y);
	void	Forget(void);
};


#endif // __DOUBLE_CLICK_H__

