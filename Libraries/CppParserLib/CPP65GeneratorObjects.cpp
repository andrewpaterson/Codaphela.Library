/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

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
size SCPP65Function::RequiredParameters(void)
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
bool CCPP65Generator::HasCopyConstructor(SCPP65Class* psClass)
{
	return GetCopyConstructor(psClass) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's copy constructor (taking one of its objects by reference, any other
// parameters having defaults), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::GetCopyConstructor(SCPP65Class* psClass)
{
	SCPP65Function*	psFunction;
	SCPP65Type*		psParameter;
	size			i;

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || (psFunction->eKind != CPP65FK_Constructor) || psFunction->bImplicit || (psFunction->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		psParameter = psFunction->mapsParameters.GetPtr(0);
		if (psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->psClass == psClass) && (psFunction->RequiredParameters() == 1))
		{
			return psFunction;
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
bool CCPP65Generator::IsTriviallyCopyable(SCPP65Class* psClass)
{
	SCPP65Type*		psType;
	size			i;

	if (HasCopyConstructor(psClass))
	{
		return false;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		if (!IsTriviallyCopyable(psClass->macBases.Get(i)->psClass))
		{
			return false;
		}
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psType = psClass->macFields.Get(i)->psType;
		psType = psType->IsArray() ? psType->psBase : psType;
		if (psType->IsClass() && !IsTriviallyCopyable(psType->psClass))
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
bool CCPP65Generator::IsMadeObject(CCPPSyntax* pcExpression, SCPP65Class* psClass)
{
	SCPP65Type*	psType;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression || !MakesObject(pcExpression))
	{
		return false;
	}
	psType = TypeOf(pcExpression);
	return psType && psType->IsClass() && (psType->psClass == psClass);
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
		return (pcBinary->meOperator != CT_Comma) && (pcBinary->meOperator != CT_Assignment) && (IsObject(pcBinary->mpcLeft) || IsObject(pcBinary->mpcRight));
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
SCPP65Class* CCPP65Generator::ObjectClass(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType->psClass;
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

	if (!mpsFunction)
	{
		return mpcDeclaringNamespace;
	}
	pcName = mpsFunction->pcDeclarator ? mpsFunction->pcDeclarator->GetName() : NULL;
	if (pcName && pcName->mpcLookup)
	{
		return pcName->mpcLookup;
	}
	if (mpsFunction->psClass && mpsFunction->psClass->pcSymbol)
	{
		return mpsFunction->psClass->pcSymbol->mpcScope;
	}
	return mpsFunction->pcNamespace;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator function (not a member) for the arguments: a declared one, or an
// instance of a function template of that name.  Those of the namespaces around
// the function being written (without the hiding names have) and those the
// arguments are associated with.  NULL, quietly, if none matches.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Function*					psFunction;
	SCPP65Template*					psTemplate;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxName*					pcName;
	CCPPScope*						pcLookup;
	CArrayScopes					apcNamespaces;
	size							uiErrors;
	size							i;
	bool							bDeclared;

	bDeclared = false;
	for (psFunction = FirstFunctionOfName(lliName); psFunction; psFunction = psFunction->psNextOfName)
	{
		if (!psFunction->psClass && !psFunction->psTemplate)
		{
			bDeclared = true;
		}
	}

	uiErrors = mcErrors.NumErrors();
	psFunction = NULL;
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
		psFunction = FindFunction(NULL, lliName, CPP65FK_Function, papcArguments, false, pcNode, -1, &apcNamespaces);
	}
	for (i = 0; !psFunction && (i < mapsTemplates.NumElements()); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if (psTemplate->bMember || (psTemplate->psPrimary != psTemplate) || !psTemplate->pcDefinition || (psTemplate->pcDefinition->GetKind() != CPPS_FunctionDefinition))
		{
			continue;
		}
		pcDefinition = (CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition;
		pcName = pcDefinition->mpcDeclarator ? pcDefinition->mpcDeclarator->GetName() : NULL;
		if (pcName && (pcName->macpComponents.NumElements() == 1) && (GetFunctionName(pcName->GetLast()) == lliName) && (apcNamespaces.Find(psTemplate->pcNamespace) != ARRAY_ELEMENT_NOT_FOUND))
		{
			psFunction = InstantiateFunction(psTemplate, NULL, papcArguments, pcNode);
		}
	}
	apcNamespaces.Kill();
	EndQuiet();
	mcErrors.Truncate(uiErrors);
	return psFunction;
}

