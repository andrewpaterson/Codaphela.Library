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
#include "CTIdentifier.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
// Objects as values: how a class is copied, whether an expression is an object
// (or makes one), and the operator functions that are not members.  Their code
// (temporaries, copies, objects made in place) is the IR front end's.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
// True if the expression makes an object of the class (a call, or an operator, that
// returns one): it can be made where the object it initialises is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsMadeObject(CCPPSyntax* pcExpression, CCPPClass* pcClass)
{
	SCPP65Type*	psType;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression || !MakesObject(pcExpression))
	{
		return false;
	}
	psType = TypeOf(pcExpression);
	return psType && psType->IsClass() && (psType->pcClass == pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is one that may make an object: a call, an operator with
// an object operand (an operator call; *pointer and the rest are not), or c ? x : y
// with object operands.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MakesObject(CCPPSyntax* pcExpression)
{
	CCPPSyntaxBinaryExpression*			pcBinary;
	CCPPSyntaxUnaryExpression*			pcUnary;
	CCPPSyntaxConditionalExpression*	pcConditional;

	switch (pcExpression->GetKind())
	{
	case CPPS_CallExpression:
		return true;
	case CPPS_ConditionalExpression:
		pcConditional = (CCPPSyntaxConditionalExpression*)pcExpression;
		return IsObject(pcConditional->mpcTrue) || IsObject(pcConditional->mpcFalse);
	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if (pcBinary->meOperator == CT_Comma)
		{
			return IsOverloadedComma(pcBinary);
		}
		return (pcBinary->meOperator != CT_Assignment) && (IsObject(pcBinary->mpcLeft) || IsObject(pcBinary->mpcRight));
	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		return IsObject(pcUnary->mpcOperand);
	case CPPS_CastExpression:
	case CPPS_NamedCastExpression:
		return CastClassType(pcExpression) != NULL;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The class a cast, (Class)x or static_cast<Class>(x), makes a temporary object of,
// or NULL for any other expression.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::CastClassType(CCPPSyntax* pcExpression)
{
	CCPPSyntaxTypeId*	pcType;
	SCPP65Type*			psType;

	if (pcExpression->GetKind() == CPPS_CastExpression)
	{
		pcType = ((CCPPSyntaxCastExpression*)pcExpression)->mpcType;
	}
	else if (pcExpression->GetKind() == CPPS_NamedCastExpression)
	{
		pcType = ((CCPPSyntaxNamedCastExpression*)pcExpression)->mpcType;
	}
	else
	{
		return NULL;
	}
	BeginQuiet();
	psType = mcDeclarators.GetTypeIdType(pcType);
	EndQuiet();
	return (psType && psType->IsClass()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the place a reference to the expression would refer to, or
// NULL if it is a value with no address (a literal, a sum, a call's int).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::PlaceOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psPlace;

	//A failure here is not one of the quiet code around it.
	BeginTrial();
	mpcStatistics->Begin(CPPCP_Types);
	psPlace = mcTypeChecker.Place(pcExpression);
	mpcStatistics->End(CPPCP_Types);
	EndTrial();
	return psPlace;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a reference parameter gets a temporary holding the argument, not
// the argument's address: a const reference to a type that isn't a class,
// and an argument that has no address or is of another type (const long&
// of a char, const char* const& of an array).  [dcl.init.ref]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument)
{
	SCPP65Type*	psPlace;

	if (!psReference->psBase->bConst || psReference->psBase->IsClass())
	{
		return false;
	}
	psPlace = PlaceOf(pcArgument);
	return !psPlace || !psReference->psBase->Equals(psPlace->WithoutReference());
}


//////////////////////////////////////////////////////////////////////////
//
// Whether left, right is a call of an operator, because an operand is an
// object and one matches.  Other operators with an object need one; a comma
// without one is the built in comma.  [over.match.oper] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsOverloadedComma(CCPPSyntaxBinaryExpression* pcBinary)
{
	CArraySyntax	apcArguments;
	SCPP65Type*		psType;
	CCPPFunction*	pcFunction;
	int64			lliName;
	size			uiErrors;

	if (!IsObject(pcBinary->mpcLeft) && !IsObject(pcBinary->mpcRight))
	{
		return false;
	}
	lliName = GetOperatorName(CT_Comma, false);
	uiErrors = mcErrors.NumErrors();
	BeginQuiet();
	pcFunction = NULL;
	psType = ValueTypeOf(pcBinary->mpcLeft);
	apcArguments.Init();
	if (psType && psType->IsClass() && CompleteClass(psType->pcClass, NULL) && HasOperator(psType->pcClass, lliName, 1))
	{
		apcArguments.Add(pcBinary->mpcRight);
		pcFunction = mcOverloads.FindOperator(psType->pcClass, lliName, &apcArguments, pcBinary, IsConstObject(pcBinary->mpcLeft));
		apcArguments.ReInit();
	}
	if (!pcFunction)
	{
		apcArguments.Add(pcBinary->mpcLeft);
		apcArguments.Add(pcBinary->mpcRight);
		pcFunction = mcOverloads.FindFreeOperator(lliName, &apcArguments, pcBinary);
	}
	apcArguments.Kill();
	EndQuiet();
	mcErrors.Truncate(uiErrors);
	return pcFunction != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is an object (or a reference to one).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsObject(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType && psType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// The class of an object expression (see IsObject).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65Generator::ObjectClass(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType->pcClass;
}


