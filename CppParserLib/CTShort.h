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
#ifndef __C_T_SHORT_H__
#define __C_T_SHORT_H__
#include "CTNumber.h"


class CCTShort : public CCTNumber
{
public:
	unsigned short int	mcValue;
	bool				mbSigned;

	CPP_TOKEN_CONSTRUCT(CCTShort);
	void Init(void);
	void Init(unsigned short int s);
	void Kill(void);
	bool IsSigned(void);
	void Append(CChars* psz, CCPPTokeniser* pcTokeniser);
};


#endif // !__C_T_SHORT_H__

