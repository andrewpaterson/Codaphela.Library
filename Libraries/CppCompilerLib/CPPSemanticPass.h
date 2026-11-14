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
class CCPPSyntax;
class CCPPSyntaxTree;
class CCPPSyntaxDeclarator;
class CCPPUnit;
struct SCPP65Type;


//What a name resolved to once the types of expressions are known: the member o.Init() and p->miCount name, and the
//overload a call calls.  A name in a template's body can resolve to something else in each of the template's
//instances (mp->Init() in CHolder<M>): each is kept, the first here and the others after it (GetNextResolved).
struct SCPPResolvedName
{
	CCPPSymbol*				pcSymbol;		//The function, field or static member named.
	CCPPSyntaxDeclarator*	pcDeclarator;	//A function's declarator: which of its overloads.  NULL for anything else.
	int32					iNext;			//The next thing the same name resolved to, in the pass's others; -1 for none.
};


typedef CMapLongTemplate<SCPPResolvedName>	CMapResolvedNames;
typedef CArrayTemplate<SCPPResolvedName>	CArrayResolvedNames;
typedef CMapLongTemplate<SCPP65Type*>		CMapCPPNodeTypes;
typedef CMapLongTemplate<uint8>				CMapCPPNodeChanges;


//How the place an expression names can be changed where it is written (CCPPSemanticPass::FindChanges).
#define CPPPC_ASSIGNED		0x01	//Assigned to (=, += and the rest), incremented or decremented.
#define CPPPC_REFERENCE		0x02	//Bound to a reference that isn't const: an argument's, or a variable's initialiser.
#define CPPPC_ADDRESS		0x04	//Its address is taken.
#define CPPPC_MEMBER_CALL	0x08	//A member function that isn't const (or an operator of its class's) is called on it.


//Works out the types of a parsed translation unit's expressions without generating any code, with the C++
//compiler's type checker (CCPPUnit::Check), so names the parser alone can't resolve are: what o.Init() and
//p->miCount name, and which overload f(x) calls.  It keeps going past what it can't type (C++ the compiler doesn't
//support, or that is wrong): the rest of the statement, and the rest of the function, are still checked.  For the
//IDE's index (CCPPIndex::Gather) and refactorings.
//
//A check that is kept (Check's bKeep) also remembers what a refactoring asks of one unit: the type of every
//expression typed (GetType), written as C++ (AppendType, AppendDeclaration), and how the place an expression
//names can be changed there (FindChanges).  The types are the checked unit's, which lives until the pass is
//killed; the index's checks aren't kept and remember none of it.  It also gives the label the compiler would give a
//function for the check's target (AppendFunctionLabel), which hand written assembly names it by.
class CCPPSemanticPass
{
protected:
	CMapResolvedNames	mcNames;		//By the C++ token of the name (its last component's): the first thing it resolved to.
	CArrayResolvedNames	masOthers;		//What names resolved to after their first, in other instances of a template.
	CCPPErrors			mcErrors;
	CCPPUnit*			mpcUnit;		//A kept check's unit, NULL otherwise.
	CMapCPPNodeTypes	mcTypes;		//A kept check: each expression's type, by its node...
	CMapCPPNodeTypes	mcPlaces;		//...the type of the place of those only typed as places (the left of an assignment)...
	CMapCPPNodeChanges	mcChanges;		//...and how the type checker found a node's place changed (CPPPC_).

public:
	void				Init(void);
	void				Kill(void);

	bool				Check(CCPPSyntaxTree* pcTree, ECPPTarget eTarget = CPPT_Host, bool bKeep = false);
	bool				IsKept(void) { return mpcUnit != NULL; }

	void				Typed(CCPPSyntax* pcExpression, SCPP65Type* psType);
	void				PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType);
	void				Changed(CCPPSyntax* pcExpression, uint8 uiChanges);
	SCPP65Type*			GetType(CCPPSyntax* pcExpression);
	uint8				GetChanges(CCPPSyntax* pcExpression);
	uint8				FindChanges(CCPPSyntax* pcExpression);
	bool				AppendType(SCPP65Type* psType, CChars* pszDest);
	bool				AppendDeclaration(SCPP65Type* psType, const char* szName, CChars* pszDest);
	bool				AppendFunctionLabel(CCPPSyntaxDeclarator* pcDeclarator, CChars* pszDest);

	void				Resolved(int iToken, CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator);
	SCPPResolvedName*	GetResolved(int iToken);
	SCPPResolvedName*	GetNextResolved(SCPPResolvedName* psResolved);
	size				NumResolved(void);
	CCPPErrors*			GetErrors(void);
};


#endif // __C_P_P_SEMANTIC_PASS_H__

