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
#include <string.h>
#include "CTInt.h"
#include "CTShort.h"
#include "CTLongLong.h"
#include "CTChar.h"
#include "CTBoolean.h"
#include "CTString.h"
#include "CTIdentifier.h"
#include "CTUserLiteral.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPPUnit.h"
#include "CPPSyntaxes.h"
#include "CPP65IRFrontEnd.h"
#include "CPPConstants.h"
#include "CPPSemanticPass.h"
#include "CPPTypeWriter.h"
#include "LiteralHelper.h"
#include "CPPDesugarings.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDesugarings::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mcRangeFors.Init();
	mcLiteralCalls.Init();
	mcMemory.Init(4096);
	mcDecltypeMembers.Init();
	mcImplicitAssignments.Init();
	mpcImplicitZero = NULL;
	mcImplicitValues.Init();
	masBracedLists.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDesugarings::Kill(void)
{
	mcRangeFors.Kill();
	mcLiteralCalls.Kill();
	mcMemory.Kill();
	mcDecltypeMembers.Kill();
	mcImplicitAssignments.Kill();
	mcImplicitValues.Kill();
	masBracedLists.Kill();
	mpcUnit = NULL;
}


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
SCPPRangeFor* CCPPDesugarings::GetRangeFor(CCPPSyntaxRangeForStatement* pcFor)
{
	SCPPRangeFor**		ppsRangeFor;
	SCPPRangeFor*		psRangeFor;
	SCPPRangeFor		sRangeFor;
	CCPPSyntaxMemory*	pcMemory;

	ppsRangeFor = mcRangeFors.Get((int64)(size)pcFor);
	if (ppsRangeFor)
	{
		return *ppsRangeFor;
	}

	pcMemory = mpcUnit->mpcTree->GetMemory();
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

	psRangeFor = (SCPPRangeFor*)mcMemory.Add(sizeof(SCPPRangeFor));
	*psRangeFor = sRangeFor;
	mcRangeFors.Put((int64)(size)pcFor, &psRangeFor);
	return psRangeFor;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol of a variable the compiler makes for a statement, which no
// name in the program is.  The unit's to free.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPDesugarings::NewHiddenVariable(const char* szName, CCPPSyntaxRangeForStatement* pcFor)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = NewMalloc<CCPPSymbol>();
	pcSymbol->Init(mpcUnit->mpcTokeniser->GetIdentifierID((char*)szName), CPPSK_Variable, pcFor, pcFor->mpcScope);
	mpcUnit->mapcHiddenSymbols.Add(pcSymbol);
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// An expression that is a hidden variable: a name with its symbol and no
// token of its own (it is where pcParent's statement starts, for an error).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPDesugarings::NewHiddenName(CCPPSymbol* pcSymbol, CCPPSyntax* pcParent)
{
	CCPPSyntaxMemory*			pcMemory;
	CCPPSyntaxIdExpression*		pcId;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;

	pcMemory = mpcUnit->mpcTree->GetMemory();
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
bool CCPPDesugarings::RangeForLocals(CCPPSyntaxRangeForStatement* pcFor)
{
	SCPPRangeFor*	psRangeFor;
	SCPP65Type*		psRanged;
	SCPP65Type*		psBegin;
	SCPP65Type*		psEnd;
	SCPP65Type*		psType;
	SCPP65Type*		psElement;
	CCPPSyntaxName*	pcName;
	bool			bRvalue;

	psRangeFor = GetRangeFor(pcFor);
	if (pcFor->mpcRange->GetKind() == CPPS_InitializerList)
	{
		//for (x : {1, 2, 3}) is over a std::initializer_list<int>.  [stmt.ranged] 1
		bRvalue = true;
		psRanged = BracedListType(pcFor->mpcRange, pcFor->mpcRange);
		if (!psRanged)
		{
			return false;
		}
	}
	else
	{
		bRvalue = mpcUnit->GetTypeChecker()->IsRvalue(pcFor->mpcRange);
		psRanged = bRvalue ? NULL : mpcUnit->GetTypeChecker()->PlaceOf(pcFor->mpcRange);
		if (!psRanged)
		{
			//A temporary's member array, Make().a, is an array, not the pointer its value would be.  [stmt.ranged] 1
			psRanged = bRvalue ? mpcUnit->GetTypeChecker()->PlaceOf(pcFor->mpcRange) : NULL;
			psRanged = (psRanged && psRanged->IsArray()) ? psRanged : NULL;
		}
		psRanged = psRanged ? psRanged : mpcUnit->GetTypeChecker()->ValueTypeOf(pcFor->mpcRange);
	}
	if (!psRanged)
	{
		mpcUnit->GetTypeChecker()->ReportTypeOf(pcFor->mpcRange);
		return false;
	}
	mpcUnit->AddVariable(mpcUnit->GetLocals(), psRangeFor->pcRange, mpcUnit->mcTypes.Reference(psRanged, bRvalue), CPPVS_Local, 0);

	if (!RangeIterators(pcFor, &psBegin, &psEnd, NULL, NULL, NULL))
	{
		return false;
	}
	mpcUnit->AddVariable(mpcUnit->GetLocals(), psRangeFor->pcBegin, psBegin, CPPVS_Local, 0);
	mpcUnit->AddVariable(mpcUnit->GetLocals(), psRangeFor->pcEnd, psEnd, CPPVS_Local, 0);

	pcName = pcFor->mpcDeclarator->GetName();
	psType = mpcUnit->mcDeclarators.GetType(pcFor->mpcSpecifiers, pcFor->mpcDeclarator, pcFor, psRangeFor->pcElement);
	if (!psType || !pcName || !pcName->mpcSymbol)
	{
		return psType ? mpcUnit->mcDiagnostics.Error(pcFor, "This declaration is not supported by the compiler yet") : false;
	}
	psElement = psType->ElementType();
	if (psElement->IsClass() && !mpcUnit->GetClasses()->CompleteObjectClass(psElement->pcClass, pcFor))
	{
		return false;
	}
	mpcUnit->AddVariable(mpcUnit->GetLocals(), pcName->mpcSymbol, psType, CPPVS_Local, 0);
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
bool CCPPDesugarings::RangeIterators(CCPPSyntaxRangeForStatement* pcFor, SCPP65Type** ppsBegin, SCPP65Type** ppsEnd, CCPPFunction** ppcBegin, CCPPFunction** ppcEnd, bool* pbMember)
{
	SCPPRangeFor*	psRangeFor;
	SCPP65Type*		psRanged;
	CCPPFunction*	pcBegin;
	CCPPFunction*	pcEnd;
	bool			bMember;

	psRangeFor = GetRangeFor(pcFor);
	psRanged = mpcUnit->GetVariable(psRangeFor->pcRange)->mpsType->psBase;
	pcBegin = NULL;
	pcEnd = NULL;
	bMember = false;
	if (psRanged->IsArray())
	{
		if (psRanged->iCount <= 0)
		{
			return mpcUnit->mcDiagnostics.Error(pcFor->mpcRange, "An array that is a range must have a length");
		}
		*ppsBegin = mpcUnit->mcTypes.Pointer(psRanged->psBase);
		*ppsEnd = *ppsBegin;
	}
	else
	{
		if (!psRanged->IsClass() || !mpcUnit->GetClasses()->CompleteClass(psRanged->pcClass, pcFor->mpcRange))
		{
			return mpcUnit->mcDiagnostics.Error(pcFor->mpcRange, "A range must be an array or an object with begin and end");
		}
		bMember = mpcUnit->GetNames()->HasMember(psRanged->pcClass, mpcUnit->mpcTokeniser->GetIdentifierID((char*)"begin")) || mpcUnit->GetNames()->HasMember(psRanged->pcClass, mpcUnit->mpcTokeniser->GetIdentifierID((char*)"end"));
		pcBegin = mpcUnit->mcBinder.BindRangeFunction(mpcUnit->mpcTokeniser->GetIdentifierID((char*)"begin"), bMember ? psRanged->pcClass : NULL, psRangeFor->pcRangeOfBegin, pcFor);
		pcEnd = mpcUnit->mcBinder.BindRangeFunction(mpcUnit->mpcTokeniser->GetIdentifierID((char*)"end"), bMember ? psRanged->pcClass : NULL, psRangeFor->pcRangeOfEnd, pcFor);
		if (!pcBegin || !pcEnd)
		{
			return mpcUnit->mcDiagnostics.Error(pcFor->mpcRange, "A range must be an array or an object with begin and end");
		}
		*ppsBegin = pcBegin->mpsReturn->WithoutReference()->Unqualified();
		*ppsEnd = pcEnd->mpsReturn->WithoutReference()->Unqualified();
		if (!(*ppsBegin)->IsScalar() && !(*ppsBegin)->IsClass())
		{
			return mpcUnit->mcDiagnostics.Error(pcFor->mpcRange, "begin of a range must give a pointer or an object");
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


//////////////////////////////////////////////////////////////////////////
//
// The user-defined literal token of a literal (10_km, 'x'_c) or a string
// ("text"_s, or "te" "xt"_s), or NULL for one without a suffix.
//
//////////////////////////////////////////////////////////////////////////
CCTUserLiteral* CCPPDesugarings::UserLiteralToken(CCPPSyntax* pcLiteral)
{
	CCPPToken*	pcToken;
	int32		iToken;

	if (pcLiteral->GetKind() == CPPS_LiteralExpression)
	{
		pcToken = mpcUnit->mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcLiteral)->miLiteral);
		return pcToken->IsUserLiteral() ? (CCTUserLiteral*)pcToken : NULL;
	}
	if (pcLiteral->GetKind() == CPPS_StringExpression)
	{
		for (iToken = pcLiteral->miFirstToken; iToken <= pcLiteral->miLastToken; iToken++)
		{
			pcToken = mpcUnit->mpcTokeniser->Get(iToken);
			if (pcToken->IsUserString())
			{
				return (CCTUserLiteral*)pcToken;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The call a user-defined literal is, worked out once for its node: its
// suffix's literal operator and what it is given ([lex.ext]).  "text"_s is
// operator "" _s("text", 4) and 'c'_c operator "" _c('c').  12_k is
// operator "" _k(12ULL) if that operator takes an unsigned long long (1.5_k
// a long double), else operator "" _k("12") if it takes a const char*, else
// operator "" _k<'1', '2'>().  NULL if no literal operator takes it.
//
//////////////////////////////////////////////////////////////////////////
SCPPLiteralCall* CCPPDesugarings::GetLiteralCall(CCPPSyntax* pcLiteral)
{
	SCPPLiteralCall**				ppsCall;
	SCPPLiteralCall*				psCall;
	SCPPLiteralCall					sCall;
	CCTUserLiteral*					pcUser;
	CCPPSyntaxImplicitArgument*		pcArgument;
	CArraySyntax*					papcArguments;
	CArraySyntax					apcExplicit;
	CArrayUint8						auiBytes;
	CChars							szSpelling;
	SCPP65Type*						psFloat;
	int64							lliName;
	int64							lliValue;
	double							dValue;
	size								i;

	ppsCall = mcLiteralCalls.Get((int64)(size)pcLiteral);
	if (ppsCall)
	{
		return (*ppsCall)->pcFunction ? *ppsCall : NULL;
	}
	pcUser = UserLiteralToken(pcLiteral);
	if (!pcUser)
	{
		return NULL;
	}

	sCall.pcFunction = NULL;
	sCall.pcArguments = mpcUnit->mpcTree->GetMemory()->Create<CCPPSyntaxParenthesisedInitializer>(pcLiteral);
	sCall.pcArguments->SetTokens(pcLiteral->miFirstToken, pcLiteral->miLastToken);
	papcArguments = &sCall.pcArguments->macpElements;
	lliName = CPP_LITERAL_NAME(pcUser->miSuffix);
	if (pcLiteral->GetKind() == CPPS_StringExpression)
	{
		auiBytes.Init();
		if (mpcUnit->mcStrings.DecodeString(pcLiteral, &auiBytes))
		{
			LiteralArgument(pcLiteral, mpcUnit->mcTypes.Pointer(mpcUnit->mcTypes.Const(mpcUnit->mcStrings.StringElementType(pcLiteral))), papcArguments)->mpcString = pcLiteral;
			pcArgument = LiteralArgument(pcLiteral, mpcUnit->mcTypes.Size(), papcArguments);
			pcArgument->mbConstant = true;
			pcArgument->mlliValue = (int64)(auiBytes.NumElements() / mpcUnit->mcStrings.StringElementBytes(pcLiteral));
			sCall.pcFunction = mpcUnit->mcBinder.FindLiteralOperator(lliName, NULL, papcArguments, pcLiteral);
		}
		auiBytes.Kill();
	}
	else if (mpcUnit->CharacterTokenType(pcUser->mpcLiteral))
	{
		//'c'_x is given a char, L'c'_x a wchar_t, u'c'_x a char16_t and U'c'_x a char32_t.  [lex.ext] 6
		pcArgument = LiteralArgument(pcLiteral, mpcUnit->CharacterTokenType(pcUser->mpcLiteral), papcArguments);
		pcArgument->mbConstant = mpcUnit->mcFolder.TokenConstant(pcUser->mpcLiteral, &pcArgument->mlliValue);
		sCall.pcFunction = mpcUnit->mcBinder.FindLiteralOperator(lliName, NULL, papcArguments, pcLiteral);
	}
	else if (pcUser->mpcLiteral->IsNumber())
	{
		psFloat = NULL;
		dValue = 0;
		if (mpcUnit->mcFolder.TokenConstant(pcUser->mpcLiteral, &lliValue))
		{
			pcArgument = LiteralArgument(pcLiteral, &mpcUnit->mcTypes.msUnsignedLongLong, papcArguments);
			pcArgument->mlliValue = lliValue;
		}
		else
		{
			pcArgument = LiteralArgument(pcLiteral, mpcUnit->mcTypes.LongDouble(), papcArguments);
			pcArgument->mdValue = mpcUnit->mcFolder.TokenFloat(pcUser->mpcLiteral, &dValue, &psFloat) ? dValue : 0;
		}
		pcArgument->mbConstant = true;
		sCall.pcFunction = mpcUnit->mcBinder.FindLiteralOperator(lliName, NULL, papcArguments, pcLiteral);

		//The raw operator, given the literal's spelling.
		if (!sCall.pcFunction)
		{
			papcArguments->ReInit();
			LiteralArgument(pcLiteral, mpcUnit->mcTypes.Pointer(mpcUnit->mcTypes.Const(&mpcUnit->mcTypes.msChar)), papcArguments)->miSpelling = ((CCPPSyntaxLiteralExpression*)pcLiteral)->miLiteral;
			sCall.pcFunction = mpcUnit->mcBinder.FindLiteralOperator(lliName, NULL, papcArguments, pcLiteral);
		}

		//The template operator, given its characters as its arguments.
		if (!sCall.pcFunction)
		{
			papcArguments->ReInit();
			apcExplicit.Init();
			szSpelling.Init();
			UserLiteralSpelling(((CCPPSyntaxLiteralExpression*)pcLiteral)->miLiteral, &szSpelling);
			for (i = 0; i < szSpelling.Length(); i++)
			{
				pcArgument = LiteralArgument(pcLiteral, &mpcUnit->mcTypes.msChar, &apcExplicit);
				pcArgument->mbConstant = true;
				pcArgument->mlliValue = szSpelling.GetChar(i);
			}
			sCall.pcFunction = mpcUnit->mcBinder.FindLiteralOperator(lliName, &apcExplicit, papcArguments, pcLiteral);
			szSpelling.Kill();
			apcExplicit.Kill();
		}
	}
	psCall = (SCPPLiteralCall*)mcMemory.Add(sizeof(SCPPLiteralCall));
	*psCall = sCall;
	mcLiteralCalls.Put((int64)(size)pcLiteral, &psCall);
	return sCall.pcFunction ? psCall : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An argument of psType for a user-defined literal's call, added to
// papcArguments: the caller says what it holds.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxImplicitArgument* CCPPDesugarings::LiteralArgument(CCPPSyntax* pcLiteral, SCPP65Type* psType, CArraySyntax* papcArguments)
{
	CCPPSyntaxImplicitArgument*	pcArgument;

	pcArgument = mpcUnit->mpcTree->GetMemory()->Create<CCPPSyntaxImplicitArgument>(NULL);
	pcArgument->SetTokens(pcLiteral->miFirstToken, pcLiteral->miLastToken);
	pcArgument->mpsType = psType;
	papcArguments->Add(pcArgument);
	return pcArgument;
}


//////////////////////////////////////////////////////////////////////////
//
// A user-defined literal token's spelling without its suffix: 0x1F of
// 0x1F_k, what a raw literal operator is given.  [lex.ext] 3
//
//////////////////////////////////////////////////////////////////////////
void CCPPDesugarings::UserLiteralSpelling(int32 iToken, CChars* pszSpelling)
{
	CCPPToken*			pcToken;
	SPreprocessedToken*	psSource;
	int					iSuffix;

	pcToken = mpcUnit->mpcTokeniser->Get(iToken);
	psSource = mpcUnit->mpcTokeniser->GetSource(pcToken);
	iSuffix = 0;
	if (pcToken->IsUserLiteral())
	{
		mpcUnit->mpcTokeniser->GetIdentifier(((CCTUserLiteral*)pcToken)->miSuffix, &iSuffix);
	}
	pszSpelling->Append(psSource->szText, (int)psSource->uiLength - iSuffix);
}


//////////////////////////////////////////////////////////////////////////
//
// The list an aggregate is filled from, with the braces it leaves out put
// back: Nested n = { 1, 2, 3, 4, 5, 6, 7 } is { { 1, 2 }, 3, { { 4, 5 },
// { 6, 7 } } }.  A member that is itself an aggregate, given neither a list
// of its own nor an object that is one of it, takes as many of the values
// as it has members, and so on down.  pcList itself where nothing is left
// out, or it isn't a list, or psType no aggregate.  A list made is kept:
// asked for again, it is the same one.  [dcl.init.aggr] 12
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPDesugarings::BracedFor(SCPP65Type* psType, CCPPSyntax* pcList)
{
	CArraySyntax*				papcElements;
	SCPPBracedList*				psBraced;
	CCPPSyntaxInitializerList*	pcBraced;
	size						uiNext;
	size						i;

	if (!pcList || (pcList->GetKind() != CPPS_InitializerList) || !mpcUnit->GetTypeChecker()->IsAggregate(psType))
	{
		return pcList;
	}
	papcElements = mpcUnit->ListElements(pcList);
	if (!ElidesBraces(psType, papcElements))
	{
		return pcList;
	}
	for (i = 0; i < masBracedLists.NumElements(); i++)
	{
		psBraced = masBracedLists.Get(i);
		if ((psBraced->pcList == pcList) && (psBraced->psType == psType))
		{
			return psBraced->pcBraced;
		}
	}

	pcBraced = mpcUnit->mpcTree->GetMemory()->Create<CCPPSyntaxInitializerList>(pcList->mpcParent);
	pcBraced->SetTokens(pcList->miFirstToken, pcList->miLastToken);
	uiNext = 0;
	Regroup(psType, papcElements, &uiNext, pcBraced);
	for (; uiNext < papcElements->NumElements(); uiNext++)
	{
		//More values than the aggregate takes are still too many.
		pcBraced->macpElements.Add(papcElements->GetPtr(uiNext));
	}
	psBraced = masBracedLists.Add();
	psBraced->pcList = pcList;
	psBraced->psType = psType;
	psBraced->pcBraced = pcBraced;
	return pcBraced;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value is all of an aggregate member by itself, not the first of
// its members: an object of the member's class (or of one derived from it),
// or for an array of characters a string.  [dcl.init.aggr] 12
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDesugarings::InitialisesWhole(SCPP65Type* psMember, CCPPSyntax* pcValue)
{
	SCPP65Type*	psValue;

	if (psMember->IsArray())
	{
		return psMember->psBase->IsInteger() && mpcUnit->IsStringLiteral(mpcUnit->SkipParentheses(pcValue));
	}
	mpcUnit->mcDiagnostics.BeginQuiet();
	psValue = mpcUnit->GetTypeChecker()->ValueTypeOf(pcValue);
	mpcUnit->mcDiagnostics.EndQuiet();
	return psValue && psValue->IsClass() && mpcUnit->IsDerived(psValue->pcClass, psMember->pcClass, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a list leaves out the braces of a member of the aggregate it
// fills: a member that is an aggregate is given a value that is no list and
// not all of it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDesugarings::ElidesBraces(SCPP65Type* psType, CArraySyntax* papcElements)
{
	SCPP65Type*	psMember;
	CCPPSyntax*	pcValue;
	size		i;

	for (i = 0; i < papcElements->NumElements(); i++)
	{
		psMember = mpcUnit->GetTypeChecker()->AggregateMember(psType, i);
		pcValue = papcElements->GetPtr(i);
		if (!psMember)
		{
			return false;
		}
		if (mpcUnit->GetTypeChecker()->IsAggregate(psMember) && (pcValue->GetKind() != CPPS_InitializerList) && !InitialisesWhole(psMember, pcValue))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives pcInto the values an aggregate of psType takes from papcElements,
// from *puiNext on: one for each of its members, and for a member that is
// an aggregate with its braces left out a list of the values that member
// takes in turn.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDesugarings::Regroup(SCPP65Type* psType, CArraySyntax* papcElements, size* puiNext, CCPPSyntaxInitializerList* pcInto)
{
	CCPPSyntaxInitializerList*	pcInner;
	SCPP65Type*					psMember;
	CCPPSyntax*					pcValue;
	size						uiMember;

	for (uiMember = 0; *puiNext < papcElements->NumElements(); uiMember++)
	{
		psMember = mpcUnit->GetTypeChecker()->AggregateMember(psType, uiMember);
		if (!psMember)
		{
			return;
		}
		pcValue = papcElements->GetPtr(*puiNext);
		if (!mpcUnit->GetTypeChecker()->IsAggregate(psMember) || (pcValue->GetKind() == CPPS_InitializerList) || InitialisesWhole(psMember, pcValue))
		{
			pcInto->macpElements.Add(pcValue);
			(*puiNext)++;
			continue;
		}
		pcInner = mpcUnit->mpcTree->GetMemory()->Create<CCPPSyntaxInitializerList>(pcInto);
		pcInner->miFirstToken = pcValue->miFirstToken;
		Regroup(psMember, papcElements, puiNext, pcInner);
		pcInner->miLastToken = papcElements->GetPtr(*puiNext - 1)->miLastToken;
		pcInto->macpElements.Add(pcInner);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The expression a type-id is if it is just decltype(e)::name, which may
// name a member that is no type: the name as an expression, made once.
// NULL for any other type-id.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPDesugarings::DecltypeMember(CCPPSyntaxTypeId* pcType)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxIdExpression**	ppcMember;
	CCPPSyntaxIdExpression*		pcMember;

	pcSpecifiers = pcType->mpcSpecifiers;
	if (!pcSpecifiers || !pcSpecifiers->mpcType || (pcSpecifiers->mpcType->GetKind() != CPPS_NamedTypeSpecifier) || (pcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)) ||
		(pcType->mpcDeclarator && !pcType->mpcDeclarator->IsEmpty()))
	{
		return NULL;
	}
	pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType)->mpcName;
	if (!pcName->IsQualifiedByDecltype())
	{
		return NULL;
	}
	ppcMember = mcDecltypeMembers.Get((int64)(size)pcType);
	if (ppcMember)
	{
		return *ppcMember;
	}
	pcMember = mpcUnit->mpcTree->GetMemory()->Create<CCPPSyntaxIdExpression>(pcType->mpcParent);
	pcMember->SetTokens(pcType->miFirstToken, pcType->miLastToken);
	pcMember->mpcName = pcName;
	mcDecltypeMembers.Put((int64)(size)pcType, &pcMember);
	return pcMember;
}


//////////////////////////////////////////////////////////////////////////
//
// object.operator=(value) where the object's class declares no operator=:
// a call of the copy assignment operator the compiler declares, which is
// object = value (made once, and kept).  NULL for any other call.
// [over.oper] 4, [class.copy] 18
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxBinaryExpression* CCPPDesugarings::ImplicitAssignment(CCPPSyntaxCallExpression* pcCall)
{
	CCPPSyntaxMemberExpression*		pcMember;
	CCPPSyntaxBinaryExpression**	ppcAssignment;
	CCPPSyntaxBinaryExpression*		pcAssignment;
	SCPP65Type*						psType;
	int64							lliName;
	size							i;

	pcMember = (pcCall->mpcCallee && (pcCall->mpcCallee->GetKind() == CPPS_MemberExpression)) ? (CCPPSyntaxMemberExpression*)pcCall->mpcCallee : NULL;
	lliName = mpcUnit->GetOperatorName(CT_Assignment, false);
	if (!pcMember || pcMember->mbArrow || (pcCall->macpArguments.NumElements() != 1) || !pcMember->mpcName || (pcMember->mpcName->macpComponents.NumElements() != 1) ||
		(mpcUnit->GetFunctionName(pcMember->mpcName->GetLast()) != lliName))
	{
		return NULL;
	}
	ppcAssignment = mcImplicitAssignments.Get((int64)(size)pcCall);
	if (ppcAssignment)
	{
		return *ppcAssignment;
	}
	psType = mpcUnit->GetTypeChecker()->ValueTypeOf(pcMember->mpcObject);
	if (!psType || !psType->IsClass() || !mpcUnit->GetClasses()->CompleteClass(psType->pcClass, pcCall))
	{
		return NULL;
	}
	for (i = 0; i < psType->pcClass->mapcMembers.NumElements(); i++)
	{
		if (psType->pcClass->mapcMembers.GetPtr(i)->mlliName == lliName)
		{
			return NULL;
		}
	}
	pcAssignment = mpcUnit->mpcTree->GetMemory()->Create<CCPPSyntaxBinaryExpression>(pcCall);
	pcAssignment->SetTokens(pcCall->miFirstToken, pcCall->miLastToken);
	pcAssignment->meOperator = CT_Assignment;
	pcAssignment->mpcLeft = pcMember->mpcObject;
	pcAssignment->mpcRight = pcCall->macpArguments.GetPtr(0);
	mcImplicitAssignments.Put((int64)(size)pcCall, &pcAssignment);
	return pcAssignment;
}


//////////////////////////////////////////////////////////////////////////
//
// An argument the IR front end gives a value, pcFor's own iWhich'th (as a
// new expression in a new expression's placement arguments mustn't change
// its size's), of the type psType: the size a new expression's allocation
// function is given, or the address, and then the size, that delete gives
// a class's operator delete.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxImplicitArgument* CCPPDesugarings::ImplicitValue(CCPPSyntax* pcFor, SCPP65Type* psType, int iWhich)
{
	CCPPSyntaxImplicitArgument**	ppcValue;
	CCPPSyntaxImplicitArgument*		pcValue;
	int64							lliKey;

	//A node's address and those of the few bytes after it are no other node's.
	lliKey = (int64)(size)pcFor + iWhich;
	ppcValue = mcImplicitValues.Get(lliKey);
	if (ppcValue)
	{
		return *ppcValue;
	}
	pcValue = mpcUnit->mpcTree->GetMemory()->Create<CCPPSyntaxImplicitArgument>(NULL);
	pcValue->mpsType = psType;
	mcImplicitValues.Put(lliKey, &pcValue);
	return pcValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The int 0 a postfix operator function is called with, made the first
// time it is wanted (in no tree's nodes: it has no parent).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPDesugarings::ImplicitZero(void)
{
	if (!mpcImplicitZero)
	{
		mpcImplicitZero = mpcUnit->mpcTree->GetMemory()->Create<CCPPSyntaxImplicitArgument>(NULL);
		mpcImplicitZero->mpsType = &mpcUnit->mcTypes.msInt;
		mpcImplicitZero->mbConstant = true;
	}
	return mpcImplicitZero;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template std::initializer_list, if the program has declared it
// (<initializer_list>), or NULL.  [support.initlist]
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPDesugarings::InitializerListTemplate(void)
{
	CCPPTemplate*	pcTemplate;
	size			i;

	for (i = 0; !mpcUnit->mpcInitializerList && (i < mpcUnit->mapcTemplates.NumElements()); i++)
	{
		pcTemplate = mpcUnit->mapcTemplates.GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->mpcSymbol || (pcTemplate->mpcSymbol->mlliName < 0))
		{
			continue;
		}
		if (mpcUnit->mcBinder.IsStdName(pcTemplate->mpcSymbol->mlliName, pcTemplate->mpcNamespace, "initializer_list"))
		{
			mpcUnit->mpcInitializerList = pcTemplate;
		}
	}
	return mpcUnit->mpcInitializerList;
}


//////////////////////////////////////////////////////////////////////////
//
// E if psType (or what it refers to) is std::initializer_list<E>, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDesugarings::InitializerListElement(SCPP65Type* psType)
{
	CCPPClass*	pcClass;

	psType = psType ? psType->WithoutReference() : NULL;
	pcClass = (psType && psType->IsClass()) ? psType->pcClass : NULL;
	if (!pcClass || !pcClass->mpcTemplate || !InitializerListTemplate() || (pcClass->mpcTemplate->mpcPrimary != mpcUnit->mpcInitializerList) || !pcClass->mpcBindings || (pcClass->mpcBindings->macBindings.NumElements() != 1))
	{
		return NULL;
	}
	return pcClass->mpcBindings->macBindings.Get(0)->psType;
}


//////////////////////////////////////////////////////////////////////////
//
// std::initializer_list<psElement>, or NULL after an error if the program
// hasn't declared it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDesugarings::InitializerListOf(SCPP65Type* psElement, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*				pcClass;
	CArrayCPP65Types		apsDeduced;
	CChars					szArguments;

	if (!InitializerListTemplate())
	{
		mpcUnit->mcDiagnostics.Error(pcNode, "A list in braces is a std::initializer_list, which needs <initializer_list>");
		return NULL;
	}
	apsDeduced.Init();
	apsDeduced.Add(psElement);
	szArguments.Init();
	pcBindings = mpcUnit->mcInstantiator.BindArguments(mpcUnit->mpcInitializerList->mpcDeclaration, NULL, &apsDeduced, &szArguments, pcNode);
	pcClass = pcBindings ? mpcUnit->mcInstantiator.InstanceOfClass(mpcUnit->mpcInitializerList, pcBindings, &szArguments, pcNode) : NULL;
	szArguments.Kill();
	apsDeduced.Kill();
	return pcClass ? pcClass->mpsType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What auto is for a list in braces (auto l = {1, 2}, and a range-for's
// range): std::initializer_list of its elements' type, which they must all
// have (an array's a pointer, without its const).  [dcl.spec.auto] 7
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDesugarings::BracedListType(CCPPSyntax* pcList, CCPPSyntax* pcNode)
{
	CArraySyntax*	papcElements;
	SCPP65Type*		psElement;
	SCPP65Type*		psType;
	size			i;

	papcElements = mpcUnit->ListElements(pcList);
	if (papcElements->NumElements() == 0)
	{
		mpcUnit->mcDiagnostics.Error(pcNode, "auto can't be worked out from {}");
		return NULL;
	}
	psElement = NULL;
	for (i = 0; i < papcElements->NumElements(); i++)
	{
		psType = mpcUnit->mcTypeChecker.ValueTypeOf(papcElements->GetPtr(i));
		if (!psType)
		{
			mpcUnit->mcTypeChecker.ReportTypeOf(papcElements->GetPtr(i));
			return NULL;
		}
		psType = (psType->IsArray() || psType->IsFunction()) ? mpcUnit->mcTypes.Pointer(psType->ElementType()) : psType;
		psType = psType->Unqualified();
		if (psElement && !psElement->Same(psType))
		{
			mpcUnit->mcDiagnostics.Error(papcElements->GetPtr(i), "The values in braces aren't all of the same type, which auto needs");
			return NULL;
		}
		psElement = psType;
	}
	return InitializerListOf(psElement, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a std::initializer_list instance is laid out as the compiler
// fills it: a pointer to its elements, then their count (an integer) or
// the end (a pointer, *pbEnd).  <initializer_list>'s are.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDesugarings::InitializerListLayout(CCPPClass* pcClass, bool* pbEnd)
{
	SCPPField*	psFirst;
	SCPPField*	psSecond;

	if (!mpcUnit->GetClasses()->CompleteClass(pcClass, NULL) || (pcClass->macFields.NumElements() != 2) || (pcClass->macBases.NumElements() != 0) || pcClass->IsPolymorphic())
	{
		return false;
	}
	psFirst = pcClass->macFields.Get(0);
	psSecond = pcClass->macFields.Get(1);
	*pbEnd = psSecond->psType->IsPointer();
	return psFirst->psType->IsPointer() && !psFirst->IsBitField() && !psSecond->IsBitField() && (*pbEnd || psSecond->psType->IsInteger());
}

