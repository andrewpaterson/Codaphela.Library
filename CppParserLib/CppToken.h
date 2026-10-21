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
#ifndef __C_P_P_TOKEN_H__
#define __C_P_P_TOKEN_H__
#include "BaseLib/Define.h"
#include "BaseLib/Chars.h"
#include "GeneralToken.h"
#include "CppTokens.h"


class CCPPTokeniser;
class CCPPToken
{
protected:
	int32	miSource;		//Index of the preprocessed token this was made from.

public:
	virtual void	Init(void) =0;
	virtual void	Kill(void) =0;
	virtual bool	IsReservedWord(void);
	virtual bool	IsIdentifier(void);
	virtual bool	IsNumber(void);
	virtual bool	IsString(void);
	virtual bool	IsChar(void);
	virtual bool	IsBool(void);
	virtual bool	Is(ECppTokens eToken);
	virtual void	Append(CChars* psz, CCPPTokeniser* pcTokeniser);
			void	SetSource(int iSource);
			int		GetSource(void);
};


#endif // !__C_P_P_TOKEN_H__

