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
#ifndef __C_P_P_NAME_CHARACTERS_H__
#define __C_P_P_NAME_CHARACTERS_H__
#include "BaseLib/PrimitiveTypes.h"


//The characters of a C++ name (an identifier or a keyword), as in the source and in the assembly the back ends
//write: a letter or '_' starts one, and digits may follow.  A name may have characters past ASCII in it, in UTF-8:
//every byte of one is 0x80 or more, and counts as a letter here.  Which of those characters the language allows is
//InvalidCPPNameCharacter's business.  [lex.name]
inline bool IsCPPNameStart(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') || ((uint8)c >= 0x80);
}


inline bool IsCPPNameCharacter(char c)
{
	return IsCPPNameStart(c) || ((c >= '0') && (c <= '9'));
}


size		UniversalCharacterNameLength(const char* sz, const char* szEnd);
const char*	InvalidCPPNameCharacter(const char* szName, size uiLength);


#endif // __C_P_P_NAME_CHARACTERS_H__

