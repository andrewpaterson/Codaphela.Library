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
#ifndef __C_P_P_SYNTAX_IMPLICIT_ZERO_H__
#define __C_P_P_SYNTAX_IMPLICIT_ZERO_H__
#include "CPPSyntaxExpression.h"


//The int 0 a postfix operator function is called with: s++ is s.operator++(0).  The unit makes one, in no tree's
//nodes (CCPPUnit::ImplicitZero).  [over.inc] 1
class CCPPSyntaxImplicitZero : public CCPPSyntaxExpression
{
public:
	CPP_SYNTAX_NODE(ImplicitZero)
	void					PrettyPrint(CChars* pszDest, int iDepth) override;
};


#endif // __C_P_P_SYNTAX_IMPLICIT_ZERO_H__
