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
#ifndef __C_P_P_SYNTAX_IMPLICIT_ARGUMENT_H__
#define __C_P_P_SYNTAX_IMPLICIT_ARGUMENT_H__
#include "CPPSyntaxExpression.h"


struct SCPP65Type;


//An argument a call is given that the program doesn't write: the int 0 a postfix operator function is called with (s++
//is s.operator++(0), mbZero), or the size an allocation function is given (operator new(sizeof(S)), its value the IR
//front end's).  The unit makes them, in no tree's nodes, so nothing reads or writes their fields (CCPPUnit::ImplicitZero,
//NewSize).  [over.inc] 1, [expr.new] 9
class CCPPSyntaxImplicitArgument : public CCPPSyntaxExpression
{
public:
	SCPP65Type*				mpsType;
	bool					mbZero;

	void					Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent) override;
	CPP_SYNTAX_NODE(ImplicitArgument)
	void					PrettyPrint(CChars* pszDest, int iDepth) override;
};


#endif // __C_P_P_SYNTAX_IMPLICIT_ARGUMENT_H__
