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
#ifndef __C_T_FLOAT_H__
#define __C_T_FLOAT_H__
#include "CTNumber.h"


class CCTFloat : public CCTNumber
{
public:
	float mcValue;

	CPP_TOKEN_CONSTRUCT(CCTFloat);
	void Init(void);
	void Init(float f);
	void Kill(void);
	void Append(CChars* psz, CCPPTokeniser* pcTokeniser);
};


#endif // !__C_T_FLOAT_H__

