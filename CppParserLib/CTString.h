/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __C_T_STRING_H__
#define __C_T_STRING_H__
#include "CppToken.h"


class CCTString : public CCppToken
{
public:
	char*	mszStart;  //This is the starting quote (") inclusive.
	char*	mszEnd;  //This is the ending quote (") inclusive.

	TOKEN_CONSTRUCT(CCTString);
	void Init(void);
	void Init(char* szStart, char* szEnd);
	void Kill(void);
	BOOL IsString(void);
	void Append(CChars* psz, CTokeniser* pcTokeniser);
};


#endif // !__C_T_STRING_H__

