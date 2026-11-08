/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_NAME_CHARACTERS_H__
#define __C_P_P_NAME_CHARACTERS_H__


//The characters of a C++ name (an identifier or a keyword), as in the source and in the assembly the back ends
//write: a letter or '_' starts one, and digits may follow.  [lex.name]
inline bool IsCPPNameStart(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_');
}


inline bool IsCPPNameCharacter(char c)
{
	return IsCPPNameStart(c) || ((c >= '0') && (c <= '9'));
}


#endif // __C_P_P_NAME_CHARACTERS_H__
