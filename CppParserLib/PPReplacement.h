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
#ifndef __P_P_REPLACEMENT_H__
#define __P_P_REPLACEMENT_H__
#include "BaseLib/Chars.h"
#include "PPToken.h"


class CPPReplacement : public CPPToken
{
public:
	int		miArgIndex;
	int64	mlliDefineID;
protected:
	bool	mbVariadic;

public:
	PP_TOKEN_CONSTRUCT(CPPReplacement);
	void 	Init(int iLine, int iColumn, char* szFileName);
	void 	Init(int64 lliDefineID, int iArgIndex, int iLine, int iColumn, char* szFileName, bool bVariadic);
	void 	Kill(void);
	bool 	IsReplacement(void);
	char* 	Print(CChars* psz);
	void 	Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens);
	bool	Equals(CPPToken* pcOther);
	bool	IsEmpty(void);
	bool	IsVariadic(void);
};


#endif // !__P_P_REPLACEMENT_H__

