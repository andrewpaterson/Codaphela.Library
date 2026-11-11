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
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"
#include "LiteralHelper.h"


//////////////////////////////////////////////////////////////////////////
//
// Numbers, characters, true, false and nullptr.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Literal(CCPPSyntaxLiteralExpression* pcLiteral)
{
	CCPPToken*		pcToken;

	pcToken = mpcTokeniser->Get(pcLiteral->miLiteral);
	if (pcToken->IsNumber())
	{
		switch (((CCTNumber*)pcToken)->meType)
		{
		case CTNT_Int:
			return IntegerLiteralType((CCTInt*)pcToken);
		case CTNT_Short:
			return &mcTypes.msInt;
		case CTNT_LongLong:
			return ((CCTLongLong*)pcToken)->IsSigned() ? &mcTypes.msLongLong : &mcTypes.msUnsignedLongLong;
		case CTNT_Float:
			return &mcTypes.msFloat;
		case CTNT_Double:
			return &mcTypes.msDouble;
		case CTNT_LongDouble:
			return &mcTypes.msLongDouble;
		default:
			Error(pcLiteral, "This literal is not supported by the compiler yet");
			return NULL;
		}
	}
	if (pcToken->IsChar())
	{
		return &mcTypes.msChar;
	}
	if (pcToken->IsBool())
	{
		return &mcTypes.msBool;
	}
	return mcTypes.Pointer(&mcTypes.msVoid);
}


//////////////////////////////////////////////////////////////////////////
//
// An int literal is int (or unsigned int) if it fits and has no L suffix, otherwise long (or unsigned long): only
// different where int is narrower than 32 bits (the W65C816).  A hex or octal one too big for an int is an unsigned
// int if it fits one ([lex.icon]).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::IntegerLiteralType(CCTInt* pcInt)
{
	int64	lliValue;
	bool	bLong;

	if (pcInt->IsSigned())
	{
		lliValue = (int32)pcInt->mcValue;
		if ((mcTypes.msInt.iBytes < 4) && !pcInt->mbLong && !pcInt->mbDecimal && (lliValue > 32767) && (lliValue <= 0xFFFF))
		{
			return &mcTypes.msUnsignedInt;
		}
		bLong = pcInt->mbLong || (lliValue < -32768) || (lliValue > 32767);
		return bLong && (mcTypes.msInt.iBytes < 4) ? &mcTypes.msLong : &mcTypes.msInt;
	}
	lliValue = (uint32)pcInt->mcValue;
	bLong = pcInt->mbLong || (lliValue > 0xFFFF);
	return bLong && (mcTypes.msInt.iBytes < 4) ? &mcTypes.msUnsignedLong : &mcTypes.msUnsignedInt;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds uiLength bytes and a zero as string data, labelled Str and the number returned.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::AddString(char* szBytes, size uiLength)
{
	CChars*	pszBytes;
	size	i;

	pszBytes = maszStrings.Add();
	pszBytes->Init();
	for (i = 0; i < uiLength; i++)
	{
		pszBytes->Append((int)(uint8)szBytes[i]);
		pszBytes->Append(", ");
	}
	pszBytes->Append('0');
	return (int)maszStrings.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// __func__ and __FUNCTION__ are the function's name.  __PRETTY_FUNCTION__ and
// __FUNCSIG__ are its class and name ("CClass::Name"), not the whole signature.
// Returns false if lliName is not one of them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::FunctionNameText(int64 lliName, CChars* pszText)
{
	char*	szName;
	int		iLength;
	bool	bPretty;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	if (!szName)
	{
		return false;
	}
	bPretty = ((iLength == 19) && (memcmp(szName, "__PRETTY_FUNCTION__", 19) == 0)) || ((iLength == 11) && (memcmp(szName, "__FUNCSIG__", 11) == 0));
	if (!bPretty && !((iLength == 8) && (memcmp(szName, "__func__", 8) == 0)) && !((iLength == 12) && (memcmp(szName, "__FUNCTION__", 12) == 0)))
	{
		return false;
	}

	if (mpsFunction)
	{
		if (bPretty && mpsFunction->psClass)
		{
			szName = mpcTokeniser->GetIdentifier(mpsFunction->psClass->lliName, &iLength);
			pszText->Append(szName, iLength);
			pszText->Append("::");
		}
		if (mpsFunction->lliName >= 0)
		{
			szName = mpcTokeniser->GetIdentifier(mpsFunction->lliName, &iLength);
			pszText->Append(szName, iLength);
		}
		else
		{
			pszText->Append("operator");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True for __func__ and the other names of the enclosing function, which the parser
// declares as globals.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsFunctionName(CCPPSyntaxIdExpression* pcId)
{
	CCPPSymbol*	pcSymbol;
	CChars		szName;
	bool		bName;

	pcSymbol = pcId->mpcName->mpcSymbol;
	if ((pcId->mpcName->macpComponents.NumElements() != 1) || (pcSymbol && ((pcSymbol->meKind != CPPSK_Variable) || GetVariable(pcSymbol))))
	{
		return false;
	}
	szName.Init();
	bName = FunctionNameText(GetComponentName(pcId->mpcName->GetLast()), &szName);
	szName.Kill();
	return bName;
}


//////////////////////////////////////////////////////////////////////////
//
// f or &f (a function's name) when the value is for psTo, a pointer to a
// function: its address is the overload of f with psTo's function type.  NULL
// for any other expression or type.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPP65Generator::FunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo)
{
	if (!pcExpression || !psTo || !psTo->IsPointer() || !psTo->psBase || !psTo->psBase->IsFunction() || psTo->IsMemberPointer())
	{
		return NULL;
	}
	return FunctionNameOf(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// f or &f: the name of a function (overloaded or not), else NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPP65Generator::FunctionNameOf(CCPPSyntax* pcExpression)
{
	CCPPSyntax*				pcFunction;
	CCPPSyntaxIdExpression*	pcId;
	CCPPSymbol*				pcSymbol;

	pcFunction = SkipParentheses(pcExpression);
	if (pcFunction && (pcFunction->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcFunction)->meOperator == CT_BitwiseAnd))
	{
		pcFunction = SkipParentheses(((CCPPSyntaxUnaryExpression*)pcFunction)->mpcOperand);
	}
	if (!pcFunction || (pcFunction->GetKind() != CPPS_IdExpression))
	{
		return NULL;
	}
	pcId = (CCPPSyntaxIdExpression*)pcFunction;
	pcSymbol = pcId->mpcName->mpcSymbol;
	if (!pcSymbol || (pcSymbol->meKind != CPPSK_Function))
	{
		return NULL;
	}
	return pcId;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function of pcFunction's (unqualified) name has the function type
// psPointer (a pointer to a function, or a reference to one) points to: an
// overloaded name passed for a parameter of that type is that overload.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasOverloadOfType(CCPPSyntaxIdExpression* pcFunction, SCPP65Type* psPointer)
{
	SCPP65Function*		psFound;
	bool				bOverloaded;

	psPointer = psPointer->WithoutReference();
	if (!psPointer->IsPointer() || psPointer->IsMemberPointer() || !psPointer->psBase || !psPointer->psBase->IsFunction() || pcFunction->mpcName->IsQualifiedByClass())
	{
		return false;
	}
	psFound = FindAddressedFunction(pcFunction->mpcName, pcFunction->mpcName->mpcSymbol->mlliName, psPointer->psBase, pcFunction, &bOverloaded);
	return psFound && psPointer->psBase->Equals(mcTypes.Function(psFound->psReturn, &psFound->mapsParameters, psFound->bVariadic));
}


//////////////////////////////////////////////////////////////////////////
//
// The static member function of the name in the class, or else in its bases (the
// first base that has one).  *pbOverloaded is set when the name has several and
// psTarget does not pick one.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindStaticFunction(SCPP65Class* psClass, int64 lliName, SCPP65Type* psTarget, bool* pbOverloaded)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psFound;
	size				i;

	mpcStatistics->Count(CPPCC_FindStaticFunction);
	psFound = NULL;
	*pbOverloaded = false;
	for (psFunction = FirstFunctionOfName(lliName); psFunction; psFunction = psFunction->psNextOfName)
	{
		if ((psFunction->psClass == psClass) && psFunction->bStatic && (psFunction->eKind == CPP65FK_Function) && !psFunction->psTemplate)
		{
			if (psTarget && psTarget->Equals(mcTypes.Function(psFunction->psReturn, &psFunction->mapsParameters, psFunction->bVariadic)))
			{
				*pbOverloaded = false;
				return psFunction;
			}
			*pbOverloaded |= (psFound != NULL);
			psFound = psFunction;
		}
	}
	if (psFound)
	{
		return psFound;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psFound = FindStaticFunction(psClass->macBases.Get(i)->psClass, lliName, psTarget, pbOverloaded);
		if (psFound || *pbOverloaded)
		{
			return psFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A name reserved for the compiler (__debugbreak): the host compiler's
// intrinsics, which no header declares.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsIntrinsic(int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	return szName && (iLength > 2) && (szName[0] == '_') && (szName[1] == '_');
}


//////////////////////////////////////////////////////////////////////////
//
// True for the names of GCC's builtin functions, which are not declared.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsBuiltin(int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	return szName && (iLength > CPP65_BUILTIN_PREFIX_LENGTH) && (memcmp(szName, CPP65_BUILTIN_PREFIX, CPP65_BUILTIN_PREFIX_LENGTH) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Constant, without reporting an error when the expression isn't constant
// (most aren't: it is only looking), and without changing whether an
// earlier quiet evaluation failed.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::QuietConstant(CCPPSyntax* pcExpression, int64* plliValue)
{
	bool	bQuietFailed;
	bool	bConstant;

	bQuietFailed = mbQuietFailed;
	BeginQuiet();
	bConstant = Constant(pcExpression, plliValue);
	EndQuiet();
	mbQuietFailed = bQuietFailed;
	return bConstant;
}


#define CPP65_BCD_ONLY	"__builtin_bcd_add and __builtin_bcd_subtract are the W65C816's decimal mode: use BaseLib's BCDAdd and BCDSubtract"


//The builtins the compiler does: the type checker and the IR front end both look them up here.
static const SCPP65Builtin gasCPP65Builtins[] =
{
	{ "unreachable",	CPP65B_Unreachable,	0, CPP65BO_None,				NULL },
	{ "trap",			CPP65B_Unreachable,	0, CPP65BO_None,				NULL },
	{ "ia32_pause",		CPP65B_Pause,		0, CPP65BO_None,				NULL },
	{ "expect",			CPP65B_Expect,		2, CPP65BO_None,				NULL },
	{ "memcpy",			CPP65B_Memcpy,		3, CPP65BO_None,				NULL },
	{ "alloca",			CPP65B_Alloca,		1, CPP65BO_None,				NULL },
	{ "bcd_add",		CPP65B_BCDAdd,		4, CPP65BO_None,				CPP65_BCD_ONLY },
	{ "bcd_subtract",	CPP65B_BCDSubtract,	4, CPP65BO_None,				CPP65_BCD_ONLY },
	{ "ctz",			CPP65B_Ctz,			1, CPP65BO_UnsignedInt,			NULL },
	{ "ctzl",			CPP65B_Ctz,			1, CPP65BO_UnsignedLong,		NULL },
	{ "ctzll",			CPP65B_Ctz,			1, CPP65BO_UnsignedLongLong,	NULL },
	{ "clz",			CPP65B_Clz,			1, CPP65BO_UnsignedInt,			NULL },
	{ "clzl",			CPP65B_Clz,			1, CPP65BO_UnsignedLong,		NULL },
	{ "clzll",			CPP65B_Clz,			1, CPP65BO_UnsignedLongLong,	NULL },
	{ "popcount",		CPP65B_Popcount,	1, CPP65BO_UnsignedInt,			NULL },
	{ "popcountl",		CPP65B_Popcount,	1, CPP65BO_UnsignedLong,		NULL },
	{ "popcountll",		CPP65B_Popcount,	1, CPP65BO_UnsignedLongLong,	NULL },
	{ NULL,				CPP65B_Unreachable,	0, CPP65BO_None,				NULL },
};


//////////////////////////////////////////////////////////////////////////
//
// The builtin lliName names (__builtin_ and an entry of gasCPP65Builtins),
// NULL for one the compiler doesn't do (or a name that isn't a builtin's).
//
//////////////////////////////////////////////////////////////////////////
const SCPP65Builtin* CCPP65Generator::FindBuiltin(int64 lliName)
{
	char*	szName;
	int		iLength;
	size	i;

	if (!IsBuiltin(lliName))
	{
		return NULL;
	}
	szName = BuiltinName(lliName, &iLength);
	for (i = 0; gasCPP65Builtins[i].szName; i++)
	{
		if ((StrLen(gasCPP65Builtins[i].szName) == (size)iLength) && (memcmp(gasCPP65Builtins[i].szName, szName, iLength) == 0))
		{
			return &gasCPP65Builtins[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type ctz, clz or popcount counts the bits of (NULL for the others).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::BuiltinOperandType(const SCPP65Builtin* psBuiltin)
{
	switch (psBuiltin->eOperand)
	{
	case CPP65BO_UnsignedInt:		return &mcTypes.msUnsignedInt;
	case CPP65BO_UnsignedLong:		return mcTypes.Long(false);
	case CPP65BO_UnsignedLongLong:	return &mcTypes.msUnsignedLongLong;
	default:						return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A builtin's name without "__builtin_" (IsBuiltin) and its length.
//
//////////////////////////////////////////////////////////////////////////
char* CCPP65Generator::BuiltinName(int64 lliName, int* piLength)
{
	char*	szName;

	szName = mpcTokeniser->GetIdentifier(lliName, piLength);
	*piLength -= CPP65_BUILTIN_PREFIX_LENGTH;
	return szName + CPP65_BUILTIN_PREFIX_LENGTH;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call to lliName is memcpy (or __builtin_memcpy) with a constant count
// of 1 to CPP65_INLINE_MEMCPY bytes (CPP65_INLINE_MEMCPY_SIZE preferring size), which InlineMemcpy copies in
// place.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsInlineMemcpy(int64 lliName, CArraySyntax* papcArguments)
{
	char*	szName;
	int		iLength;
	int64	lliBytes;

	if ((lliName < 0) || (papcArguments->NumElements() != 3))
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	if (!szName || !(((iLength == 6) && (memcmp(szName, "memcpy", 6) == 0)) || ((iLength == 16) && (memcmp(szName, "__builtin_memcpy", 16) == 0))))
	{
		return false;
	}
	return Constant(papcArguments->GetPtr(2), &lliBytes) && (lliBytes > 0) && (lliBytes <= (mpcOptions->PrefersSize() ? CPP65_INLINE_MEMCPY_SIZE : CPP65_INLINE_MEMCPY));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the class or a base has a member function of the name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasMember(SCPP65Class* psClass, int64 lliName)
{
	SCPP65Function*	psFunction;
	size			i;

	if (!CompleteClass(psClass, NULL))
	{
		return false;
	}
	for (psFunction = FirstFunctionOfName(lliName); psFunction; psFunction = psFunction->psNextOfName)
	{
		if ((psFunction->psClass == psClass) && (psFunction->eKind == CPP65FK_Function))
		{
			return true;
		}
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		if (HasMember(psClass->macBases.Get(i)->psClass, lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psClass (or a base) has an operator function named lliName: a member function, or a member template taking
// uiArguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasOperator(SCPP65Class* psClass, int64 lliName, size uiArguments)
{
	return HasMember(psClass, lliName) || FindMemberTemplate(psClass, lliName, uiArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// The operator function of psClass (or a base) for the arguments: a member function that takes them, or else an
// instance of a member template.  A class without a member template reports errors as FindFunction does.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindOperator(SCPP65Class* psClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int iConstObject)
{
	SCPP65Template*	psTemplate;
	SCPP65Function*	psFunction;
	size			uiErrors;

	mpcStatistics->Count(CPPCC_FindOperator);
	psTemplate = FindMemberTemplate(psClass, lliName, papcArguments ? papcArguments->NumElements() : 0, iConstObject);
	if (!psTemplate)
	{
		return FindFunction(psClass, lliName, CPP65FK_Function, papcArguments, true, pcNode, iConstObject);
	}

	uiErrors = mcErrors.NumErrors();
	BeginQuiet();
	psFunction = FindFunction(psClass, lliName, CPP65FK_Function, papcArguments, true, pcNode, iConstObject);
	if (!psFunction)
	{
		psFunction = InstantiateFunction(psTemplate, NULL, papcArguments, pcNode);
	}
	EndQuiet();
	mcErrors.Truncate(uiErrors);
	return psFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The member of the kind eKind named lliName of psClass or of a base, and in ppsClass the class that declares it.  A
// specialisation has its own members: its symbol is its primary template's, whose members they aren't.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPP65Generator::ClassMember(SCPP65Class* psClass, int64 lliName, ECPPSymbolKind eKind, SCPP65Class** ppsClass)
{
	CCPPScope*	pcMembers;
	CCPPSymbol*	pcSymbol;
	size		i;

	pcMembers = psClass->pcSpecifier ? psClass->pcSpecifier->mpcScope : (psClass->pcSymbol ? psClass->pcSymbol->mpcMembers : NULL);
	pcSymbol = pcMembers ? pcMembers->FindHere(lliName) : NULL;
	if (pcSymbol && (pcSymbol->meKind == eKind))
	{
		*ppsClass = psClass;
		return pcSymbol;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		pcSymbol = ClassMember(psClass->macBases.Get(i)->psClass, lliName, eKind, ppsClass);
		if (pcSymbol)
		{
			return pcSymbol;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The enumerator Class::Name names.  See QualifiedMember.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPP65Generator::QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, SCPP65Class** ppsClass)
{
	return QualifiedMember(pcName, CPPSK_Enumerator, pcNode, ppsClass);
}


//////////////////////////////////////////////////////////////////////////
//
// The member of the kind eKind Class::Name names, where Class is a class, an instance of a class template, a typedef of
// one or a template parameter, and in ppsClass the class that declares it: a member of an instance can depend on its
// template arguments, so it is worked out with that class's bindings.  NULL if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPP65Generator::QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, SCPP65Class** ppsClass)
{
	CCPPSyntaxNameComponent*	pcQualifier;
	SCPP65Class*				psClass;
	CCPPSymbol*					pcSymbol;
	size						uiErrors;

	if (pcName->macpComponents.NumElements() < 2)
	{
		return NULL;
	}
	pcQualifier = pcName->GetQualifier();
	if (!pcQualifier || !pcQualifier->mpcSymbol)
	{
		return NULL;
	}
	switch (pcQualifier->mpcSymbol->meKind)
	{
	case CPPSK_Class:
	case CPPSK_ClassTemplate:
	case CPPSK_Typedef:
	case CPPSK_TypeParameter:
		break;
	default:
		return NULL;
	}

	uiErrors = mcErrors.NumErrors();
	BeginQuiet();
	psClass = GetComponentClass(pcQualifier, NULL, pcNode);
	pcSymbol = (psClass && CompleteClass(psClass, pcNode)) ? ClassMember(psClass, GetComponentName(pcName->GetLast()), eKind, ppsClass) : NULL;
	EndQuiet();
	mcErrors.Truncate(uiErrors);
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is object.*member or pointer->*member.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsMemberPointerOperator(CCPPSyntax* pcExpression)
{
	ECppTokens	eOperator;

	if (pcExpression->GetKind() != CPPS_BinaryExpression)
	{
		return false;
	}
	eOperator = ((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator;
	return (eOperator == CT_MemberFunctionPointerIndirection) || (eOperator == CT_MemberByPointerFunctionPointerIndirection);
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol for an element of the function parameter pack being generated: a
// variable with the pack's name, kept until the generator is killed.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPP65Generator::NewPackElement(CCPPSymbol* pcPack)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = NewMalloc<CCPPSymbol>();
	pcSymbol->Init(pcPack->mlliName, CPPSK_Variable, pcPack->mpcDeclaration, pcPack->mpcScope);
	mapcPackSymbols.Add(pcSymbol);
	mapcPackElements.Add(pcSymbol);
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call's arguments expand a pack (f(args...)).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasPackExpansion(CArraySyntax* papcArguments)
{
	size	i;

	for (i = 0; i < papcArguments->NumElements(); i++)
	{
		if (papcArguments->GetPtr(i)->GetKind() == CPPS_PackExpansionExpression)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// f(args...): the call's arguments are moved to papcArguments and replaced by
// themselves with args... replaced by a name for each of the pack's elements.
// RestoreArguments gives them back (the same call is in instances with other
// packs).  Only the function parameter pack itself can be expanded: otherwise
// the arguments are given back and the expansion is returned.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPP65Generator::ExpandPackArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments)
{
	CCPPSyntax*					pcArgument;
	CCPPSyntax*					pcPattern;
	CCPPSyntaxIdExpression*		pcId;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntaxNameComponent*	pcLast;
	CCPPSyntaxMemory*			pcMemory;
	size						i;
	size						j;

	mpcStatistics->Count(CPPCC_PackExpansions);
	papcArguments->Init();
	papcArguments->Copy(&pcCall->macpArguments);
	pcCall->macpArguments.ReInit();
	pcMemory = mpcTree->GetMemory();
	for (i = 0; i < papcArguments->NumElements(); i++)
	{
		pcArgument = papcArguments->GetPtr(i);
		if (pcArgument->GetKind() != CPPS_PackExpansionExpression)
		{
			pcCall->macpArguments.Add(pcArgument);
			continue;
		}
		pcPattern = SkipParentheses(((CCPPSyntaxPackExpansionExpression*)pcArgument)->mpcPattern);
		if (!mpcPackParameter || (pcPattern->GetKind() != CPPS_IdExpression) || (((CCPPSyntaxIdExpression*)pcPattern)->mpcName->mpcSymbol != mpcPackParameter))
		{
			RestoreArguments(pcCall, papcArguments);
			return pcArgument;
		}
		pcLast = ((CCPPSyntaxIdExpression*)pcPattern)->mpcName->GetLast();
		for (j = 0; j < mapcPackElements.NumElements(); j++)
		{
			mpcStatistics->Count(CPPCC_PackExpansionNodes, 3);
			pcId = pcMemory->Create<CCPPSyntaxIdExpression>(pcCall);
			pcId->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcName = pcMemory->Create<CCPPSyntaxName>(pcId);
			pcName->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcName->mpcSymbol = mapcPackElements.GetPtr(j);
			pcComponent = pcMemory->Create<CCPPSyntaxNameComponent>(pcName);
			pcComponent->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcComponent->meKind = pcLast->meKind;
			pcComponent->miName = pcLast->miName;
			pcComponent->mpcSymbol = pcName->mpcSymbol;
			pcName->macpComponents.Add(pcComponent);
			pcId->mpcName = pcName;
			pcCall->macpArguments.Add(pcId);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::RestoreArguments(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments)
{
	pcCall->macpArguments.ReInit();
	pcCall->macpArguments.Copy(papcArguments);
	papcArguments->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A call to a function whose declaration could not be generated: the declaration's
// errors are reported now.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ReportFailedDeclaration(int64 lliName)
{
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntax*					pcParent;
	size						i;

	for (i = 0; i < mapcFailed.NumElements(); i++)
	{
		pcDeclarator = (CCPPSyntaxDeclarator*)mapcFailed.GetPtr(i);
		if (!pcDeclarator->GetName() || (GetFunctionName(pcDeclarator->GetName()->GetLast()) != lliName))
		{
			continue;
		}
		pcParent = pcDeclarator->mpcParent;
		pcSpecifiers = NULL;
		if (pcParent && (pcParent->GetKind() == CPPS_InitDeclarator) && pcParent->mpcParent && (pcParent->mpcParent->GetKind() == CPPS_SimpleDeclaration))
		{
			pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcParent->mpcParent)->mpcSpecifiers;
		}
		else if (pcParent && (pcParent->GetKind() == CPPS_FunctionDefinition))
		{
			pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcParent)->mpcSpecifiers;
		}
		DeclareFunction(pcSpecifiers, pcDeclarator, false, NULL, false);
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// TypeOf, a reference as the type it refers to (NULL if it is wrong).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ValueTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = TypeOf(pcExpression);
	return psType ? psType->WithoutReference() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression (NULL if it is wrong), worked out without its code by
// CCPP65TypeChecker.  Its errors are dropped: they are reported where its code is
// written (ReportTypeOf).  An expression's type only depends on the template
// arguments and the function it is in, so it is remembered.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::TypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*		psType;
	SCPP65TypeOf*	psTypeOf;
	SCPP65TypeOf	sTypeOf;
	size			uiErrors;

	mpcStatistics->Count(CPPCC_TypeOf);
	psTypeOf = mcTypeOfs.Get((int64)(size)pcExpression);
	if (psTypeOf && (psTypeOf->psBindings == mpsBindings) && (psTypeOf->psFunction == mpsFunction))
	{
		return psTypeOf->psType;
	}

	mpcStatistics->Count(CPPCC_TypeOfWorkedOut);
	uiErrors = mcErrors.NumErrors();
	mpcStatistics->Begin(CPPCP_Types);
	psType = mcTypeChecker.Value(pcExpression);
	mpcStatistics->End(CPPCP_Types);
	mcErrors.Truncate(uiErrors);

	sTypeOf.psBindings = mpsBindings;
	sTypeOf.psFunction = mpsFunction;
	sTypeOf.psType = psType;
	mcTypeOfs.Put((int64)(size)pcExpression, &sTypeOf);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the place an lvalue is (an array stays an array), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::PlaceTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	BeginQuiet();
	mpcStatistics->Begin(CPPCP_Types);
	psType = mcTypeChecker.Place(pcExpression);
	mpcStatistics->End(CPPCP_Types);
	EndQuiet();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Reports what is wrong with an expression whose type the IR front end needed
// and TypeOf couldn't work out, in quiet mode too: a call that no function
// matches, or that more than one does, is then an error where it is ("No
// function matches the arguments": an unsigned short* passed for an unsigned
// long*), not only "doesn't support yet".
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ReportTypeOf(CCPPSyntax* pcExpression)
{
	int		iQuiet;
	bool	bQuietFailed;

	iQuiet = miQuiet;
	bQuietFailed = mbQuietFailed;
	miQuiet = 0;
	mpcStatistics->Begin(CPPCP_Types);
	mcTypeChecker.Value(pcExpression);
	mpcStatistics->End(CPPCP_Types);
	miQuiet = iQuiet;
	mbQuietFailed = bQuietFailed;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue can initialise a psTo without losing const or volatile
// (CCPP65TypeChecker::KeepsQualifiers), for the IR front end's initialisers
// and returns.  It is an error where it is, in quiet mode too, as ReportTypeOf's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CheckInitialiser(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	int		iQuiet;
	bool	bQuietFailed;
	bool	bKept;

	iQuiet = miQuiet;
	bQuietFailed = mbQuietFailed;
	miQuiet = 0;
	bKept = mcTypeChecker.KeepsQualifiers(pcValue, psTo);
	miQuiet = iQuiet;
	mbQuietFailed = bQuietFailed;
	return bKept;
}


//////////////////////////////////////////////////////////////////////////
//
// = and the compound assignments (+= ...).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsAssignmentOperator(ECppTokens eOperator)
{
	switch (eOperator)
	{
	case CT_Assignment:
	case CT_AssignmentAdd:
	case CT_AssignmentSubtract:
	case CT_AssignmentMultiply:
	case CT_AssignmentDivide:
	case CT_AssignmentModulus:
	case CT_AssignmentBitwiseAnd:
	case CT_AssignmentBitwiseOr:
	case CT_AssignmentBitwiseXor:
	case CT_AssignmentLeftShift:
	case CT_AssignmentRightShift:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Supported(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	if (psType->IsScalar() && (psType->iBytes > 8))
	{
		return Error(pcNode, "Integers wider than 64 bits are not supported by the compiler");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Works out an integer constant expression.  Each value is kept as its
// type keeps it (ConstantOfType): unsigned and narrow arithmetic wraps.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Constant(CCPPSyntax* pcExpression, int64* plliValue)
{
	CCPPToken*						pcToken;
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxBinaryExpression*		pcBinary;
	CCPPSyntaxConditionalExpression*	pcConditional;
	CCPPSyntaxSizeofExpression*		pcSizeof;
	CCPPSyntaxCastExpression*		pcCast;
	CCPPSyntaxConstructExpression*	pcConstruct;
	CArraySyntax*					papcElements;
	CCPPSyntaxIdExpression*			pcId;
	CCPPSyntax*						pcOperand;
	CCPPSymbol*						pcSymbol;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	SCPP65Variable*					psVariable;
	SCPP65Type*						psType;
	SCPP65Binding*					psBinding;
	SCPP65Bindings*					psSaved;
	SCPP65Class*					psClass;
	CCPPSymbol*						pcEnumerator;
	bool							bResult;
	int64							lliLeft;
	int64							lliRight;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}

	switch (pcExpression->GetKind())
	{
	case CPPS_LiteralExpression:
		pcToken = mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcExpression)->miLiteral);
		if (pcToken->IsNumber())
		{
			switch (((CCTNumber*)pcToken)->meType)
			{
			case CTNT_Int:
				*plliValue = ((CCTInt*)pcToken)->mcValue;
				if (((CCTInt*)pcToken)->IsSigned())
				{
					*plliValue = (int32)((CCTInt*)pcToken)->mcValue;
				}
				return true;
			case CTNT_Short:
				*plliValue = ((CCTShort*)pcToken)->mcValue;
				return true;
			case CTNT_LongLong:
				*plliValue = (int64)((CCTLongLong*)pcToken)->mcValue;
				return true;
			default:
				return false;
			}
		}
		if (pcToken->IsChar())
		{
			*plliValue = (int8)((CCTChar*)pcToken)->mcChar;
			return true;
		}
		if (pcToken->IsBool())
		{
			*plliValue = ((CCTBoolean*)pcToken)->mcBool ? 1 : 0;
			return true;
		}
		*plliValue = 0;
		return true;

	case CPPS_IdExpression:
		pcId = (CCPPSyntaxIdExpression*)pcExpression;
		pcSymbol = pcId->mpcName->mpcSymbol;
		if (!pcSymbol || (pcSymbol->meKind == CPPSK_Enumerator))
		{
			pcEnumerator = QualifiedEnumerator(pcId->mpcName, pcExpression, &psClass);
			if (pcEnumerator)
			{
				psSaved = mpsBindings;
				mpsBindings = psClass->psBindings;
				bResult = EnumeratorValue(pcEnumerator, plliValue);
				mpsBindings = psSaved;
				return bResult;
			}
		}
		if (!pcSymbol)
		{
			return false;
		}
		if (pcSymbol->meKind == CPPSK_Enumerator)
		{
			return EnumeratorValue(pcSymbol, plliValue);
		}
		if (pcSymbol->meKind == CPPSK_ValueParameter)
		{
			psBinding = FindBinding(pcSymbol);
			if (!psBinding)
			{
				return false;
			}
			*plliValue = psBinding->lliValue;
			return true;
		}

		//A const integer initialised with a constant.
		pcInit = (CCPPSyntaxInitDeclarator*)pcSymbol->mpcDeclaration;
		if ((pcSymbol->meKind != CPPSK_Variable) || !pcInit || (pcInit->GetKind() != CPPS_InitDeclarator) || !pcInit->mpcInitializer)
		{
			return false;
		}
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
		if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_SimpleDeclaration) || !(pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_CONSTEXPR)))
		{
			return false;
		}
		psVariable = GetVariable(pcSymbol);
		if (psVariable && !psVariable->psType->IsInteger())
		{
			return false;
		}
		return Constant(pcInit->mpcInitializer, plliValue);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (!Constant(pcUnary->mpcOperand, &lliLeft))
		{
			return false;
		}
		if (pcUnary->meOperator == CT_LogicalNot)
		{
			*plliValue = !lliLeft;
			return true;
		}
		//Otherwise the operand is promoted to the expression's type, and the result kept in it.
		psType = ConstantTypeOf(pcExpression);
		lliLeft = ConstantOfType(psType, lliLeft);
		switch (pcUnary->meOperator)
		{
		case CT_Add:			*plliValue = lliLeft;																	break;
		case CT_Subtract:		*plliValue = (int64)(0 - (uint64)lliLeft);	ConstantOverflow(pcExpression, *plliValue, true);	break;
		case CT_BitwiseNot:		*plliValue = ~lliLeft;																	break;
		default:				return false;
		}
		*plliValue = ConstantOfType(psType, *plliValue);
		return true;

	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if (!Constant(pcBinary->mpcLeft, &lliLeft) || !Constant(pcBinary->mpcRight, &lliRight))
		{
			return false;
		}
		return BinaryConstant(pcBinary, lliLeft, lliRight, plliValue);

	case CPPS_ConditionalExpression:
		pcConditional = (CCPPSyntaxConditionalExpression*)pcExpression;
		if (!Constant(pcConditional->mpcCondition, &lliLeft))
		{
			return false;
		}
		return Constant(lliLeft ? pcConditional->mpcTrue : pcConditional->mpcFalse, plliValue);

	case CPPS_CastExpression:
		pcCast = (CCPPSyntaxCastExpression*)pcExpression;
		psType = GetTypeIdType(pcCast->mpcType);
		if (!psType || !(psType->IsInteger() || psType->IsPointer()) || !Constant(pcCast->mpcOperand, &lliLeft))
		{
			return false;
		}
		*plliValue = ConstantOfType(psType, lliLeft);
		return true;

	case CPPS_ConstructExpression:
		pcConstruct = (CCPPSyntaxConstructExpression*)pcExpression;
		psType = GetTypeIdType(pcConstruct->mpcType);
		papcElements = ConstructElements(pcConstruct);
		if (!psType || !(psType->IsInteger() || psType->IsPointer()) || !papcElements || (papcElements->NumElements() > 1))
		{
			return false;
		}
		lliLeft = 0;
		if ((papcElements->NumElements() == 1) && !Constant(papcElements->GetPtr(0), &lliLeft))
		{
			return false;
		}
		*plliValue = ConstantOfType(psType, lliLeft);
		return true;

	case CPPS_TypeTraitExpression:
		return TypeTrait((CCPPSyntaxTypeTraitExpression*)pcExpression, plliValue);

	case CPPS_SizeofExpression:
		pcSizeof = (CCPPSyntaxSizeofExpression*)pcExpression;
		if ((pcSizeof->meKeyword != CT_sizeof) && (pcSizeof->meKeyword != CT_alignof))
		{
			return false;
		}
		if (pcSizeof->mpcType)
		{
			psType = GetTypeIdType(pcSizeof->mpcType);
		}
		else
		{
			//sizeof an array variable is the array's size, not a pointer's.
			pcOperand = SkipParentheses(pcSizeof->mpcOperand);
			psType = NULL;
			if (pcOperand && (pcOperand->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol)
			{
				psVariable = GetVariable(((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol);
				psType = psVariable ? psVariable->psType : NULL;
			}
			if (!psType && pcOperand && ((pcOperand->GetKind() == CPPS_IdExpression) || (pcOperand->GetKind() == CPPS_MemberExpression)))
			{
				//A field (this's or an object's) that is an array.
				psType = PlaceTypeOf(pcOperand);
			}
			if (!psType)
			{
				psType = TypeOf(pcOperand);
			}
		}
		if (!psType || psType->IsVoid() || psType->IsFunction())
		{
			return false;
		}
		psType = psType->WithoutReference();
		if ((psType->IsArray() ? psType->psBase : psType)->IsClass() && !CompleteClass((psType->IsArray() ? psType->psBase : psType)->psClass, pcExpression))
		{
			return false;
		}
		*plliValue = (pcSizeof->meKeyword == CT_alignof) ? TypeAlignment(psType) : psType->iBytes;
		return true;

	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What is in the brackets or braces of Type(arguments) or Type{arguments}, or
// NULL if it isn't either.
//
//////////////////////////////////////////////////////////////////////////
CArraySyntax* CCPP65Generator::ConstructElements(CCPPSyntaxConstructExpression* pcConstruct)
{
	CCPPSyntax*	pcInitialiser;

	pcInitialiser = pcConstruct->mpcInitializer;
	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer))
	{
		return &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
	}
	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList))
	{
		return &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type Constant works pcExpression out in, or NULL (64 bit) if that is
// being found already: TypeOf can ask for the constant whose type it is
// finding.  Other constants on the way have their own types.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ConstantTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	if (mapcConstantTypes.Find(pcExpression) != ARRAY_ELEMENT_NOT_FOUND)
	{
		return NULL;
	}
	mapcConstantTypes.Add(pcExpression);
	psType = TypeOf(pcExpression);
	mapcConstantTypes.RemoveTail();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// lliValue as an integer (or pointer) of psType holds it: cut to its width,
// then sign or zero extended.  Unchanged for other types and NULL.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::ConstantOfType(SCPP65Type* psType, int64 lliValue)
{
	if (!psType || !(psType->IsInteger() || psType->IsPointer()))
	{
		return lliValue;
	}
	switch (psType->iBytes)
	{
	case 1:		return psType->bSigned ? (int64)(int8)lliValue : (int64)(uint8)lliValue;
	case 2:		return psType->bSigned ? (int64)(int16)lliValue : (int64)(uint16)lliValue;
	case 8:		return lliValue;
	default:	return psType->bSigned ? (int64)(int32)lliValue : (int64)(uint32)lliValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// left op right, of constants: the operands converted to their common type
// (the left's promoted for a shift), the arithmetic on that type (unsigned
// divides, shifts and compares as unsigned) and the result kept in the
// expression's type.  The sum's, product's and shift's exact values are
// checked for overflow.  False (not a constant) for a divide by 0, a shift
// by a negative count or by the type's width or more, and the most
// negative 64 bit value divided by -1.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::BinaryConstant(CCPPSyntaxBinaryExpression* pcBinary, int64 lliLeft, int64 lliRight, int64* plliValue)
{
	SCPP65Type*	psResult;
	SCPP65Type*	psOperands;
	SCPP65Type*	psLeft;
	SCPP65Type*	psRight;
	bool		bUnsigned;
	int			iBits;
	uint64		a;
	uint64		b;
	int64		lliResult;

	psResult = ConstantTypeOf(pcBinary);
	psOperands = psResult;
	switch (pcBinary->meOperator)
	{
	case CT_LogicalAnd:		*plliValue = lliLeft && lliRight;	return true;
	case CT_LogicalOr:		*plliValue = lliLeft || lliRight;	return true;
	case CT_EqualTo:
	case CT_NotEqualTo:
	case CT_LessThan:
	case CT_LessThanEqualTo:
	case CT_GreaterThan:
	case CT_GreaterThanEqualTo:
		psLeft = ConstantTypeOf(pcBinary->mpcLeft);
		psRight = ConstantTypeOf(pcBinary->mpcRight);
		psLeft = (psLeft && psLeft->IsReference()) ? psLeft->psBase : psLeft;
		psRight = (psRight && psRight->IsReference()) ? psRight->psBase : psRight;
		psOperands = (psLeft && psRight && psLeft->IsInteger() && psRight->IsInteger()) ? mcTypes.Arithmetic(psLeft, psRight) : NULL;
		break;
	default:
		break;
	}
	bUnsigned = psOperands && psOperands->IsInteger() && !psOperands->bSigned;
	lliLeft = ConstantOfType(psOperands, lliLeft);
	if ((pcBinary->meOperator == CT_LeftShift) || (pcBinary->meOperator == CT_RightShift))
	{
		iBits = (psOperands && psOperands->IsInteger()) ? psOperands->iBytes * 8 : 64;
		if ((lliRight < 0) || (lliRight >= iBits))
		{
			return false;
		}
	}
	else
	{
		lliRight = ConstantOfType(psOperands, lliRight);
	}
	a = (uint64)lliLeft;
	b = (uint64)lliRight;

	switch (pcBinary->meOperator)
	{
	case CT_Add:				lliResult = (int64)(a + b);		ConstantOverflow(pcBinary, lliResult, false);	break;
	case CT_Subtract:			lliResult = (int64)(a - b);		ConstantOverflow(pcBinary, lliResult, true);	break;
	case CT_Multiply:			lliResult = (int64)(a * b);		ConstantOverflow(pcBinary, lliResult, false);	break;
	case CT_LeftShift:			lliResult = (int64)(a << b);	ConstantOverflow(pcBinary, lliResult, false);	break;
	case CT_RightShift:			lliResult = bUnsigned ? (int64)(a >> b) : (lliLeft >> b);	break;
	case CT_BitwiseAnd:			lliResult = (int64)(a & b);		break;
	case CT_BitwiseOr:			lliResult = (int64)(a | b);		break;
	case CT_BitwiseXor:			lliResult = (int64)(a ^ b);		break;
	case CT_EqualTo:			*plliValue = a == b;													return true;
	case CT_NotEqualTo:			*plliValue = a != b;													return true;
	case CT_LessThan:			*plliValue = bUnsigned ? (a < b) : (lliLeft < lliRight);				return true;
	case CT_LessThanEqualTo:	*plliValue = bUnsigned ? (a <= b) : (lliLeft <= lliRight);			return true;
	case CT_GreaterThan:		*plliValue = bUnsigned ? (a > b) : (lliLeft > lliRight);				return true;
	case CT_GreaterThanEqualTo:	*plliValue = bUnsigned ? (a >= b) : (lliLeft >= lliRight);			return true;
	case CT_Divide:
	case CT_Modulus:
		if ((b == 0) || (!bUnsigned && (lliLeft == (int64)MIN_INT64) && (lliRight == -1)))
		{
			return false;
		}
		if (pcBinary->meOperator == CT_Divide)
		{
			lliResult = bUnsigned ? (int64)(a / b) : (lliLeft / lliRight);
		}
		else
		{
			lliResult = bUnsigned ? (int64)(a % b) : (lliLeft % lliRight);
		}
		break;
	default:
		return false;
	}
	*plliValue = ConstantOfType(psResult, lliResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if lliValue, an integer constant expression's exact value, doesn't
// fit its type: an int expression that is right with a 32 bit int can
// silently be wrong with the W65C816's 16 bit one ("12000 * 6").  Unsigned
// arithmetic wraps by definition; bUnsignedWraps says if this one is meant to
// (subtraction and negation are, a sum, product or shift that loses bits is
// taken as a mistake).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ConstantOverflow(CCPPSyntax* pcExpression, int64 lliValue, bool bUnsignedWraps)
{
	SCPP65Type*	psType;
	int			iBits;
	int64		lliMin;
	int64		lliMax;
	CChars		szMessage;

	if (miWarningChecks > 0)
	{
		return;
	}
	miWarningChecks++;
	psType = TypeOf(pcExpression);
	miWarningChecks--;
	if (!psType || !psType->IsInteger() || (psType->iBytes >= 8))
	{
		return;
	}
	iBits = psType->iBytes * 8;
	if (psType->bSigned)
	{
		lliMin = -((int64)1 << (iBits - 1));
		lliMax = ((int64)1 << (iBits - 1)) - 1;
	}
	else
	{
		if (bUnsignedWraps)
		{
			return;
		}
		lliMin = 0;
		lliMax = ((int64)1 << iBits) - 1;
	}
	if ((lliValue >= lliMin) && (lliValue <= lliMax))
	{
		return;
	}

	szMessage.Init("Integer overflow: the constant expression is ");
	szMessage.Append(lliValue)->Append(", which doesn't fit its ")->Append(iBits)->Append(" bit ")->Append(psType->bSigned ? "signed" : "unsigned")->Append(" type.");
	Warning(pcExpression, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if pcExpression is an integer constant that an implicit conversion to
// psTo truncates: it fits neither psTo's signed nor its unsigned range (so -1
// to an unsigned type and 0xFFFF to a 16 bit signed one are bit patterns, not
// warned about).  89037124 passed to an int parameter is.  With
// SetWarnNarrowing a non constant integer converted to a narrower type is
// warned about too: a 32 bit index stored in a 16 bit one.  A multiply or
// left shift converted to a wider type is warned about (WideningArithmetic).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ConversionOverflow(CCPPSyntax* pcExpression, SCPP65Type* psTo)
{
	SCPP65Type*	psFrom;
	int64		lliValue;
	int			iBits;
	bool		bConstant;
	CChars		szMessage;

	if (!psTo || !psTo->IsInteger() || (psTo->eKind == CPP65TK_Bool) || (miWarningChecks > 0))
	{
		return;
	}
	miWarningChecks++;
	psFrom = TypeOf(pcExpression);
	miWarningChecks--;
	if (!psFrom || !psFrom->IsInteger())
	{
		return;
	}

	//Most expressions aren't constant; looking must not leave an error.
	bConstant = QuietConstant(pcExpression, &lliValue);
	if (!bConstant)
	{
		WideningArithmetic(pcExpression, psFrom, psTo);
		if (mpcOptions->mbWarnNarrowing && (psFrom->eKind == CPP65TK_Integer) && (psFrom->iBytes > psTo->iBytes))
		{
			szMessage.Init("Integer conversion narrows: a ");
			szMessage.Append(psFrom->iBytes * 8)->Append(" bit value is converted to the ")->Append(psTo->iBytes * 8)->Append(" bit type and may be truncated.");
			Warning(pcExpression, szMessage.Text());
			szMessage.Kill();
		}
		return;
	}
	if (psTo->iBytes >= 8)
	{
		return;
	}

	iBits = psTo->iBytes * 8;
	if ((lliValue >= -((int64)1 << (iBits - 1))) && (lliValue <= ((int64)1 << iBits) - 1))
	{
		return;
	}

	szMessage.Init("Integer conversion overflow: the constant ");
	szMessage.Append(lliValue)->Append(" doesn't fit the ")->Append(iBits)->Append(" bit type it is converted to and is truncated.");
	Warning(pcExpression, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if pcExpression (of type psFrom) is a multiply or a left shift done
// in an integer type narrower than psTo, which it is implicitly converted
// to: the result may have wrapped before it was widened.  iKey = ui * 0x39e7
// with a 16 bit ui and a 64 bit iKey multiplies in 16 bits.  Converting an
// operand first ((int64)ui * 0x39e7) does the arithmetic in the wider type.
// An explicit cast of the result ((int64)(ui * 0x39e7)) is warned about too:
// it doesn't make the multiply wider.  Constants are ConstantOverflow's.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::WideningArithmetic(CCPPSyntax* pcExpression, SCPP65Type* psFrom, SCPP65Type* psTo)
{
	CCPPSyntaxBinaryExpression*	pcBinary;
	CChars						szMessage;
	int64						lliValue;
	bool						bConstant;

	if ((psFrom->eKind != CPP65TK_Integer) || (psTo->eKind != CPP65TK_Integer) || (psTo->iBytes <= psFrom->iBytes) || (miWarningChecks > 0))
	{
		return;
	}
	pcBinary = NarrowMultiply(pcExpression, psFrom);
	if (!pcBinary)
	{
		return;
	}

	bConstant = QuietConstant(pcBinary, &lliValue);
	if (bConstant)
	{
		return;
	}

	szMessage.Init("Integer arithmetic widened after it may have wrapped: the ");
	szMessage.Append(psFrom->iBytes * 8)->Append(" bit result of '")->Append((pcBinary->meOperator == CT_Multiply) ? "*" : "<<");
	szMessage.Append("' is converted to the ")->Append(psTo->iBytes * 8)->Append(" bit type.  Convert an operand first to do it in ")->Append(psTo->iBytes * 8)->Append(" bits.");
	Warning(pcExpression, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a shift's count is a constant not less than iBits, the width of
// the (promoted) value shifted: the result is undefined.  On the W65C816 an
// int is 16 bits, so a uint8 or int promoted to it and shifted by 16 or more
// ((uint32)(b << 16) with a uint8 b) is zero, not the 32 bit value meant.
// Converting the operand first ((uint32)b << 16) shifts in the wider type.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ShiftCount(CCPPSyntaxBinaryExpression* pcBinary, int iBits)
{
	CChars	szMessage;
	int64	lliCount;
	bool	bConstant;

	if (miWarningChecks > 0)
	{
		return;
	}
	bConstant = QuietConstant(pcBinary->mpcRight, &lliCount);
	if (!bConstant || (lliCount < iBits))
	{
		return;
	}

	szMessage.Init("Shift count ");
	szMessage.Append(lliCount)->Append(" is not less than the ")->Append(iBits)->Append(" bit width of the value shifted, so the result is undefined.  Convert the operand first to shift in a wider type.");
	Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The multiply or left shift WideningArithmetic warns about in pcExpression:
// the expression itself, or one under +, -, |, ^ or & that is done in the
// same type (psType): y * uiRowStride + x * uiPixelStride with 16 bit
// operands made a 16 bit offset before it was widened to the pointer's
// ptrdiff.  NULL if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxBinaryExpression* CCPP65Generator::NarrowMultiply(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxBinaryExpression*	pcFound;
	SCPP65Type*					psSide;
	int							i;

	pcInner = SkipParentheses(pcExpression);
	if (!pcInner || (pcInner->GetKind() != CPPS_BinaryExpression))
	{
		return NULL;
	}
	pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
	if ((pcBinary->meOperator == CT_Multiply) || (pcBinary->meOperator == CT_LeftShift))
	{
		return pcBinary;
	}
	if ((pcBinary->meOperator != CT_Add) && (pcBinary->meOperator != CT_Subtract) && (pcBinary->meOperator != CT_BitwiseOr) &&
		(pcBinary->meOperator != CT_BitwiseXor) && (pcBinary->meOperator != CT_BitwiseAnd))
	{
		return NULL;
	}

	//Only a side already the type the whole is done in: a wider side would have made the whole wider.
	for (i = 0; i < 2; i++)
	{
		miWarningChecks++;
		psSide = TypeOf((i == 0) ? pcBinary->mpcLeft : pcBinary->mpcRight);
		miWarningChecks--;
		if (!psSide || (psSide->eKind != CPP65TK_Integer) || (psSide->iBytes != psType->iBytes))
		{
			continue;
		}
		pcFound = NarrowMultiply((i == 0) ? pcBinary->mpcLeft : pcBinary->mpcRight, psType);
		if (pcFound)
		{
			return pcFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a for loop's counter (a variable its increment steps with ++,
// --, += or -=) is compared in its condition with a wider integer that isn't
// a constant: for (size i = 0; i < aoi.NumElements(); i++) with a 16 bit i
// and a 32 bit count wraps i at 65535 and goes round forever once the count
// passes it.  Converting the bound to the counter's type ((size)uiCount) says
// it fits.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::LoopCounterWidth(CCPPSyntaxForStatement* pcFor)
{
	CArrayVoidPtr	apcCounters;

	if (!pcFor->mpcCondition || !pcFor->mpcIncrement || (miWarningChecks > 0))
	{
		return;
	}
	apcCounters.Init();
	LoopCounters(pcFor->mpcIncrement, &apcCounters);
	if (apcCounters.NumElements() > 0)
	{
		LoopCounterComparisons(pcFor->mpcCondition, &apcCounters);
	}
	apcCounters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The variables pcIncrement steps (through commas): i++, --i, i += 2.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::LoopCounters(CCPPSyntax* pcIncrement, CArrayVoidPtr* papcCounters)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CCPPSymbol*					pcSymbol;

	pcInner = SkipParentheses(pcIncrement);
	if (!pcInner)
	{
		return;
	}
	pcSymbol = NULL;
	if (pcInner->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcInner;
		if ((pcUnary->meOperator == CT_Increment) || (pcUnary->meOperator == CT_Decrement))
		{
			pcSymbol = LoopCounterSymbol(pcUnary->mpcOperand);
		}
	}
	else if (pcInner->GetKind() == CPPS_BinaryExpression)
	{
		pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
		if (pcBinary->meOperator == CT_Comma)
		{
			LoopCounters(pcBinary->mpcLeft, papcCounters);
			LoopCounters(pcBinary->mpcRight, papcCounters);
		}
		else if ((pcBinary->meOperator == CT_AssignmentAdd) || (pcBinary->meOperator == CT_AssignmentSubtract))
		{
			pcSymbol = LoopCounterSymbol(pcBinary->mpcLeft);
		}
	}
	if (pcSymbol)
	{
		papcCounters->Add(pcSymbol);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The comparisons in pcCondition (through && and ||) of a counter with a
// wider integer that isn't a constant.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::LoopCounterComparisons(CCPPSyntax* pcCondition, CArrayVoidPtr* papcCounters)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntax*					pcCounter;
	CCPPSyntax*					pcBound;
	CCPPSymbol*					pcSymbol;
	SCPP65Type*					psCounter;
	SCPP65Type*					psBound;
	int64						lliValue;
	bool						bConstant;
	bool						bQuietFailed;
	CChars						szMessage;
	ECppTokens					eOperator;

	pcInner = SkipParentheses(pcCondition);
	if (!pcInner || (pcInner->GetKind() != CPPS_BinaryExpression))
	{
		return;
	}
	pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
	eOperator = pcBinary->meOperator;
	if ((eOperator == CT_LogicalAnd) || (eOperator == CT_LogicalOr))
	{
		LoopCounterComparisons(pcBinary->mpcLeft, papcCounters);
		LoopCounterComparisons(pcBinary->mpcRight, papcCounters);
		return;
	}
	if ((eOperator != CT_LessThan) && (eOperator != CT_LessThanEqualTo) && (eOperator != CT_GreaterThan) &&
		(eOperator != CT_GreaterThanEqualTo) && (eOperator != CT_NotEqualTo))
	{
		return;
	}

	pcSymbol = LoopCounterSymbol(pcBinary->mpcLeft);
	if (pcSymbol && (papcCounters->Find(pcSymbol) != ARRAY_ELEMENT_NOT_FOUND))
	{
		pcCounter = pcBinary->mpcLeft;
		pcBound = pcBinary->mpcRight;
	}
	else
	{
		pcSymbol = LoopCounterSymbol(pcBinary->mpcRight);
		if (!pcSymbol || (papcCounters->Find(pcSymbol) == ARRAY_ELEMENT_NOT_FOUND))
		{
			return;
		}
		pcCounter = pcBinary->mpcRight;
		pcBound = pcBinary->mpcLeft;
	}

	miWarningChecks++;
	psCounter = TypeOf(pcCounter);
	psBound = TypeOf(pcBound);
	miWarningChecks--;
	if (!psCounter || !psBound || (psCounter->eKind != CPP65TK_Integer) || (psBound->eKind != CPP65TK_Integer) || (psBound->iBytes <= psCounter->iBytes))
	{
		return;
	}

	//A constant bound is ComparisonRange's, and a choice of constants (b ? 7 : 3) is two of them.
	bQuietFailed = mbQuietFailed;
	BeginQuiet();
	pcInner = SkipParentheses(pcBound);
	if (pcInner && (pcInner->GetKind() == CPPS_ConditionalExpression))
	{
		bConstant = Constant(((CCPPSyntaxConditionalExpression*)pcInner)->mpcTrue, &lliValue) &&
					Constant(((CCPPSyntaxConditionalExpression*)pcInner)->mpcFalse, &lliValue);
	}
	else
	{
		bConstant = Constant(pcBound, &lliValue);
	}
	EndQuiet();
	mbQuietFailed = bQuietFailed;
	if (bConstant || HasWarning(pcBinary, "Loop counter"))
	{
		return;
	}

	szMessage.Init("Loop counter narrower than its bound: the ");
	szMessage.Append(psCounter->iBytes * 8)->Append(" bit counter is compared with a ")->Append(psBound->iBytes * 8);
	szMessage.Append(" bit value and wraps before it can pass one that doesn't fit.  Widen the counter, or convert the bound if it always fits.");
	Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The variable pcExpression names (i, or a member m), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPP65Generator::LoopCounterSymbol(CCPPSyntax* pcExpression)
{
	CCPPSyntax*				pcInner;
	CCPPSyntaxIdExpression*	pcId;

	pcInner = SkipParentheses(pcExpression);
	if (!pcInner || (pcInner->GetKind() != CPPS_IdExpression))
	{
		return NULL;
	}
	pcId = (CCPPSyntaxIdExpression*)pcInner;
	if (!pcId->mpcName)
	{
		return NULL;
	}
	return pcId->mpcName->mpcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if one side of an integer comparison is a constant outside the range
// of the other side's type, so the comparison is always true or always false:
// a 16 bit size compared with 0xFFFFFFFF (a 32 bit "not found").  The constant
// is converted to the comparison's type first, so -1 compared with a 16 bit
// unsigned int is 0xFFFF and fine.  A side the comparison's type can't hold
// every value of (signed compared as unsigned, or a 32 bit unsigned enum
// compared as a 32 bit long) wraps, so it isn't looked at.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ComparisonRange(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*		psLeft;
	SCPP65Type*		psRight;
	SCPP65Type*		psVariable;
	SCPP65Type*		psCommon;
	CCPPSyntax*		pcConstant;
	int64			lliValue;
	int64			lliMin;
	int64			lliMax;
	int64			lliMask;
	int				iBits;
	bool			bConstant;
	bool			bQuietFailed;
	bool			bAlways;
	CChars			szMessage;

	if (miWarningChecks > 0)
	{
		return;
	}
	miWarningChecks++;
	psLeft = TypeOf(pcBinary->mpcLeft);
	psRight = TypeOf(pcBinary->mpcRight);
	miWarningChecks--;
	if (!psLeft || !psRight || (psLeft->eKind != CPP65TK_Integer) || (psRight->eKind != CPP65TK_Integer))
	{
		return;
	}

	//Most comparisons have no constant side; looking must not leave an error.
	bQuietFailed = mbQuietFailed;
	BeginQuiet();
	pcConstant = pcBinary->mpcRight;
	psVariable = psLeft;
	bConstant = Constant(pcConstant, &lliValue);
	if (!bConstant)
	{
		pcConstant = pcBinary->mpcLeft;
		psVariable = psRight;
		bConstant = Constant(pcConstant, &lliValue) && !Constant(pcBinary->mpcRight, &lliMin);
	}
	else if (Constant(pcBinary->mpcLeft, &lliMin))
	{
		bConstant = false;
	}
	EndQuiet();
	mbQuietFailed = bQuietFailed;
	if (!bConstant)
	{
		return;
	}

	psCommon = mcTypes.Arithmetic(psLeft, psRight);
	if (!psCommon || (psCommon->iBytes >= 8) || (psVariable->bSigned && !psCommon->bSigned) || (!psVariable->bSigned && psCommon->bSigned && (psCommon->iBytes <= psVariable->iBytes)))
	{
		return;
	}

	//The constant as the comparison's type sees it.
	iBits = psCommon->iBytes * 8;
	lliMask = ((int64)1 << iBits) - 1;
	lliValue &= lliMask;
	if (psCommon->bSigned && (lliValue > (lliMask >> 1)))
	{
		lliValue -= lliMask + 1;
	}

	iBits = psVariable->iBytes * 8;
	if (psVariable->bSigned)
	{
		lliMin = -((int64)1 << (iBits - 1));
		lliMax = ((int64)1 << (iBits - 1)) - 1;
	}
	else
	{
		lliMin = 0;
		lliMax = ((int64)1 << iBits) - 1;
	}
	if ((lliValue >= lliMin) && (lliValue <= lliMax))
	{
		ComparisonAtLimit(pcBinary, pcConstant, psVariable, lliValue, lliMin, lliMax);
		return;
	}

	//Out of range: the result is the same for every value of the variable side.
	switch (pcBinary->meOperator)
	{
	case CT_EqualTo:				bAlways = false;	break;
	case CT_NotEqualTo:				bAlways = true;		break;
	case CT_LessThan:
	case CT_LessThanEqualTo:		bAlways = (lliValue > lliMax) == (pcConstant == pcBinary->mpcRight);	break;
	case CT_GreaterThan:
	case CT_GreaterThanEqualTo:		bAlways = (lliValue < lliMin) == (pcConstant == pcBinary->mpcRight);	break;
	default:						return;
	}

	szMessage.Init("Comparison is always ");
	szMessage.Append(bAlways ? "true" : "false")->Append(": the constant ")->Append(lliValue)->Append(" is out of the range of the ")->Append(iBits)->Append(" bit ")->Append(psVariable->bSigned ? "signed" : "unsigned")->Append(" value it is compared with.");
	Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a <= (or >=) comparison's constant is the largest value the
// variable side's type holds (or a signed type's smallest), so it is always
// true: for (i = -32768; i <= 32767; i++) with a 16 bit int never ends.
// Always false comparisons at a limit (ui > MAX_UINT32, a guard for wider
// builds) and an unsigned value compared with 0 (u >= 0, written to say what
// is expected) aren't warned about.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ComparisonAtLimit(CCPPSyntaxBinaryExpression* pcBinary, CCPPSyntax* pcConstant, SCPP65Type* psVariable, int64 lliValue, int64 lliMin, int64 lliMax)
{
	ECppTokens	eOperator;
	bool		bAlways;
	CChars		szMessage;

	//As variable OP constant.
	eOperator = pcBinary->meOperator;
	if (pcConstant == pcBinary->mpcLeft)
	{
		switch (eOperator)
		{
		case CT_LessThan:				eOperator = CT_GreaterThan;			break;
		case CT_LessThanEqualTo:		eOperator = CT_GreaterThanEqualTo;	break;
		case CT_GreaterThan:			eOperator = CT_LessThan;			break;
		case CT_GreaterThanEqualTo:		eOperator = CT_LessThanEqualTo;		break;
		default:						break;
		}
	}

	if ((lliValue == lliMax) && (eOperator == CT_LessThanEqualTo))
	{
		bAlways = true;
	}
	else if (psVariable->bSigned && (lliValue == lliMin) && (eOperator == CT_GreaterThanEqualTo))
	{
		bAlways = true;
	}
	else
	{
		return;
	}

	szMessage.Init("Comparison is always ");
	szMessage.Append(bAlways ? "true" : "false")->Append(": the constant ")->Append(lliValue)->Append(" is the ")->Append((lliValue == lliMax) ? "largest" : "smallest");
	szMessage.Append(" value of the ")->Append(psVariable->iBytes * 8)->Append(" bit ")->Append(psVariable->bSigned ? "signed" : "unsigned")->Append(" value it is compared with.");
	Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//The compiler's type traits of one type.
enum ECPP65TypeTrait
{
	CPP65TT_None,
	CPP65TT_Class,
	CPP65TT_Union,
	CPP65TT_Enum,
	CPP65TT_Empty,
	CPP65TT_Polymorphic,
	CPP65TT_Abstract,
	CPP65TT_TrivialConstructor,
	CPP65TT_TrivialDestructor,
	CPP65TT_Trivial,
};


struct SCPP65TypeTraitName
{
	const char*			szName;
	ECPP65TypeTrait		eTrait;
};


static SCPP65TypeTraitName gasCPP65TypeTraits[] =
{
	{ "__is_class", CPP65TT_Class },
	{ "__is_union", CPP65TT_Union },
	{ "__is_enum", CPP65TT_Enum },
	{ "__is_empty", CPP65TT_Empty },
	{ "__is_polymorphic", CPP65TT_Polymorphic },
	{ "__is_abstract", CPP65TT_Abstract },
	{ "__is_trivially_constructible", CPP65TT_TrivialConstructor },
	{ "__has_trivial_constructor", CPP65TT_TrivialConstructor },
	{ "__is_trivially_destructible", CPP65TT_TrivialDestructor },
	{ "__has_trivial_destructor", CPP65TT_TrivialDestructor },
	{ "__is_pod", CPP65TT_Trivial },
	{ "__is_trivial", CPP65TT_Trivial },
	{ "__is_trivially_copyable", CPP65TT_Trivial },
};


//////////////////////////////////////////////////////////////////////////
//
// The type trait called pszName, CPP65TT_None if there is none this
// compiler knows.
//
//////////////////////////////////////////////////////////////////////////
static ECPP65TypeTrait CPP65FindTypeTrait(CChars* pszName)
{
	size	ui;

	for (ui = 0; ui < sizeof(gasCPP65TypeTraits) / sizeof(gasCPP65TypeTraits[0]); ui++)
	{
		if (pszName->Equals(gasCPP65TypeTraits[ui].szName))
		{
			return gasCPP65TypeTraits[ui].eTrait;
		}
	}
	return CPP65TT_None;
}


//////////////////////////////////////////////////////////////////////////
//
// A compiler type trait's value (see gasCPP65TypeTraits).  A class is
// abstract when one of its virtual functions is still pure, and trivial when
// nothing the compiler writes runs to make, copy or destroy one: no
// constructor, copy constructor, destructor or vtable.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TypeTrait(CCPPSyntaxTypeTraitExpression* pcTrait, int64* plliValue)
{
	SCPP65Type*		psType;
	SCPP65Class*	psClass;
	CChars			szTrait;
	ECPP65TypeTrait	eTrait;

	if ((pcTrait->macpArguments.NumElements() < 1) || (pcTrait->macpArguments.GetPtr(0)->GetKind() != CPPS_TypeId))
	{
		return false;
	}
	psType = GetTypeIdType((CCPPSyntaxTypeId*)pcTrait->macpArguments.GetPtr(0));
	if (!psType)
	{
		return false;
	}
	psClass = psType->IsClass() ? psType->psClass : NULL;
	if (psClass && !CompleteClass(psClass, pcTrait))
	{
		return false;
	}

	szTrait.Init();
	mpcTokeniser->GetSource(mpcTokeniser->Get(pcTrait->miName))->Append(&szTrait);
	eTrait = CPP65FindTypeTrait(&szTrait);
	szTrait.Kill();

	switch (eTrait)
	{
	case CPP65TT_Class:					*plliValue = psClass && !psClass->bUnion;					return true;
	case CPP65TT_Union:					*plliValue = psClass && psClass->bUnion;					return true;
	case CPP65TT_Enum:					*plliValue = psType->pcEnum != NULL;						return true;
	case CPP65TT_Empty:					*plliValue = psClass && psClass->bEmpty;					return true;
	case CPP65TT_Polymorphic:			*plliValue = psClass && IsPolymorphic(psClass);				return true;
	case CPP65TT_Abstract:				*plliValue = psClass && IsAbstract(psClass);				return true;
	case CPP65TT_TrivialConstructor:	*plliValue = !psClass || !NeedsConstructor(psClass);		return true;
	case CPP65TT_TrivialDestructor:		*plliValue = !psClass || !psClass->psDestructor;			return true;
	case CPP65TT_Trivial:
		*plliValue = !psClass || (!NeedsConstructor(psClass) && !psClass->psDestructor && !HasCopyConstructor(psClass) && !IsPolymorphic(psClass));
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An enumerator's type: its enum's (an unnamed enum's is its underlying type).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::EnumeratorType(CCPPSymbol* pcSymbol)
{
	CCPPSyntax*					pcEnumerator;
	CCPPSyntaxEnumSpecifier*	pcSpecifier;

	pcEnumerator = pcSymbol->mpcDeclaration;
	if (!pcEnumerator || !pcEnumerator->mpcParent || (pcEnumerator->mpcParent->GetKind() != CPPS_EnumSpecifier))
	{
		return &mcTypes.msInt;
	}
	pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcEnumerator->mpcParent;
	return EnumType(pcSpecifier->mpcSymbol, pcSpecifier);
}


//////////////////////////////////////////////////////////////////////////
//
// An enum's type (pcEnum's, or an unnamed enum's from pcSpecifier).  Its
// underlying type is the one given ("enum E : uint8"), int for a scoped enum,
// or else [dcl.enum] the first of int, unsigned int, long, unsigned long and
// long long that holds all its enumerators: TRIERROR = 0xffffffff makes
// TRISTATE an unsigned long on the W65C816, where int is 16 bits.
// pcSpecifier can be NULL or a forward declaration.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::EnumType(CCPPSymbol* pcEnum, CCPPSyntaxEnumSpecifier* pcSpecifier)
{
	SCPP65Type*				psType;
	SCPP65Type*				psBase;
	CCPPSyntaxEnumerator*	pcEnumerator;
	CCPPSymbol**			ppcMember;
	SMapIterator			sIter;
	CCPPSyntax*				pcDeclaration;
	int64					lliValue;
	int64					lliMin;
	int64					lliMax;
	int						iBits;
	int						iBytes;
	bool					bSigned;
	size					i;

	if (pcEnum)
	{
		psType = mcTypes.FindEnum(pcEnum);
		if (psType)
		{
			return psType;
		}
	}

	//The specifier with the enumerators: the symbol's declaration, or the parent of one of its enumerators.
	if ((!pcSpecifier || !pcSpecifier->mbBody) && pcEnum)
	{
		pcSpecifier = NULL;
		pcDeclaration = pcEnum->mpcDeclaration;
		if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_EnumSpecifier) && ((CCPPSyntaxEnumSpecifier*)pcDeclaration)->mbBody)
		{
			pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcDeclaration;
		}
		else if (pcEnum->mpcMembers && pcEnum->mpcMembers->mcSymbols.StartIteration(&sIter, NULL, NULL, (void**)&ppcMember, NULL) && ppcMember && *ppcMember)
		{
			pcDeclaration = (*ppcMember)->mpcDeclaration;
			if (pcDeclaration && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_EnumSpecifier))
			{
				pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcDeclaration->mpcParent;
			}
		}
	}

	iBytes = mcTypes.msInt.iBytes;
	bSigned = true;
	if (pcSpecifier && pcSpecifier->mpcBase)
	{
		psBase = GetTypeIdType(pcSpecifier->mpcBase);
		if (psBase && psBase->IsInteger())
		{
			iBytes = psBase->iBytes;
			bSigned = psBase->bSigned;
		}
	}
	else if (pcSpecifier && !pcSpecifier->mbScoped)
	{
		lliValue = 0;
		lliMin = 0;
		lliMax = 0;
		for (i = 0; i < pcSpecifier->macpEnumerators.NumElements(); i++)
		{
			pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
			if (pcEnumerator->mpcValue && !Constant(pcEnumerator->mpcValue, &lliValue))
			{
				break;
			}
			lliMin = (lliValue < lliMin) ? lliValue : lliMin;
			lliMax = (lliValue > lliMax) ? lliValue : lliMax;
			lliValue++;
		}

		iBits = mcTypes.msInt.iBytes * 8;
		if ((lliMin >= -((int64)1 << (iBits - 1))) && (lliMax < ((int64)1 << (iBits - 1))))
		{
			//int.
		}
		else if ((lliMin >= 0) && (lliMax < ((int64)1 << iBits)))
		{
			bSigned = false;
		}
		else if ((lliMin >= -((int64)1 << 31)) && (lliMax < ((int64)1 << 31)))
		{
			iBytes = 4;
		}
		else if ((lliMin >= 0) && (lliMax <= (int64)0xFFFFFFFF))
		{
			iBytes = 4;
			bSigned = false;
		}
		else
		{
			iBytes = 8;
		}
	}

	if (pcEnum)
	{
		return mcTypes.Enum(pcEnum, iBytes, bSigned);
	}
	return mcTypes.Integer(iBytes, bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
// An enumerator is one more than the one before it unless it is given a
// value: the nearest one at or before it that is given a value (or the
// first, 0) plus how far before it that one is.  Only that value is worked
// out.  Working out every value before it, each of which can name the one
// before it (urlmon.h's BINDSTATUS_CONNECTING = (BINDSTATUS_FINDINGRESOURCE
// + 1) and so on 78 times), doubled the work with each enumerator.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue)
{
	CCPPSyntaxEnumerator*		pcEnumerator;
	CCPPSyntaxEnumerator*		pcOther;
	CCPPSyntaxEnumSpecifier*	pcEnum;
	size						i;
	size						uiIndex;
	size						uiValued;
	size						uiNumEnumerators;
	int64						lliValue;

	pcEnumerator = (CCPPSyntaxEnumerator*)pcSymbol->mpcDeclaration;
	if (!pcEnumerator || !pcEnumerator->mpcParent || (pcEnumerator->mpcParent->GetKind() != CPPS_EnumSpecifier))
	{
		return false;
	}
	pcEnum = (CCPPSyntaxEnumSpecifier*)pcEnumerator->mpcParent;

	uiNumEnumerators = pcEnum->macpEnumerators.NumElements();
	for (uiIndex = 0; uiIndex < uiNumEnumerators; uiIndex++)
	{
		if (pcEnum->macpEnumerators.GetPtr(uiIndex) == pcEnumerator)
		{
			break;
		}
	}
	if (uiIndex == uiNumEnumerators)
	{
		return false;
	}

	//Without one given a value at or before it, it is as far from the first as the first's 0.
	lliValue = 0;
	uiValued = 0;
	for (i = uiIndex + 1; i > 0; i--)
	{
		pcOther = (CCPPSyntaxEnumerator*)pcEnum->macpEnumerators.GetPtr(i - 1);
		if (pcOther->mpcValue)
		{
			if (!Constant(pcOther->mpcValue, &lliValue))
			{
				return false;
			}
			uiValued = i - 1;
			break;
		}
	}
	*plliValue = lliValue + (int64)(uiIndex - uiValued);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPP65Generator::SkipParentheses(CCPPSyntax* pcExpression)
{
	while (pcExpression && (pcExpression->GetKind() == CPPS_ParenthesisedExpression))
	{
		pcExpression = ((CCPPSyntaxParenthesisedExpression*)pcExpression)->mpcExpression;
	}
	return pcExpression;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of a narrow string literal, without the zero at the end.  Adjacent
// literals are joined.  MSVC's __FUNCTION__, __FUNCSIG__ and __FUNCDNAME__ can be
// parts; they are the function's name, its class and name, and its label.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes)
{
	CCPPToken*	pcToken;
	int			iToken;
	int			iElementBytes;
	CChars		szName;
	char*		szIdentifier;
	int			iLength;
	size		i;

	iElementBytes = StringElementBytes(pcString);
	for (iToken = pcString->miFirstToken; iToken <= pcString->miLastToken; iToken++)
	{
		pcToken = mpcTokeniser->Get(iToken);
		if (pcToken->IsString())
		{
			if (!DecodeStringToken(pcString, (CCTString*)pcToken, iElementBytes, pauiBytes))
			{
				return false;
			}
			continue;
		}

		if (iElementBytes != 1)
		{
			return Error(pcString, "The function's name can't be part of a wide string literal");
		}
		szIdentifier = pcToken->IsIdentifier() ? mpcTokeniser->GetIdentifier(((CCTIdentifier*)pcToken)->miIdentifier, &iLength) : NULL;
		szName.Init();
		if (szIdentifier && (iLength == 13) && (memcmp(szIdentifier, "__FUNCDNAME__", 13) == 0))
		{
			if (mpsFunction)
			{
				AppendFunctionLabel(&szName, mpsFunction);
			}
		}
		else if (!szIdentifier || !FunctionNameText(((CCTIdentifier*)pcToken)->miIdentifier, &szName))
		{
			szName.Kill();
			return Error(pcString, "This part of a string literal is not supported by the compiler yet");
		}
		for (i = 0; i < szName.Length(); i++)
		{
			pauiBytes->Add((uint8)szName.GetChar(i));
		}
		szName.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends a string's element, the low byte first.
//
//////////////////////////////////////////////////////////////////////////
static void AppendStringElement(CArrayUint8* pauiBytes, uint32 uiElement, int iElementBytes)
{
	int		i;

	for (i = 0; i < iElementBytes; i++)
	{
		pauiBytes->Add((uint8)(uiElement >> (8 * i)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A character of a wide string: UTF-32 in 4 byte elements, otherwise UTF-16,
// where one past 0xFFFF is a surrogate pair.
//
//////////////////////////////////////////////////////////////////////////
static void AppendWideCharacter(CArrayUint8* pauiBytes, uint32 uiCharacter, int iElementBytes)
{
	if ((iElementBytes == 2) && (uiCharacter > 0xFFFF))
	{
		AppendStringElement(pauiBytes, 0xD800 + ((uiCharacter - 0x10000) >> 10), 2);
		AppendStringElement(pauiBytes, 0xDC00 + ((uiCharacter - 0x10000) & 0x3FF), 2);
		return;
	}
	AppendStringElement(pauiBytes, uiCharacter, iElementBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// The character whose UTF-8 *psz is at (a byte that doesn't start one is that
// byte), with *psz moved past it.
//
//////////////////////////////////////////////////////////////////////////
static uint32 DecodeUTF8(const char** psz, const char* szEnd)
{
	const uint8*	pu;
	uint32			uiCharacter;
	int				iMore;
	int				i;

	pu = (const uint8*)*psz;
	iMore = ((pu[0] & 0xE0) == 0xC0) ? 1 : ((pu[0] & 0xF0) == 0xE0) ? 2 : ((pu[0] & 0xF8) == 0xF0) ? 3 : 0;
	uiCharacter = (iMore == 0) ? pu[0] : (pu[0] & (0x3F >> iMore));
	for (i = 1; i <= iMore; i++)
	{
		if (((const char*)&pu[i] >= szEnd) || ((pu[i] & 0xC0) != 0x80))
		{
			(*psz)++;
			return pu[0];
		}
		uiCharacter = (uiCharacter << 6) | (pu[i] & 0x3F);
	}
	*psz += iMore + 1;
	return uiCharacter;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of an element of a string expression: its widest token's, as
// adjacent literals take the prefix any of them has.  [lex.string]
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::StringElementBytes(CCPPSyntax* pcString)
{
	CCPPToken*	pcToken;
	int			iToken;
	int			iBytes;

	iBytes = 1;
	for (iToken = pcString->miFirstToken; iToken <= pcString->miLastToken; iToken++)
	{
		pcToken = mpcTokeniser->Get(iToken);
		if (pcToken->IsString() && (((CCTString*)pcToken)->meEncoding == CTE_UTF32))
		{
			return 4;
		}
		if (pcToken->IsString() && ((((CCTString*)pcToken)->meEncoding == CTE_Wide) || (((CCTString*)pcToken)->meEncoding == CTE_UTF16)))
		{
			iBytes = 2;
		}
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a string expression's elements: char, or for a wide string
// wchar_t, char16_t or char32_t (the same unsigned integers of their widths).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::StringElementType(CCPPSyntax* pcString)
{
	int		iBytes;

	iBytes = StringElementBytes(pcString);
	return (iBytes == 1) ? &mcTypes.msChar : mcTypes.Integer(iBytes, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A decoded string's element uiElement.
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65Generator::StringElement(CArrayUint8* pauiBytes, int iElementBytes, size uiElement)
{
	uint32	uiValue;
	int		i;

	uiValue = 0;
	for (i = 0; i < iElementBytes; i++)
	{
		uiValue |= (uint32)pauiBytes->GetValue(uiElement * iElementBytes + i) << (8 * i);
	}
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the UTF-8 bytes of the character uiCharacter.
//
//////////////////////////////////////////////////////////////////////////
static void AppendUTF8(CArrayUint8* pauiBytes, uint32 uiCharacter)
{
	if (uiCharacter < 0x80)
	{
		pauiBytes->Add((uint8)uiCharacter);
	}
	else if (uiCharacter < 0x800)
	{
		pauiBytes->Add((uint8)(0xC0 | (uiCharacter >> 6)));
		pauiBytes->Add((uint8)(0x80 | (uiCharacter & 0x3F)));
	}
	else if (uiCharacter < 0x10000)
	{
		pauiBytes->Add((uint8)(0xE0 | (uiCharacter >> 12)));
		pauiBytes->Add((uint8)(0x80 | ((uiCharacter >> 6) & 0x3F)));
		pauiBytes->Add((uint8)(0x80 | (uiCharacter & 0x3F)));
	}
	else
	{
		pauiBytes->Add((uint8)(0xF0 | (uiCharacter >> 18)));
		pauiBytes->Add((uint8)(0x80 | ((uiCharacter >> 12) & 0x3F)));
		pauiBytes->Add((uint8)(0x80 | ((uiCharacter >> 6) & 0x3F)));
		pauiBytes->Add((uint8)(0x80 | (uiCharacter & 0x3F)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of one narrow string literal token (its escapes decoded as
// the tokeniser decodes them, DecodeEscape).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, int iElementBytes, CArrayUint8* pauiBytes)
{
	const char*		sz;
	const char*		szEnd;
	uint32			uiValue;
	char			cEscape;

	sz = pcToken->mszStart;
	while (*sz != '"')
	{
		if (*sz == 'R')
		{
			return Error(pcString, "Raw string literals are not supported by the compiler yet");
		}
		sz++;
	}
	sz++;
	szEnd = pcToken->mszEnd;

	while (sz < szEnd)
	{
		if (*sz != '\\')
		{
			if (iElementBytes == 1)
			{
				pauiBytes->Add((uint8)*sz);
				sz++;
			}
			else
			{
				AppendWideCharacter(pauiBytes, DecodeUTF8(&sz, szEnd), iElementBytes);
			}
			continue;
		}

		//An escape, as the tokeniser decodes it: \u and \U are the character's UTF-8, the others one byte.
		sz++;
		cEscape = *sz;
		if (!DecodeEscape(&sz, szEnd, &uiValue))
		{
			return Error(pcString, "Invalid escape sequence in a string literal");
		}
		if ((cEscape == 'u') || (cEscape == 'U'))
		{
			if (iElementBytes == 1)
			{
				AppendUTF8(pauiBytes, uiValue);
			}
			else
			{
				AppendWideCharacter(pauiBytes, uiValue, iElementBytes);
			}
		}
		else if ((iElementBytes < 4) && (uiValue >= ((uint32)1 << (8 * iElementBytes))))
		{
			return Error(pcString, (iElementBytes == 1) ? "The escape sequence's value is too large for a char" : "The escape sequence's value is too large for a wide character");
		}
		else
		{
			AppendStringElement(pauiBytes, uiValue, iElementBytes);
		}
	}
	return true;
}
