#ifndef __ASSERT_GEOMETRIC_FUNCTIONS_H__
#define __ASSERT_GEOMETRIC_FUNCTIONS_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/Int2.h"
#include "Assert.h"


bool PrivateAssertInt2(int iExpectedX, int iExpectedY, SInt2* psActual, int iLine, char* szFile);


#define AssertInt2(ex, ey, a)			Validate(PrivateAssertInt2(ex, ey, a, __LINE__, __FILE__))


#endif // __ASSERT_GEOMETRIC_FUNCTIONS_H__

