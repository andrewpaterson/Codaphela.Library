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
#ifndef __P_P_ABSTRACT_HOLDER_H__
#define __P_P_ABSTRACT_HOLDER_H__
#include "PPToken.h"
#include "PPTokenHolder.h"


class CPPAbstractHolder : public CPPToken
{
protected:
	CPPTokenHolder	mcTokens;

public:
	void 				Init(int iLine, int iColumn);
	void 				Kill(void);
	void				Fake(CPPTokenHolder* pcTokens);
	BOOL 				IsAbstractHolder(void);
	char*				Print(CChars* psz);
	void 				Copy(CPPAbstractHolder* pcCast, CPPTokenMemory* pcTokens);
	int	 				TokenLength(void);
	CPPToken*			Get(int iIndex);
	void				SavageDump(void);
	BOOL				Equals(CPPToken* pcOther);
	BOOL				IsEmpty(void);
	CPPTokenHolder*		GetTokens(void);
};


#endif // !__P_P_ABSTRACT_HOLDER_H__

