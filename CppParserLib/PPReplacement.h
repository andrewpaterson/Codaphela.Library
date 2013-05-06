/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __P_P_REPLACEMENT_H__
#define __P_P_REPLACEMENT_H__
#include "BaseLib/Chars.h"
#include "PPToken.h"


class CPPReplacement : public CPPToken
{
public:
	int		miArgIndex;
	int		miDefineIndex;

	TOKEN_CONSTRUCT(CPPReplacement);
	void 		Init(int iLine, int iColumn);
	void 		Init(int iDefineIndex, int iArgIndex, int iLine, int iColumn);
	void 		Kill(void);
	BOOL 		IsReplacement(void);
	void 		Append(CChars* psz);
	void 		Copy(CPPToken* pcSource, CMemoryStackExtended* pcStack);
	BOOL		Equals(CPPToken* pcOther);
	BOOL		IsEmpty(void);
};


#endif // __P_P_REPLACEMENT_H__

