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
#include "CPPUnit.h"
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
// for (declaration : range) statement is
//
//   auto&& __range = range;
//   for (auto __begin = begin, __end = end; __begin != __end; ++__begin)
//   {
//     declaration = *__begin;
//     statement
//   }
//
// where begin and end are __range and __range + its length for an array,
// __range.begin() and __range.end() for a class with either as a member,
// and else begin(__range) and end(__range) of the namespaces the range's
// type is of.  [stmt.ranged]
//
// It is made of that: three variables the program can't name, and the
// expressions on them, which are syntax like any other (so an iterator
// that is an object has its operators chosen and called as written ones
// are).  This is them for a statement, made the first time it is asked.
//
//////////////////////////////////////////////////////////////////////////
SCPPRangeFor* CCPPUnit::GetRangeFor(CCPPSyntaxRangeForStatement* pcFor)
{
	SCPPRangeFor*		psRangeFor;
	SCPPRangeFor		sRangeFor;
	CCPPSyntaxMemory*	pcMemory;

	psRangeFor = mcRangeFors.Get((int64)(size)pcFor);
	if (psRangeFor)
	{
		return psRangeFor;
	}

	pcMemory = mpcTree->GetMemory();
	sRangeFor.pcRange = NewHiddenVariable("__range", pcFor);
	sRangeFor.pcBegin = NewHiddenVariable("__begin", pcFor);
	sRangeFor.pcEnd = NewHiddenVariable("__end", pcFor);
	sRangeFor.pcRangeOfBegin = NewHiddenName(sRangeFor.pcRange, pcFor);
	sRangeFor.pcRangeOfEnd = NewHiddenName(sRangeFor.pcRange, pcFor);

	sRangeFor.pcCondition = pcMemory->Create<CCPPSyntaxBinaryExpression>(pcFor);
	sRangeFor.pcCondition->SetTokens(pcFor->miFirstToken, pcFor->miFirstToken);
	sRangeFor.pcCondition->meOperator = CT_NotEqualTo;
	sRangeFor.pcCondition->mpcLeft = NewHiddenName(sRangeFor.pcBegin, sRangeFor.pcCondition);
	sRangeFor.pcCondition->mpcRight = NewHiddenName(sRangeFor.pcEnd, sRangeFor.pcCondition);

	sRangeFor.pcIncrement = pcMemory->Create<CCPPSyntaxUnaryExpression>(pcFor);
	sRangeFor.pcIncrement->SetTokens(pcFor->miFirstToken, pcFor->miFirstToken);
	sRangeFor.pcIncrement->meOperator = CT_Increment;
	sRangeFor.pcIncrement->mbPostfix = false;
	sRangeFor.pcIncrement->mpcOperand = NewHiddenName(sRangeFor.pcBegin, sRangeFor.pcIncrement);

	sRangeFor.pcElement = pcMemory->Create<CCPPSyntaxUnaryExpression>(pcFor);
	sRangeFor.pcElement->SetTokens(pcFor->miFirstToken, pcFor->miFirstToken);
	sRangeFor.pcElement->meOperator = CT_Multiply;
	sRangeFor.pcElement->mbPostfix = false;
	sRangeFor.pcElement->mpcOperand = NewHiddenName(sRangeFor.pcBegin, sRangeFor.pcElement);

	mcRangeFors.Put((int64)(size)pcFor, &sRangeFor);
	return mcRangeFors.Get((int64)(size)pcFor);
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol of a variable the compiler makes for a statement, which no
// name in the program is.  The unit's to free.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::NewHiddenVariable(const char* szName, CCPPSyntaxRangeForStatement* pcFor)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = NewMalloc<CCPPSymbol>();
	pcSymbol->Init(mpcTokeniser->GetIdentifierID((char*)szName), CPPSK_Variable, pcFor, pcFor->mpcScope);
	mapcHiddenSymbols.Add(pcSymbol);
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// An expression that is a hidden variable: a name with its symbol and no
// token of its own (it is where pcParent's statement starts, for an error).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::NewHiddenName(CCPPSymbol* pcSymbol, CCPPSyntax* pcParent)
{
	CCPPSyntaxMemory*			pcMemory;
	CCPPSyntaxIdExpression*		pcId;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;

	pcMemory = mpcTree->GetMemory();
	pcId = pcMemory->Create<CCPPSyntaxIdExpression>(pcParent);
	pcId->SetTokens(pcParent->miFirstToken, pcParent->miFirstToken);
	pcName = pcMemory->Create<CCPPSyntaxName>(pcId);
	pcName->SetTokens(pcParent->miFirstToken, pcParent->miFirstToken);
	pcName->mpcSymbol = pcSymbol;
	pcComponent = pcMemory->Create<CCPPSyntaxNameComponent>(pcName);
	pcComponent->SetTokens(pcParent->miFirstToken, pcParent->miFirstToken);
	pcComponent->meKind = CPPNC_Identifier;
	pcComponent->miName = -1;
	pcComponent->mpcSymbol = pcSymbol;
	pcName->macpComponents.Add(pcComponent);
	pcId->mpcName = pcName;
	return pcId;
}


//////////////////////////////////////////////////////////////////////////
//
// The locals of a range-for in the function being written: the range (a
// reference to it, so it is worked out once), the two iterators and the
// variable the statement declares, whose type may be auto of *__begin.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::RangeForLocals(CCPPSyntaxRangeForStatement* pcFor)
{
	SCPPRangeFor*	psRangeFor;
	SCPP65Type*		psRanged;
	SCPP65Type*		psBegin;
	SCPP65Type*		psEnd;
	SCPP65Type*		psType;
	SCPP65Type*		psElement;
	CCPPSyntaxName*	pcName;
	bool			bRvalue;

	if (pcFor->mpcRange->GetKind() == CPPS_InitializerList)
	{
		return Error(pcFor->mpcRange, "A list in braces as a range is not supported by the compiler yet");
	}
	psRangeFor = GetRangeFor(pcFor);
	bRvalue = IsRvalue(pcFor->mpcRange);
	psRanged = bRvalue ? NULL : PlaceOf(pcFor->mpcRange);
	psRanged = psRanged ? psRanged : ValueTypeOf(pcFor->mpcRange);
	if (!psRanged)
	{
		ReportTypeOf(pcFor->mpcRange);
		return false;
	}
	AddVariable(&mapcLocals, psRangeFor->pcRange, mcTypes.Reference(psRanged, bRvalue), CPPVS_Local, 0);

	if (!RangeIterators(pcFor, &psBegin, &psEnd, NULL, NULL, NULL))
	{
		return false;
	}
	AddVariable(&mapcLocals, psRangeFor->pcBegin, psBegin, CPPVS_Local, 0);
	AddVariable(&mapcLocals, psRangeFor->pcEnd, psEnd, CPPVS_Local, 0);

	pcName = pcFor->mpcDeclarator->GetName();
	psType = mcDeclarators.GetType(pcFor->mpcSpecifiers, pcFor->mpcDeclarator, pcFor, psRangeFor->pcElement);
	if (!psType || !pcName || !pcName->mpcSymbol)
	{
		return psType ? Error(pcFor, "This declaration is not supported by the compiler yet") : false;
	}
	psElement = psType->IsArray() ? psType->psBase : psType;
	if (psElement->IsClass() && !CompleteObjectClass(psElement->pcClass, pcFor))
	{
		return false;
	}
	AddVariable(&mapcLocals, pcName->mpcSymbol, psType, CPPVS_Local, 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The types of a range-for's __begin and __end, and for a range that is an
// object the functions that give them (members of its class with
// *pbMember, else functions that take it); an array has neither.  The
// range's variable has been made (RangeForLocals).  False after an error.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::RangeIterators(CCPPSyntaxRangeForStatement* pcFor, SCPP65Type** ppsBegin, SCPP65Type** ppsEnd, CCPPFunction** ppcBegin, CCPPFunction** ppcEnd, bool* pbMember)
{
	SCPPRangeFor*	psRangeFor;
	SCPP65Type*		psRanged;
	CCPPFunction*	pcBegin;
	CCPPFunction*	pcEnd;
	bool			bMember;

	psRangeFor = GetRangeFor(pcFor);
	psRanged = GetVariable(psRangeFor->pcRange)->mpsType->psBase;
	pcBegin = NULL;
	pcEnd = NULL;
	bMember = false;
	if (psRanged->IsArray())
	{
		if (psRanged->iCount <= 0)
		{
			return Error(pcFor->mpcRange, "An array that is a range must have a length");
		}
		*ppsBegin = mcTypes.Pointer(psRanged->psBase);
		*ppsEnd = *ppsBegin;
	}
	else
	{
		if (!psRanged->IsClass() || !CompleteClass(psRanged->pcClass, pcFor->mpcRange))
		{
			return Error(pcFor->mpcRange, "A range must be an array or an object with begin and end");
		}
		bMember = HasMember(psRanged->pcClass, mpcTokeniser->GetIdentifierID((char*)"begin")) || HasMember(psRanged->pcClass, mpcTokeniser->GetIdentifierID((char*)"end"));
		pcBegin = mcBinder.BindRangeFunction(mpcTokeniser->GetIdentifierID((char*)"begin"), bMember ? psRanged->pcClass : NULL, psRangeFor->pcRangeOfBegin, pcFor);
		pcEnd = mcBinder.BindRangeFunction(mpcTokeniser->GetIdentifierID((char*)"end"), bMember ? psRanged->pcClass : NULL, psRangeFor->pcRangeOfEnd, pcFor);
		if (!pcBegin || !pcEnd)
		{
			return Error(pcFor->mpcRange, "A range must be an array or an object with begin and end");
		}
		*ppsBegin = pcBegin->mpsReturn->WithoutReference()->Unqualified();
		*ppsEnd = pcEnd->mpsReturn->WithoutReference()->Unqualified();
		if (!(*ppsBegin)->IsScalar() && !(*ppsBegin)->IsClass())
		{
			return Error(pcFor->mpcRange, "begin of a range must give a pointer or an object");
		}
	}
	if (ppcBegin)
	{
		*ppcBegin = pcBegin;
		*ppcEnd = pcEnd;
		*pbMember = bMember;
	}
	return true;
}
