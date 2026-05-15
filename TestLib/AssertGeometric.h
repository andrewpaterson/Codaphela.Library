#ifndef __ASSERT_GEOMETRIC_FUNCTIONS_H__
#define __ASSERT_GEOMETRIC_FUNCTIONS_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela TestLib

Codaphela TestLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela TestLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela TestLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/IntVec2.h"
#include "BaseLib/Int32Vec2.h"
#include "Assert.h"


bool PrivateAssertIntVec2(int iExpectedX, int iExpectedY, SIntVec2* psActual, int iLine, char* szFile);
bool PrivateAssertInt32Vec2(int32 iExpectedX, int32 iExpectedY, SInt32Vec2* psActual, int iLine, char* szFile);


#define AssertIntVec2(ex, ey, a)			Validate(PrivateAssertIntVec2(ex, ey, a, __LINE__, __FILE__))
#define AssertInt32Vec2(ex, ey, a)			Validate(PrivateAssertInt32Vec2(ex, ey, a, __LINE__, __FILE__))


#endif // __ASSERT_GEOMETRIC_FUNCTIONS_H__

