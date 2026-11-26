/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_T_USER_LITERAL_H__
#define __C_T_USER_LITERAL_H__
#include "CppToken.h"


//A user-defined literal, as 10_km, 1.5_m, 'x'_c and "text"_s: the literal it is without its suffix, which is a token
//as any other number, character or string is (but is not one of the tokeniser's own), and the suffix.  It is none of
//IsNumber, IsChar and IsString itself: its value is what a literal operator makes of the literal.  [lex.ext]
class CCTUserLiteral : public CCPPToken
{
public:
	CCPPToken*	mpcLiteral;
	int32		miSuffix;		//The suffix's ID in the tokeniser's identifiers.

	CPP_TOKEN_CONSTRUCT(CCTUserLiteral);
	void	Init(void);
	void	Init(CCPPToken* pcLiteral, int64 lliSuffix);
	void	Kill(void);
	bool	IsUserLiteral(void) const;
	bool	IsUserString(void) override;
};


#endif // !__C_T_USER_LITERAL_H__

