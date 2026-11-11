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
// The number of parameters a call must give: those before the trailing ones
// with defaults.
//
//////////////////////////////////////////////////////////////////////////
size CCPPFunction::RequiredParameters(void)
{
	size	uiRequired;

	uiRequired = mapsParameters.NumElements();
	while ((uiRequired > 0) && mapcDefaults.GetPtr(uiRequired - 1))
	{
		uiRequired--;
	}
	return uiRequired;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class declares a constructor that takes one of its objects (a copy
// constructor).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasCopyConstructor(CCPPClass* pcClass)
{
	return GetCopyConstructor(pcClass) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's copy constructor (taking one of its objects by reference, any other
// parameters having defaults), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPP65Generator::GetCopyConstructor(CCPPClass* pcClass)
{
	CCPPFunction*	pcFunction;
	SCPP65Type*		psParameter;
	size			i;

	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->meKind != CPPFK_Constructor) || pcFunction->mbImplicit || (pcFunction->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		psParameter = pcFunction->mapsParameters.GetPtr(0);
		if (psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->pcClass == pcClass) && (pcFunction->RequiredParameters() == 1))
		{
			return pcFunction;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if copying the class's objects copies their bytes: neither it nor its bases
// and fields have copy constructors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsTriviallyCopyable(CCPPClass* pcClass)
{
	SCPP65Type*		psType;
	size			i;

	if (HasCopyConstructor(pcClass))
	{
		return false;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (!IsTriviallyCopyable(pcClass->macBases.Get(i)->pcClass))
		{
			return false;
		}
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psType = pcClass->macFields.Get(i)->psType;
		psType = psType->IsArray() ? psType->psBase : psType;
		if (psType->IsClass() && !IsTriviallyCopyable(psType->pcClass))
		{
			return false;
		}
	}
	return true;
}


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
	psType = GetTypeIdType(pcType);
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
	bool		bQuietFailed;

	//A failure here is not one of the quiet code around it.
	bQuietFailed = mbQuietFailed;
	BeginQuiet();
	mpcStatistics->Begin(CPPCP_Types);
	psPlace = mcTypeChecker.Place(pcExpression);
	mpcStatistics->End(CPPCP_Types);
	EndQuiet();
	mbQuietFailed = bQuietFailed;
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
		pcFunction = FindOperator(psType->pcClass, lliName, &apcArguments, pcBinary, IsConstObject(pcBinary->mpcLeft));
		apcArguments.ReInit();
	}
	if (!pcFunction)
	{
		apcArguments.Add(pcBinary->mpcLeft);
		apcArguments.Add(pcBinary->mpcRight);
		pcFunction = FindFreeOperator(lliName, &apcArguments, pcBinary);
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


//////////////////////////////////////////////////////////////////////////
//
// The scope code being written is in, as far as the generator knows without
// its block: the one the function's name is declared from, or the namespace
// being declared in.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPP65Generator::CurrentLookupScope(void)
{
	CCPPSyntaxName*	pcName;

	if (!mpcFunction)
	{
		return mpcDeclaringNamespace;
	}
	pcName = mpcFunction->mpcDeclarator ? mpcFunction->mpcDeclarator->GetName() : NULL;
	if (pcName && pcName->mpcLookup)
	{
		return pcName->mpcLookup;
	}
	if (mpcFunction->mpcClass && mpcFunction->mpcClass->mpcSymbol)
	{
		return mpcFunction->mpcClass->mpcSymbol->mpcScope;
	}
	return mpcFunction->mpcNamespace;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator function (not a member) for the arguments: a declared one, or an
// instance of a function template of that name.  Those of the namespaces around
// the function being written (without the hiding names have) and those the
// arguments are associated with.  NULL, quietly, if none matches.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPP65Generator::FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPFunction*					pcFunction;
	CCPPTemplate*					pcTemplate;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxName*					pcName;
	CCPPScope*						pcLookup;
	CArrayScopes					apcNamespaces;
	size							uiErrors;
	size							i;
	bool							bDeclared;

	mpcStatistics->Count(CPPCC_FindFreeOperator);
	bDeclared = false;
	for (pcFunction = FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (!pcFunction->mpcClass && !pcFunction->mpcTemplate)
		{
			bDeclared = true;
		}
	}

	uiErrors = mcErrors.NumErrors();
	pcFunction = NULL;
	BeginQuiet();
	apcNamespaces.Init();
	pcLookup = CurrentLookupScope();
	if (pcLookup)
	{
		pcLookup->GatherEnclosingNamespaces(&apcNamespaces);
	}
	AddArgumentNamespaces(papcArguments, &apcNamespaces);
	NormaliseNamespaces(&apcNamespaces);
	if (apcNamespaces.Find(NULL) == ARRAY_ELEMENT_NOT_FOUND)
	{
		apcNamespaces.Add(NULL);
	}
	if (bDeclared)
	{
		pcFunction = FindFunction(NULL, lliName, CPPFK_Function, papcArguments, false, pcNode, -1, &apcNamespaces);
	}
	for (i = 0; !pcFunction && (i < mapcTemplates.NumElements()); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->mpcDefinition || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
		{
			continue;
		}
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition;
		pcName = pcDefinition->mpcDeclarator ? pcDefinition->mpcDeclarator->GetName() : NULL;
		if (pcName && (pcName->macpComponents.NumElements() == 1) && (GetFunctionName(pcName->GetLast()) == lliName) && (apcNamespaces.Find(pcTemplate->mpcNamespace) != ARRAY_ELEMENT_NOT_FOUND))
		{
			pcFunction = InstantiateFunction(pcTemplate, NULL, papcArguments, pcNode);
		}
	}
	apcNamespaces.Kill();
	EndQuiet();
	mcErrors.Truncate(uiErrors);
	return pcFunction;
}

