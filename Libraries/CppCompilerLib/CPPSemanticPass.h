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
#include "CPPUnitObserver.h"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "CPPErrors.h"
#include "CPPTarget.h"
#include "CPPCompileOptions.h"


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


typedef CHashMapLongTemplate<SCPPResolvedName>	CMapResolvedNames;
typedef CArrayTemplate<SCPPResolvedName>	CArrayResolvedNames;
typedef CHashMapLongTemplate<SCPP65Type*>		CMapCPPNodeTypes;
typedef CHashMapLongTemplate<uint8>				CMapCPPNodeChanges;


//Where a value would need a cast if an enum were an enum class (CCPPSemanticPass::SetRecordedEnum): what the type
//checker refuses of an enum class is recorded for that enum, and checked on as if it were allowed.
#define CPPEC_TO_OTHER		0	//pcNode is a value of the enum given to psTo, another type: it needs a cast to psTo.
#define CPPEC_TO_ENUM		1	//pcNode is another type's value (psFrom) given to the enum: it needs a cast to the enum.
#define CPPEC_ARITHMETIC	2	//pcNode is an operator's expression (binary, unary, an assignment like +=, a subscript)
								//with the enum's value as an operand: each such operand needs a cast to an integer.
#define CPPEC_COMPARE		3	//pcNode compares the enum's value with another type's: it needs a cast to an integer.
#define CPPEC_CONDITION		4	//pcNode is the enum's value used as a condition.
#define CPPEC_INCREMENT		5	//pcNode steps the enum's value with ++ or --: no cast makes that an enum class's.


struct SCPPEnumCast
{
	CCPPSyntax*		pcNode;
	SCPP65Type*		psFrom;
	SCPP65Type*		psTo;
	uint8			uiKind;		//CPPEC_.
};


typedef CArrayTemplate<SCPPEnumCast>	CArrayCPPEnumCasts;


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
//
//SetRecordedEnum, before the check, names an enum to be checked as an enum class though it isn't one: every place
//the type checker would refuse its value (given to another type, in arithmetic, compared with an integer, as a
//condition) is recorded instead (GetEnumCast), and the check goes on as if it were allowed, so what follows from
//one cast is found too.  A refactoring that makes the enum an enum class writes the casts from them.
class CCPPSemanticPass : public CCPPUnitObserver
{
protected:
	CMapResolvedNames	mcNames;		//By the C++ token of the name (its last component's): the first thing it resolved to.
	CArrayResolvedNames	masOthers;		//What names resolved to after their first, in other instances of a template.
	CCPPErrors			mcErrors;
	CCPPErrors			mcWarnings;  //What the check would warn of (GetWarnings).
	CCPPUnit*			mpcUnit;		//A kept check's unit, NULL otherwise.
	CMapCPPNodeTypes	mcTypes;		//A kept check: each expression's type, by its node...
	CMapCPPNodeTypes	mcPlaces;		//...the type of the place of those only typed as places (the left of an assignment)...
	CMapCPPNodeChanges	mcChanges;		//...and how the type checker found a node's place changed (CPPPC_).
	bool				mbTemplatesAsThemselves;	//A check that isn't kept checks each template in its own words too (CCPPUnit::CheckTemplatesAsThemselves), as a kept one does.
	CChars				mszRecordedEnum;	//An enum, by its name with its scopes ("CMotor::EState"), to check as an enum class; empty for none.
	SCPP65Type*			mpsRecordedEnum;	//Its type, once the check has met it.
	CArrayCPPEnumCasts	masEnumCasts;		//Where its values would need casts.
	CCPPCompileOptions	mcOptions;			//What the unit is checked with (SetOptions): the defaults, or a build's.

public:
	void				Init(void);
	void				Kill(void);

	bool				Check(CCPPSyntaxTree* pcTree, ECPPTarget eTarget = CPPT_Host, bool bKeep = false);
	bool				IsKept(void) const;
	void				SetOptions(CCPPCompileOptions* pcOptions);
	CCPPCompileOptions*	GetOptions(void);
	void				SetTemplatesAsThemselves(bool bTemplatesAsThemselves) { mbTemplatesAsThemselves = bTemplatesAsThemselves; }
	bool				IsTemplatesAsThemselves(void) const;
	void				SetRecordedEnum(const char* szQualifiedName);
	const char*			GetRecordedEnum(void) const;
	void				EnumMade(CCPPUnit* pcUnit, SCPP65Type* psType) override;
	SCPP65Type*			GetRecordedEnumType(void) const;
	bool				IsRecordedEnum(SCPP65Type* psType) override;
	bool				RecordEnumCast(CCPPSyntax* pcNode, SCPP65Type* psFrom, SCPP65Type* psTo, uint8 uiKind) override;
	size				NumEnumCasts(void) const;
	SCPPEnumCast*		GetEnumCast(size uiIndex) { return masEnumCasts.Get(uiIndex); }

	void				Typed(CCPPSyntax* pcExpression, SCPP65Type* psType) override;
	void				PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType) override;
	void				Changed(CCPPSyntax* pcExpression, uint8 uiChanges) override;
	SCPP65Type*			GetType(CCPPSyntax* pcExpression);
	uint8				GetChanges(CCPPSyntax* pcExpression);
	uint8				FindChanges(CCPPSyntax* pcExpression);
	bool				AppendType(SCPP65Type* psType, CChars* pszDest);
	bool				AppendDeclaration(SCPP65Type* psType, const char* szName, CChars* pszDest);
	bool				AppendFunctionLabel(CCPPSyntaxDeclarator* pcDeclarator, CChars* pszDest);

	void				Resolved(int iToken, CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator) override;
	SCPPResolvedName*	GetResolved(int iToken);
	SCPPResolvedName*	GetNextResolved(SCPPResolvedName* psResolved);
	size				NumResolved(void);
	CCPPErrors*			GetErrors(void);
	CCPPErrors*			GetWarnings(void);
};


#endif // __C_P_P_SEMANTIC_PASS_H__

