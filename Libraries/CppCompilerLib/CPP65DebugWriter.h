/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CPP65_DEBUG_WRITER_H__
#define __CPP65_DEBUG_WRITER_H__
#include "BaseLib/Chars.h"


//Writes the debug directives a unit's assembly describes its functions with for a debugger (Assembler65Lib's
//CAsm65DebugRecords, Assembler65C64Syntax.txt): what the front end knows of a function (its name, its variables and
//their types).  Where its frame is made is the back end's (CIRBackend::DescribeFrame).
class CCPP65DebugWriter
{
public:
	static void		AppendString(CChars* pszDest, const char* szText);
	static bool		DescribeFunction(CChars* pszCode, const char* szLabel, const char* szName);
	static size		FindAfterLabel(const char* szCode, const char* szLabel);
};


#endif // __CPP65_DEBUG_WRITER_H__
