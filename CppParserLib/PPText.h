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
#ifndef __P_P_TEXT_H__
#define __P_P_TEXT_H__
#include "BaseLib/ExternalString.h"
#include "PPToken.h"


enum EPreprocessorText
{
	PPT_Identifier,
	PPT_Number,
	PPT_Decorator,
	PPT_DoubleQuoted,
	PPT_SingleQuoted
};


class CPPText : public CPPToken
{
public:
	EPreprocessorText	meType;
	CExternalString		mcText;

	PP_TOKEN_CONSTRUCT(CPPText);
	void 			Init(int iLine, int iColumn, char* szFileName);
	void 			Init(EPreprocessorText eType, int iLine, int iColumn, char* szFileName, char* szStart, int iLength);
	void 			Init(EPreprocessorText eType, int iLine, int iColumn, char* szFileName, char* szStart, char* szEndExclusive);
	void 			Kill(void);
	bool 			IsText(void);
	char* 			Print(CChars* psz);
	void 			Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens);
	virtual bool	HasSource(void);
	bool			Equals(CPPToken* pcOther);
	virtual	char*	TextStart(void);
	virtual	char*	TextEndInclusive(void);
	bool			IsEmpty(void);
	int				Length(void);
};


#endif // !__P_P_TEXT_H__

