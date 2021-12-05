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
#ifndef __P_P_UNKNOWN_H__
#define __P_P_UNKNOWN_H__
#include "BaseLib/ExternalString.h"
#include "PPDirective.h"
#include "PPToken.h"


class CPPUnknown : public CPPDirective
{
public:
	TOKEN_CONSTRUCT(CPPUnknown);
	void	Init(int iLine, int iColumn);
	void	Kill(void);
	void	Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens);
	char*	Print(CChars* psz);
	BOOL	IsUnknown(void);
};


#endif // !__P_P_UNKNOWN_H__

