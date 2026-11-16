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
#include "CPPUnit.h"


//////////////////////////////////////////////////////////////////////////
//
// Objects as values: how a class is copied, whether an expression is an object
// (or makes one), and the operator functions that are not members.  Their code
// (temporaries, copies, objects made in place) is the IR front end's.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is an rvalue: a value with no name (3, a + b, an
// object a call or a constructor makes) or something said to be about to
// go (what a function returning T&& returns, a cast to T&&, a member of
// either).  What has a name is an lvalue, a variable declared T&& too,
// but for a local object where it is returned.  An rvalue binds to T&&
// and not to T&; an lvalue to T& and not to T&&.  [basic.lval]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsRvalue(CCPPSyntax* pcExpression)
{
	CCPPSyntaxMemberExpression*	pcMember;
	SCPP65Type*					psType;

	pcExpression = SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_StringExpression:
		return false;
	case CPPS_IdExpression:
		return (PlaceOf(pcExpression) == NULL) || IsReturnedLocal((CCPPSyntaxIdExpression*)pcExpression);
	case CPPS_MemberExpression:
		pcMember = (CCPPSyntaxMemberExpression*)pcExpression;
		return !pcMember->mbArrow && IsRvalue(pcMember->mpcObject);
	default:
		break;
	}

	psType = DeclaredTypeOf(pcExpression);
	if (!psType)
	{
		return false;
	}
	if (psType->IsReference())
	{
		return psType->bRvalue;
	}
	if (psType->IsClass())
	{
		return MakesObject(pcExpression) || (pcExpression->GetKind() == CPPS_ConstructExpression);
	}
	return PlaceOf(pcExpression) == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcId is return object; of one of the function's own objects (a
// local or a parameter, not a reference): it is going, so it is moved to
// the result if its class has a move.  [class.copy] 32
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsReturnedLocal(CCPPSyntaxIdExpression* pcId)
{
	CCPPSyntax*		pcParent;
	CCPPBoundName	cBound;

	for (pcParent = pcId->mpcParent; pcParent && (pcParent->GetKind() == CPPS_ParenthesisedExpression); pcParent = pcParent->mpcParent)
	{
	}
	if (!pcParent || (pcParent->GetKind() != CPPS_ReturnStatement))
	{
		return false;
	}
	mcBinder.BindName(pcId, &cBound);
	return (cBound.meWhat == CPPBN_Variable) && ((cBound.mpcVariable->meStorage == CPPVS_Local) || (cBound.mpcVariable->meStorage == CPPVS_Parameter)) && cBound.mpcVariable->mpsType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression makes an object of the class (a call, or an operator, that
// returns one): it can be made where the object it initialises is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsMadeObject(CCPPSyntax* pcExpression, CCPPClass* pcClass)
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
bool CCPPUnit::MakesObject(CCPPSyntax* pcExpression)
{
	CCPPSyntaxBinaryExpression*			pcBinary;
	CCPPSyntaxUnaryExpression*			pcUnary;
	CCPPSyntaxConditionalExpression*	pcConditional;

	switch (pcExpression->GetKind())
	{
	case CPPS_CallExpression:
	case CPPS_LambdaExpression:
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
// The class of c ? x : y with objects: x's and y's if they are the same,
// otherwise the one the other is derived from (the derived one is converted
// to its base).  NULL if they aren't both objects, or neither class is the
// other's base.  [expr.cond] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::ConditionalClass(SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	psTrue = psTrue->WithoutReference();
	psFalse = psFalse->WithoutReference();
	if (!psTrue->IsClass() || !psFalse->IsClass())
	{
		return NULL;
	}
	if (IsDerived(psTrue->pcClass, psFalse->pcClass, NULL))
	{
		return psFalse->pcClass;
	}
	return IsDerived(psFalse->pcClass, psTrue->pcClass, NULL) ? psTrue->pcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a cast, (Class)x or static_cast<Class>(x), makes a temporary object of,
// or NULL for any other expression.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::CastClassType(CCPPSyntax* pcExpression)
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
// An expression of a class type that is an object where it is (named,
// pointed at, indexed, or a reference to one), not one to be made.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsObjectLvalue(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	pcExpression = SkipParentheses(pcExpression);
	psType = TypeOf(pcExpression);
	if (psType && psType->IsReference())
	{
		return true;
	}
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
	case CPPS_MemberExpression:
	case CPPS_SubscriptExpression:
		return true;
	case CPPS_UnaryExpression:
		return ((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator == CT_Multiply;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the place a reference to the expression would refer to, or
// NULL if it is a value with no address (a literal, a sum, a call's int).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::PlaceOf(CCPPSyntax* pcExpression)
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
// True if a reference gets a temporary holding the value, not the value's
// address: a const reference or T&&, and a value that has no address or
// is of another type (const long& of a char, const char* const& of an
// array, const Class& of what a Class is made from, a list in braces).  An
// object of the class, or of one derived from it, is referred to itself.
// [dcl.init.ref]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument)
{
	SCPP65Type*	psPlace;
	SCPP65Type*	psValue;

	if (!(psReference->psBase->bConst || psReference->bRvalue))
	{
		return false;
	}
	if (pcArgument->GetKind() == CPPS_InitializerList)
	{
		//const S& s = {1, 2}: what the list makes.  [dcl.init.list] 3
		return true;
	}
	if (psReference->psBase->IsClass())
	{
		psValue = ValueTypeOf(pcArgument);
		return psValue && !(psValue->IsClass() && ((psValue->pcClass == psReference->psBase->pcClass) || IsDerived(psValue->pcClass, psReference->psBase->pcClass, NULL)));
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
bool CCPPUnit::IsOverloadedComma(CCPPSyntaxBinaryExpression* pcBinary)
{
	CCPPBoundCall	cBound;
	size			uiErrors;
	bool			bOverloaded;

	if (!IsObject(pcBinary->mpcLeft) && !IsObject(pcBinary->mpcRight))
	{
		return false;
	}
	uiErrors = mcErrors.NumErrors();
	BeginQuiet();
	bOverloaded = mcBinder.BindOperator(CT_Comma, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary, &cBound, false);
	EndQuiet();
	mcErrors.Truncate(uiErrors);
	return bOverloaded;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is an object (or a reference to one).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsObject(CCPPSyntax* pcExpression)
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
CCPPClass* CCPPUnit::ObjectClass(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType->pcClass;
}


