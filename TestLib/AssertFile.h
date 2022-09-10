/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __ASSERT_FILE_FUNCTIONS_H__
#define __ASSERT_FILE_FUNCTIONS_H__
#include "Assert.h"


bool PrivateAssertFilePath(char* szExpected, char* szActual, int iLine, char* szFile);

#define AssertFilePath(e, a)			Validate(PrivateAssertFilePath(e, a, __LINE__, __FILE__))


#endif // !__ASSERT_FILE_FUNCTIONS_H__

