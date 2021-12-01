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
#ifndef __P_P_TEXT_WITH_SOURCE_H__
#define __P_P_TEXT_WITH_SOURCE_H__
#include "PPText.h"


class CPPTextWithSource : public CPPText
{
public:
	char*	mszText;

	TOKEN_CONSTRUCT(CPPTextWithSource);
	void 	Init(int iLine, int iColumn);
	void 	Init(EPreprocessorText eType, int iLine, int iColumn, char* szStart, int iLength);
	void 	Init(EPreprocessorText eType, int iLine, int iColumn, char* szStart, char* szEndExclusive);
	void 	Kill(void);
	void 	Copy(CPPToken* pcSource, CPPTokens* pcTokens);
	BOOL 	HasSource(void);
	char*	TextStart(void);
	char*	TextEndInclusive(void);
	int		Sizeof(void);
};


#endif // !__P_P_TEXT_WITH_SOURCE_H__

