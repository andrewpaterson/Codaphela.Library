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
#ifndef __P_P_LINE_H__
#define __P_P_LINE_H__
#include "PPToken.h"
#include "PPTokenHolder.h"
#include "PPAbstractHolder.h"


class CPPLine : public CPPAbstractHolder
{
public:
	TOKEN_CONSTRUCT(CPPLine);
	void	Init(int iLine, int miColumn);
	void	Kill(void);
	BOOL	IsLine(void);
	void	Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens);
	BOOL	Equals(CPPToken* pcOther);
	BOOL	NeedsNewLine(void);
};


#endif // !__P_P_LINE_H__

