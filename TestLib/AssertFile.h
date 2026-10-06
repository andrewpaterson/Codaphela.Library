#ifndef __ASSERT_FILE_FUNCTIONS_H__
#define __ASSERT_FILE_FUNCTIONS_H__
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
#include "Assert.h"


bool PrivateAssertFile(const char* szExpectedFilename, char* szActualFilename, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFile(CChars szExpectedFilename, CChars szActualFilename, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFileMemory(const char* szExpectedFilename, void* pcMemory, size iLength, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFileString(const char* szExpectedFilename, const char* szString, char* szPrefix, size iLine, char* szFile);

bool PrivateAssertDirectory(char* szExpectedDirectoryName, char* szActualDirectoryName, char* szPrefix, size iLine, char* szFile);

bool PrivateAssertFilePath(char* szExpected, char* szActual, int iLine, char* szFile);

#define AssertFile(e, a)				Validate(PrivateAssertFile(e, a, NULL, __LINE__, __FILE__))
#define AssertFileMemory(e, a, l)		Validate(PrivateAssertFileMemory(e, a, l, NULL, __LINE__, __FILE__))
#define AssertFileString(e, a)			Validate(PrivateAssertFileString(e, a, NULL, __LINE__, __FILE__))
#define AssertDirectory(e, a)			Validate(PrivateAssertDirectory(e, a, NULL, __LINE__, __FILE__))

#define AssertFilePath(e, a)			Validate(PrivateAssertFilePath(e, a, __LINE__, __FILE__))


#endif // __ASSERT_FILE_FUNCTIONS_H__

