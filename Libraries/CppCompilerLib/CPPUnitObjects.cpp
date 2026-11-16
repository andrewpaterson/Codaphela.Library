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
// The class template std::initializer_list, if the program has declared it
// (<initializer_list>), or NULL.  [support.initlist]
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::InitializerListTemplate(void)
{
	CCPPTemplate*	pcTemplate;
	CCPPScope*		pcNamespace;
	char*			szName;
	int				iLength;
	size			i;

	for (i = 0; !mpcInitializerList && (i < mapcTemplates.NumElements()); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->mpcSymbol || (pcTemplate->mpcSymbol->mlliName < 0))
		{
			continue;
		}
		szName = mpcTokeniser->GetIdentifier(pcTemplate->mpcSymbol->mlliName, &iLength);
		pcNamespace = pcTemplate->mpcNamespace;
		if ((iLength != 16) || (memcmp(szName, "initializer_list", 16) != 0) || !pcNamespace || !pcNamespace->mpcOwner || (pcNamespace->mpcOwner->mlliName < 0))
		{
			continue;
		}
		szName = mpcTokeniser->GetIdentifier(pcNamespace->mpcOwner->mlliName, &iLength);
		if ((iLength == 3) && (memcmp(szName, "std", 3) == 0) && (!pcNamespace->mpcParent || !pcNamespace->mpcParent->mpcOwner))
		{
			mpcInitializerList = pcTemplate;
		}
	}
	return mpcInitializerList;
}


//////////////////////////////////////////////////////////////////////////
//
// E if psType (or what it refers to) is std::initializer_list<E>, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::InitializerListElement(SCPP65Type* psType)
{
	CCPPClass*	pcClass;

	psType = psType ? psType->WithoutReference() : NULL;
	pcClass = (psType && psType->IsClass()) ? psType->pcClass : NULL;
	if (!pcClass || !pcClass->mpcTemplate || !InitializerListTemplate() || (pcClass->mpcTemplate->mpcPrimary != mpcInitializerList) || !pcClass->mpcBindings || (pcClass->mpcBindings->macBindings.NumElements() != 1))
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
SCPP65Type* CCPPUnit::InitializerListOf(SCPP65Type* psElement, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*				pcClass;
	CArrayCPP65Types		apsDeduced;
	CChars					szArguments;

	if (!InitializerListTemplate())
	{
		Error(pcNode, "A list in braces is a std::initializer_list, which needs <initializer_list>");
		return NULL;
	}
	apsDeduced.Init();
	apsDeduced.Add(psElement);
	szArguments.Init();
	pcBindings = BindArguments(mpcInitializerList->mpcDeclaration, NULL, &apsDeduced, &szArguments, pcNode);
	pcClass = pcBindings ? InstanceOfClass(mpcInitializerList, pcBindings, &szArguments, pcNode) : NULL;
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
SCPP65Type* CCPPUnit::BracedListType(CCPPSyntax* pcList, CCPPSyntax* pcNode)
{
	CArraySyntax*	papcElements;
	SCPP65Type*		psElement;
	SCPP65Type*		psType;
	size			i;

	papcElements = ListElements(pcList);
	if (papcElements->NumElements() == 0)
	{
		Error(pcNode, "auto can't be worked out from {}");
		return NULL;
	}
	psElement = NULL;
	for (i = 0; i < papcElements->NumElements(); i++)
	{
		psType = ValueTypeOf(papcElements->GetPtr(i));
		if (!psType)
		{
			ReportTypeOf(papcElements->GetPtr(i));
			return NULL;
		}
		psType = (psType->IsArray() || psType->IsFunction()) ? mcTypes.Pointer(psType->IsArray() ? psType->psBase : psType) : psType;
		psType = psType->Unqualified();
		if (psElement && !psElement->Same(psType))
		{
			Error(papcElements->GetPtr(i), "The values in braces aren't all of the same type, which auto needs");
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
bool CCPPUnit::InitializerListLayout(CCPPClass* pcClass, bool* pbEnd)
{
	SCPPField*	psFirst;
	SCPPField*	psSecond;

	if (!CompleteClass(pcClass, NULL) || (pcClass->macFields.NumElements() != 2) || (pcClass->macBases.NumElements() != 0) || pcClass->IsPolymorphic())
	{
		return false;
	}
	psFirst = pcClass->macFields.Get(0);
	psSecond = pcClass->macFields.Get(1);
	*pbEnd = psSecond->psType->IsPointer();
	return psFirst->psType->IsPointer() && !psFirst->IsBitField() && !psSecond->IsBitField() && (*pbEnd || psSecond->psType->IsInteger());
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


