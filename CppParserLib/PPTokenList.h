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
#ifndef __P_P_TOKEN_LIST_H__
#define __P_P_TOKEN_LIST_H__
#include "BaseLib/Chars.h"
#include "PPToken.h"


class CPPTokenList
{
protected:
	CArrayPPTokenPtrs	mcArray;

public:
	void		Init(void);
	void		Kill(void);
	char*		Print(CChars* psz, bool bShowFileAndLine = false);
	void		Add(CPPToken* ppcToken);
	bool		Equals(CPPTokenList* pcOther);
	void		SavageAppend(CChars* psz, size iDepth);
	size		NumTokens(void);
	CPPToken*	Get(size iTokenIndex);
	void		Dump(void);
};


typedef CArrayTemplate<CPPTokenList> CArrayPPTokenLists;


#endif // !__P_P_TOKEN_LIST_H__

