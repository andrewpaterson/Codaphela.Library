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
#ifndef __C_P_P_DECLARATORS_H__
#define __C_P_P_DECLARATORS_H__
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"


class CCPP65Generator;


//Turns what a declaration says into a type: the specifiers give the type it starts from (int, a class, a
//typedef, a template parameter's argument), and the declarator makes pointers, references, arrays and functions
//of it.  It asks the unit what a name is and where the compiler is; it keeps nothing itself.
class CCPPDeclarators
{
protected:
	CCPP65Generator*	mpcUnit;

public:
	void					Init(CCPP65Generator* pcUnit);
	void					Kill(void);

	SCPP65Type*				GetType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser = NULL);
	SCPP65Type*				GetDeclaratorType(SCPP65Type* psType, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser, CCPPSyntaxFunctionSuffix* pcFunction = NULL);
	SCPP65Type*				GetFunctionType(SCPP65Type* psReturn, CCPPSyntaxFunctionSuffix* pcSuffix, CCPPSyntax* pcNode);
	int						GetInitialiserCount(CCPPSyntax* pcInitialiser);
	SCPP65Type*				GetBaseType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntax* pcNode);
	SCPP65Type*				GetTypeIdType(CCPPSyntaxTypeId* pcTypeId);
	SCPP65Type*				GetSymbolType(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
};


#endif // __C_P_P_DECLARATORS_H__
