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
#ifndef __C_P_P_DESUGARINGS_H__
#define __C_P_P_DESUGARINGS_H__
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "CPPSyntaxes.h"


class CCPPUnit;
class CCPPSymbol;
class CCPPFunction;
class CCTUserLiteral;
struct SCPP65Type;


//A list in braces whose left out braces were put back for the aggregate it fills (CCPPDesugarings::BracedFor).
class CCPPSyntaxInitializerList;
struct SCPPBracedList
{
	CCPPSyntax*					pcList;
	SCPP65Type*					psType;
	CCPPSyntaxInitializerList*	pcBraced;
};


typedef CArrayTemplate<SCPPBracedList>	CArrayCPPBracedLists;


//What a range-for statement is made of (CCPPDesugarings::GetRangeFor): the variables the compiler makes for it, and the
//expressions on them.
struct SCPPRangeFor
{
	CCPPSymbol*					pcRange;		//auto&& __range = range.
	CCPPSymbol*					pcBegin;		//auto __begin = begin.
	CCPPSymbol*					pcEnd;			//auto __end = end.
	CCPPSyntaxIdExpression*		pcRangeOfBegin;	//__range, as what begin is of.
	CCPPSyntaxIdExpression*		pcRangeOfEnd;	//__range, as what end is of.
	CCPPSyntaxBinaryExpression*	pcCondition;	//__begin != __end.
	CCPPSyntaxUnaryExpression*	pcIncrement;	//++__begin.
	CCPPSyntaxUnaryExpression*	pcElement;		//*__begin.
};


typedef CHashMapLongTemplate<SCPPRangeFor*>	CMapCPPRangeFors;


//The call a user-defined literal is (CCPPDesugarings::GetLiteralCall): the literal operator of its suffix, and its arguments
//(implicit ones, holding the literal) in an initialiser of their own.
struct SCPPLiteralCall
{
	CCPPFunction*							pcFunction;		//NULL if no literal operator takes the literal.
	CCPPSyntaxParenthesisedInitializer*		pcArguments;
};


typedef CHashMapLongTemplate<SCPPLiteralCall*>	CMapCPPLiteralCalls;



//What the compiler makes once for a node, to stand for what the program wrote (CCPPUnit::GetDesugarings), each kept by
//the node it is for: a range-for's variables and the expressions on them, the call a user-defined literal is, a braced
//list with its left out braces put back, the expression decltype(e)::name is, and the implicit arguments and
//assignments the IR front end gives values.
class CCPPDesugarings
{
protected:
	CCPPUnit*				mpcUnit;
	CMapCPPRangeFors		mcRangeFors;		//What each range-for statement is made of, by its node.
	CMapCPPLiteralCalls		mcLiteralCalls;		//The call each user-defined literal is, by its node.
	CMemoryStackExtended	mcMemory;			//The range-fors and literal calls, where they don't move.
	CHashMapLongTemplate<CCPPSyntaxIdExpression*>	mcDecltypeMembers;	//The expression each decltype(e)::name written as a type is (DecltypeMember).
	CHashMapLongTemplate<CCPPSyntaxBinaryExpression*>	mcImplicitAssignments;	//object = value for each object.operator=(value) of the operator= the compiler declares (ImplicitAssignment).
	CCPPSyntaxImplicitArgument*	mpcImplicitZero;	//The int 0 of s++'s operator++(0) (ImplicitZero).
	CHashMapLongTemplate<CCPPSyntaxImplicitArgument*>	mcImplicitValues;	//The arguments the IR front end gives values (ImplicitValue).
	CArrayCPPBracedLists	masBracedLists;			//The lists BracedFor has made, each for the list and the type it was made of.

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	SCPPRangeFor*			GetRangeFor(CCPPSyntaxRangeForStatement* pcFor);
	CCPPSymbol*				NewHiddenVariable(const char* szName, CCPPSyntaxRangeForStatement* pcFor);
	CCPPSyntaxIdExpression*	NewHiddenName(CCPPSymbol* pcSymbol, CCPPSyntax* pcParent);
	bool					RangeForLocals(CCPPSyntaxRangeForStatement* pcFor);
	bool					RangeIterators(CCPPSyntaxRangeForStatement* pcFor, SCPP65Type** ppsBegin, SCPP65Type** ppsEnd, CCPPFunction** ppcBegin, CCPPFunction** ppcEnd, bool* pbMember);
	CCTUserLiteral*			UserLiteralToken(CCPPSyntax* pcLiteral);
	SCPPLiteralCall*		GetLiteralCall(CCPPSyntax* pcLiteral);
	CCPPSyntaxImplicitArgument*	LiteralArgument(CCPPSyntax* pcLiteral, SCPP65Type* psType, CArraySyntax* papcArguments);
	void					UserLiteralSpelling(int32 iToken, CChars* pszSpelling);
	CCPPSyntax*				BracedFor(SCPP65Type* psType, CCPPSyntax* pcList);
	bool					InitialisesWhole(SCPP65Type* psMember, CCPPSyntax* pcValue);
	bool					ElidesBraces(SCPP65Type* psType, CArraySyntax* papcElements);
	void					Regroup(SCPP65Type* psType, CArraySyntax* papcElements, size* puiNext, CCPPSyntaxInitializerList* pcInto);
	CCPPSyntaxIdExpression*	DecltypeMember(CCPPSyntaxTypeId* pcType);
	CCPPSyntaxBinaryExpression*	ImplicitAssignment(CCPPSyntaxCallExpression* pcCall);
	CCPPSyntaxImplicitArgument*	ImplicitValue(CCPPSyntax* pcFor, SCPP65Type* psType, int iWhich = 0);
	CCPPSyntax*				ImplicitZero(void);
	CCPPTemplate*			InitializerListTemplate(void);
	SCPP65Type*				InitializerListElement(SCPP65Type* psType);
	SCPP65Type*				InitializerListOf(SCPP65Type* psElement, CCPPSyntax* pcNode);
	SCPP65Type*				BracedListType(CCPPSyntax* pcList, CCPPSyntax* pcNode);
	bool					InitializerListLayout(CCPPClass* pcClass, bool* pbEnd);
};


#endif // __C_P_P_DESUGARINGS_H__

