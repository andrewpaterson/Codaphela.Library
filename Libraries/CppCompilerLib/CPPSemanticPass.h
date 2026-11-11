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
#ifndef __C_P_P_SEMANTIC_PASS_H__
#define __C_P_P_SEMANTIC_PASS_H__
#include "BaseLib/MapLongTemplate.h"
#include "CPPErrors.h"
#include "CPPTarget.h"


class CCPPSymbol;
class CCPPSyntaxTree;
class CCPPSyntaxDeclarator;


//What a name resolved to once the types of expressions are known: the member o.Init() and p->miCount name, and the
//overload a call calls.
struct SCPPResolvedName
{
	CCPPSymbol*				pcSymbol;		//The function, field or static member named.
	CCPPSyntaxDeclarator*	pcDeclarator;	//A function's declarator: which of its overloads.  NULL for anything else.
};


typedef CMapLongTemplate<SCPPResolvedName>	CMapResolvedNames;


//Works out the types of a parsed translation unit's expressions without generating any code, with the C++
//compiler's type checker (CCPP65Generator::Check), so names the parser alone can't resolve are: what o.Init() and
//p->miCount name, and which overload f(x) calls.  It keeps going past what it can't type (C++ the compiler doesn't
//support, or that is wrong): the rest of the statement, and the rest of the function, are still checked.  For the
//IDE's index (CCPPIndex::Gather) and refactorings.
class CCPPSemanticPass
{
protected:
	CMapResolvedNames	mcNames;		//By the C++ token of the name (its last component's).
	CCPPErrors			mcErrors;

public:
	void				Init(void);
	void				Kill(void);

	bool				Check(CCPPSyntaxTree* pcTree, ECPPTarget eTarget = CPPT_Host);

	void				Resolved(int iToken, CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator);
	SCPPResolvedName*	GetResolved(int iToken);
	size				NumResolved(void);
	CCPPErrors*			GetErrors(void);
};


#endif // __C_P_P_SEMANTIC_PASS_H__

