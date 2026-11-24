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
#include "CPP65IRFrontEnd.h"
#include "CPPSyntaxTree.h"
#include "CPPUnit.h"
#include "CPPConstants.h"
#include "CPPSemanticPass.h"
#include "CPPTypeWriter.h"
#include "LiteralHelper.h"


//////////////////////////////////////////////////////////////////////////
//
// __func__ and __FUNCTION__ are the function's name.  __PRETTY_FUNCTION__ and
// __FUNCSIG__ are its class and name ("CClass::Name"), not the whole signature.
// Returns false if lliName is not one of them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::FunctionNameText(int64 lliName, CChars* pszText)
{
	char*	szName;
	int		iLength;
	bool	bPretty;

	bPretty = (lliName == mpcTokeniser->mlliPrettyFunction) || (lliName == mpcTokeniser->mlliFuncSig);
	if (!bPretty && (lliName != mpcTokeniser->mlliFunc) && (lliName != mpcTokeniser->mlliFunction))
	{
		return false;
	}

	if (mcContext.GetFunction())
	{
		if (bPretty && mcContext.GetFunction()->mpcClass)
		{
			szName = mpcTokeniser->GetIdentifier(mcContext.GetFunction()->mpcClass->mlliName, &iLength);
			pszText->Append(szName, iLength);
			pszText->Append("::");
		}
		if (!mcContext.GetFunction()->IsOperator())
		{
			szName = mpcTokeniser->GetIdentifier(mcContext.GetFunction()->mlliName, &iLength);
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
bool CCPPUnit::IsFunctionName(CCPPSyntaxIdExpression* pcId)
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
CCPPSyntaxIdExpression* CCPPUnit::FunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo)
{
	if (!pcExpression || !psTo || !psTo->IsPointer() || !psTo->psBase || !psTo->psBase->IsFunction() || psTo->IsMemberPointer())
	{
		return NULL;
	}
	return FunctionNameOf(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// &Class::f when the value is for psTo, a pointer to a member function:
// the member is the overload of f with psTo's function type.  NULL for any
// other expression or type.  [over.over] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::MemberFunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo)
{
	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression || !psTo || !psTo->IsMemberPointer() || !psTo->psBase->IsFunction() || (pcExpression->GetKind() != CPPS_UnaryExpression) || (((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator != CT_BitwiseAnd))
	{
		return NULL;
	}
	return MemberPointerName((CCPPSyntaxUnaryExpression*)pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The name &Class::member forms a pointer to a member from, or NULL: the
// operand of & as it is written, a name a class qualifies with no brackets
// about it (&(Class::member) is the member's own address, where it has
// one).  [expr.unary.op] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::MemberPointerName(CCPPSyntaxUnaryExpression* pcAddress)
{
	CCPPSyntax*	pcOperand;

	pcOperand = pcAddress->mpcOperand;
	if (!pcOperand || (pcOperand->GetKind() != CPPS_IdExpression) || !((CCPPSyntaxIdExpression*)pcOperand)->mpcName->IsQualifiedByClass())
	{
		return NULL;
	}
	return (CCPPSyntaxIdExpression*)pcOperand;
}


//////////////////////////////////////////////////////////////////////////
//
// The function suffix of the function a declarator's pointer or reference
// is to: void (*p)() noexcept's "() noexcept".  NULL for a declarator of
// anything else.  The specification is the pointer's, not its type's
// ([except.spec] 13), so it is read from where the pointer is declared.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxFunctionSuffix* CCPPUnit::PointedFunctionSuffix(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntax*	pcSuffix;

	while (pcDeclarator)
	{
		if (pcDeclarator->mpcNested && !pcDeclarator->GetFunctionSuffix() && (pcDeclarator->macpSuffixes.NumElements() > 0))
		{
			pcSuffix = pcDeclarator->macpSuffixes.GetPtr(0);
			if (pcSuffix->GetKind() == CPPS_FunctionSuffix)
			{
				return (CCPPSyntaxFunctionSuffix*)pcSuffix;
			}
		}
		pcDeclarator = pcDeclarator->mpcNested;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// f or &f: the name of a function (overloaded or not), else NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::FunctionNameOf(CCPPSyntax* pcExpression)
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
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate) && !pcId->mpcName->GetLast()->mbTemplateArguments)
	{
		//A function template's name without template arguments is an overload set too, its instances' arguments
		//deduced from the type wanted.  [over.over] 2
		return pcId;
	}
	if (!pcSymbol || (pcSymbol->meKind != CPPSK_Function))
	{
		return NULL;
	}
	return pcId;
}


//////////////////////////////////////////////////////////////////////////
//
// Constant, without reporting an error when the expression isn't constant
// (most aren't: it is only looking), and without changing whether an
// earlier quiet evaluation failed.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::QuietConstant(CCPPSyntax* pcExpression, int64* plliValue)
{
	bool	bConstant;

	mcDiagnostics.BeginTrial();
	bConstant = mcFolder.Constant(pcExpression, plliValue);
	mcDiagnostics.EndTrial();
	return bConstant;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a variable template's instance that is a const or
// constexpr integer (size<long>): its initialiser, worked out with its
// template's parameters its arguments.  False if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::VariableTemplateValue(CCPPSyntaxIdExpression* pcId, int64* plliValue)
{
	CCPPVariable*					pcVariable;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxInitDeclarator*		pcInit;
	bool							bResult;

	pcVariable = mcInstantiator.VariableInstance(pcId->mpcName, pcId);
	if (!pcVariable || !pcVariable->mpsType->IsInteger())
	{
		return false;
	}
	pcInit = mcInstantiator.VariableDeclarator(pcVariable);
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
	if (!pcSimple->mpcSpecifiers || !(pcSimple->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_CONSTEXPR)) || !pcInit->mpcInitializer)
	{
		return false;
	}
	mcContext.EnterBindings(pcVariable->mpcBindings);
	bResult = mcFolder.Constant(pcInit->mpcInitializer, plliValue);
	mcContext.LeaveBindings();
	if (bResult)
	{
		*plliValue = mcFolder.ConstantOfType(pcVariable->mpsType, *plliValue);
	}
	return bResult;
}




//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is object.*member or pointer->*member.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsMemberPointerOperator(CCPPSyntax* pcExpression)
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
// Whether pcNode is the operand of a decltype, or the right of a comma that
// is (in brackets or not): what decltype asks only the type of, so a call
// there makes no temporary.  [expr.call] 11, [dcl.type.simple] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsDecltypeOperand(CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcParent;

	for (pcParent = pcNode->mpcParent; pcParent; pcNode = pcParent, pcParent = pcParent->mpcParent)
	{
		switch (pcParent->GetKind())
		{
		case CPPS_DecltypeSpecifier:
			return ((CCPPSyntaxDecltypeSpecifier*)pcParent)->mpcOperand == pcNode;
		case CPPS_ParenthesisedExpression:
			break;
		case CPPS_BinaryExpression:
			if ((((CCPPSyntaxBinaryExpression*)pcParent)->meOperator != CT_Comma) || (((CCPPSyntaxBinaryExpression*)pcParent)->mpcRight != pcNode))
			{
				return false;
			}
			break;
		default:
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The parameter whose default argument pcNode is written in (not in the
// body of a lambda written there, which is a function of its own), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxParameter* CCPPUnit::DefaultArgumentOf(CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcParent;

	for (pcParent = pcNode->mpcParent; pcParent; pcNode = pcParent, pcParent = pcParent->mpcParent)
	{
		switch (pcParent->GetKind())
		{
		case CPPS_Parameter:
			return (((CCPPSyntaxParameter*)pcParent)->mpcDefault == pcNode) ? (CCPPSyntaxParameter*)pcParent : NULL;
		case CPPS_FunctionDefinition:
		case CPPS_LambdaExpression:
			return NULL;
		default:
			break;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcNode is written in an operand that isn't evaluated, of sizeof,
// decltype, noexcept or typeid, in the function (or lambda) it is in.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsInUnevaluatedOperand(CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcParent;

	for (pcParent = pcNode->mpcParent; pcParent; pcParent = pcParent->mpcParent)
	{
		switch (pcParent->GetKind())
		{
		case CPPS_SizeofExpression:
		case CPPS_DecltypeSpecifier:
		case CPPS_NoexceptExpression:
		case CPPS_TypeidExpression:
			return true;
		case CPPS_FunctionDefinition:
		case CPPS_LambdaExpression:
			return false;
		default:
			break;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is a function's name: the function itself, an
// lvalue, where it isn't converted to a pointer to it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::NamesFunction(CCPPSyntax* pcExpression)
{
	CCPPSymbol*	pcSymbol;

	if (pcExpression->GetKind() != CPPS_IdExpression)
	{
		return false;
	}
	pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
	return pcSymbol && (pcSymbol->meKind == CPPSK_Function);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue can initialise a psTo without losing const or volatile
// (CCPP65TypeChecker::KeepsQualifiers), for the IR front end's initialisers
// and returns.  It is an error where it is, in quiet mode too, as ReportTypeOf's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckInitialiser(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	bool	bKept;

	mcDiagnostics.BeginLoud();
	bKept = mcTypeChecker.KeepsQualifiers(pcValue, psTo);
	mcDiagnostics.EndLoud();
	return bKept;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue, the one value in braces that initialises a psTo, is not
// narrowed by its conversion (CCPP65TypeChecker::NotNarrowed), for the
// variables' initialisers the unit converts itself.  An error
// where it is, as CheckInitialiser's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckBracedValue(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	bool	bNotNarrowed;

	mcDiagnostics.BeginLoud();
	bNotNarrowed = mcTypeChecker.NotNarrowed(pcValue, psTo);
	mcDiagnostics.EndLoud();
	return bNotNarrowed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Supported(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	if (psType->IsScalar() && (psType->iBytes > 8))
	{
		return mcDiagnostics.Error(pcNode, "Integers wider than 64 bits are not supported by the compiler");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A local declared constexpr is initialised with a constant expression:
// one of an integer type is worked out here, and any other as a global's is
// (CCPPConstants::CheckLocal), and is an error (and false) where it has no
// value.  The local is still made when the function runs.
// [dcl.constexpr] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckConstexprLocal(CCPPVariable* pcVariable, CCPPSyntaxInitDeclarator* pcInit)
{
	CChars	szError;
	int64	lliValue;

	if (!pcInit->mpcInitializer)
	{
		return mcDiagnostics.ReportError(pcInit, "A constexpr variable needs an initialiser");
	}
	if (!pcVariable->mpsType->IsInteger())
	{
		return mpcConstants->CheckLocal(pcVariable, pcInit);
	}
	if (mcFolder.Constant(pcInit->mpcInitializer, &lliValue))
	{
		return true;
	}
	szError.Init();
	mcFolder.ConstantMessage("A constexpr variable's initialiser must be a constant", &szError);
	mcDiagnostics.ReportError(pcInit, szError.Text());
	szError.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What is in the brackets or braces of Type(arguments) or Type{arguments}, or
// NULL if it isn't either.
//
//////////////////////////////////////////////////////////////////////////
CArraySyntax* CCPPUnit::ConstructElements(CCPPSyntaxConstructExpression* pcConstruct)
{
	CCPPSyntax*	pcInitialiser;

	pcInitialiser = pcConstruct->mpcInitializer;
	if (pcInitialiser && ((pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer) || (pcInitialiser->GetKind() == CPPS_InitializerList)))
	{
		return mcInstantiator.ListElements(pcInitialiser);
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::SkipParentheses(CCPPSyntax* pcExpression)
{
	return pcExpression ? pcExpression->WithoutParentheses() : NULL;
}


