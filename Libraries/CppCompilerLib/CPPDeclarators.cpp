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
#include "CPPDeclarators.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeclarators::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDeclarators::Kill(void)
{
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type a declarator declares.  pcDeclarator may be NULL for the base type.  An
// array without a size gets it from pcInitialiser, and auto is what the initialiser
// makes it (AutoType).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::GetType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser)
{
	SCPP65Type*	psType;

	if (IsDecltypeAuto(pcSpecifiers))
	{
		psType = pcInitialiser ? DecltypeType(pcInitialiser, pcNode) : NULL;
		if (!pcInitialiser)
		{
			mpcDiagnostics->Error(pcNode, "decltype(auto) needs an initialiser to take its type from");
		}
		return psType;
	}
	psType = IsAuto(pcSpecifiers) ? AutoType(pcSpecifiers, pcDeclarator, pcNode, pcInitialiser) : GetBaseType(pcSpecifiers, pcNode);
	if (psType && pcSpecifiers && (pcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
	{
		//The base type's own qualifiers stay (a typedef of const int).
		psType = mpcUnit->GetTypes()->Qualified(psType, psType->bConst || (pcSpecifiers->muiFlags & CPPDS_CONST), psType->bVolatile || (pcSpecifiers->muiFlags & CPPDS_VOLATILE));
	}
	if (!psType || !pcDeclarator)
	{
		return psType;
	}
	psType = GetDeclaratorType(psType, pcDeclarator, pcNode, pcInitialiser);
	psType = psType ? CompletedArray(psType, pcInitialiser) : NULL;
	if (psType && pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_CONSTEXPR))
	{
		//A constexpr object is const: constexpr int* p is int* const.  A function and a reference are as they are.
		//[dcl.constexpr] 9
		psType = mpcUnit->GetTypes()->Qualified(psType, true, psType->bVolatile);
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// An array of unknown bound (a typedef of one: typedef int A[]; A a = {1,
// 2};) with the bound its initialiser gives it; any other type, or with no
// initialiser, as it is.  [dcl.array] 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::CompletedArray(SCPP65Type* psType, CCPPSyntax* pcInitialiser)
{
	SCPP65Type*	psArray;
	int64		lliCount;

	if (!psType->IsArray() || (psType->iCount != 0) || !pcInitialiser)
	{
		return psType;
	}
	lliCount = GetInitialiserCount(mpcUnit->StringInitialiser(pcInitialiser, psType->psBase), psType->psBase);
	if (lliCount <= 0)
	{
		return psType;
	}
	psArray = mpcUnit->GetTypes()->Array(psType->psBase, (int)lliCount);
	return mpcUnit->GetTypes()->Qualified(psArray, psType->bConst, psType->bVolatile);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a declarator is of a declaration that makes no object:
// an extern one, or a typedef.  An array there may have no bound, an
// incomplete type that is completed where the object is defined.
// [dcl.array] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeclarators::DeclaresNoObject(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntax*	pcDeclaration;

	for (pcDeclaration = pcDeclarator->mpcParent; pcDeclaration && ((pcDeclaration->GetKind() == CPPS_Declarator) || (pcDeclaration->GetKind() == CPPS_InitDeclarator)); pcDeclaration = pcDeclaration->mpcParent)
	{
	}
	if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_SimpleDeclaration) || !((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers)
	{
		return false;
	}
	return (((CCPPSyntaxSimpleDeclaration*)pcDeclaration)->mpcSpecifiers->muiFlags & (CPPDS_EXTERN | CPPDS_TYPEDEF)) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Applies a declarator to a type: the pointer and reference operators first, then
// the array and function suffixes from the last, then a nested declarator, as in
// int (*pf)(int) (a pointer to a function).  [dcl.meaning]  A parameter's array
// without a size (char* argv[]) is a pointer.  [dcl.fct]  It stops at pcFunction
// if that is given, for the type a function with that suffix returns.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::GetDeclaratorType(SCPP65Type* psType, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser, CCPPSyntaxFunctionSuffix* pcFunction)
{
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntax*					pcSuffix;
	CCPPSyntaxArraySuffix*		pcArray;
	CCPPClass*				pcClass;
	size						i;
	int64						lliCount;

	for (i = 0; i < pcDeclarator->NumPointers(); i++)
	{
		pcPointer = pcDeclarator->GetPointer(i);
		switch (pcPointer->meKind)
		{
		case CPPP_Pointer:
			if (psType->IsReference())
			{
				//[dcl.ptr] 4
				mpcDiagnostics->Error(pcNode, "A pointer to a reference can't be declared");
				return NULL;
			}
			psType = mpcUnit->QualifiedPointer(mpcUnit->GetTypes()->Pointer(psType), pcPointer);
			break;
		case CPPP_Reference:
		case CPPP_RValueReference:
			if ((i > 0) && ((pcDeclarator->GetPointer(i - 1)->meKind == CPPP_Reference) || (pcDeclarator->GetPointer(i - 1)->meKind == CPPP_RValueReference)))
			{
				//Written out, int& & r: one a typedef or a template's parameter brings is the one reference.  [dcl.ref] 5, 6
				mpcDiagnostics->Error(pcNode, "A reference to a reference can't be declared");
				return NULL;
			}
			psType = mpcUnit->GetTypes()->Reference(psType, pcPointer->meKind == CPPP_RValueReference);
			break;
		case CPPP_MemberPointer:
			pcClass = pcPointer->mpcClass ? mpcUnit->GetNameClass(pcPointer->mpcClass, pcNode) : NULL;
			if (!pcClass)
			{
				mpcDiagnostics->Error(pcNode, "The class of a pointer to a member is not supported by the compiler yet");
				return NULL;
			}
			psType = mpcUnit->GetTypes()->MemberPointer(pcClass, psType);
			break;
		default:
			mpcDiagnostics->Error(pcNode, "This declarator is not supported by the compiler yet");
			return NULL;
		}
	}

	for (i = pcDeclarator->macpSuffixes.NumElements(); i > 0; i--)
	{
		pcSuffix = pcDeclarator->macpSuffixes.GetPtr(i - 1);
		if (pcSuffix == pcFunction)
		{
			return psType;
		}
		if (pcSuffix->GetKind() == CPPS_FunctionSuffix)
		{
			psType = GetFunctionType(psType, (CCPPSyntaxFunctionSuffix*)pcSuffix, pcNode);
			if (!psType)
			{
				return NULL;
			}
			continue;
		}

		pcArray = (CCPPSyntaxArraySuffix*)pcSuffix;
		if (!pcArray->mpcSize && (i == 1) && !pcDeclarator->mpcNested && pcDeclarator->mpcParent && (pcDeclarator->mpcParent->GetKind() == CPPS_Parameter))
		{
			psType = mpcUnit->GetTypes()->Pointer(psType);
			continue;
		}
		if (!pcArray->mpcSize && (i == 1) && !pcDeclarator->mpcNested && !pcInitialiser && ((pcDeclarator->mpcParent && (pcDeclarator->mpcParent->GetKind() == CPPS_TypeId)) || DeclaresNoObject(pcDeclarator)))
		{
			//A<int[]>, extern int a[]; and typedef int A[];: an array of unknown bound, an incomplete type, its count 0.
			//[dcl.array] 3
			lliCount = 0;
		}
		else if (!pcArray->mpcSize)
		{
			lliCount = ((i == 1) && !pcDeclarator->mpcNested) ? GetInitialiserCount(mpcUnit->StringInitialiser(pcInitialiser, psType), psType) : 0;
			if (lliCount <= 0)
			{
				mpcDiagnostics->Error(pcNode, "An array needs a size or an initialiser");
				return NULL;
			}
		}
		else if (!mpcUnit->GetFolder()->Constant(pcArray->mpcSize, &lliCount))
		{
			mpcUnit->GetFolder()->ConstantError(pcArray->mpcSize, "An array size must be a positive constant");
			return NULL;
		}
		else if (lliCount <= 0)
		{
			mpcDiagnostics->Error(pcArray->mpcSize, "An array size must be a positive constant");
			return NULL;
		}
		if (psType->IsClass() && !mpcUnit->CompleteClass(psType->pcClass, pcNode))
		{
			return NULL;
		}
		if (pcArray->mpcSize && (i == 1) && !pcDeclarator->mpcNested && psType->IsInteger() && mpcUnit->IsStringLiteral(mpcUnit->StringInitialiser(pcInitialiser, psType)) &&
			(GetInitialiserCount(mpcUnit->StringInitialiser(pcInitialiser, psType), psType) > lliCount + (mpcUnit->IsCSource() ? 1 : 0)))
		{
			//char a[3] = "abc" has no room for the string's end (which C lets go).  [dcl.init.string] 2
			mpcDiagnostics->Error(pcNode, "The string has more characters, with its end, than the array has elements");
			return NULL;
		}
		if (psType->IsVoid() || psType->IsReference() || psType->IsFunction())
		{
			//[dcl.array] 1
			mpcDiagnostics->Error(pcNode, "An array's elements can't be void, references or functions");
			return NULL;
		}
		psType = mpcUnit->GetTypes()->Array(psType, (int)lliCount);
	}

	if (pcDeclarator->mpcNested)
	{
		return GetDeclaratorType(psType, pcDeclarator->mpcNested, pcNode, pcInitialiser, pcFunction);
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function may return a psReturn: not an array or a function,
// which is an error (and false).  [dcl.fct] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeclarators::IsReturnable(SCPP65Type* psReturn, CCPPSyntax* pcNode)
{
	return !(psReturn->IsArray() || psReturn->IsFunction()) || mpcDiagnostics->Error(pcNode, "A function can't return an array or a function: it returns a pointer or a reference to one");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a parameter of type void is what one may be: a function's whole
// parameter list, with no name, (void), which is no parameters.  An error
// (and false) if not.  [dcl.fct] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeclarators::IsWholeVoid(CCPPSyntaxFunctionSuffix* pcSuffix, CCPPSyntaxParameter* pcParameter)
{
	if ((pcSuffix->macpParameters.NumElements() == 1) && !(pcParameter->mpcDeclarator && pcParameter->mpcDeclarator->GetName()))
	{
		return true;
	}
	return mpcDiagnostics->Error(pcParameter, "A parameter can't be void: (void) alone, with no name, is no parameters");
}


//////////////////////////////////////////////////////////////////////////
//
// A function type: what it returns and its parameters' types.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::GetFunctionType(SCPP65Type* psReturn, CCPPSyntaxFunctionSuffix* pcSuffix, CCPPSyntax* pcNode)
{
	CCPPSyntaxParameter*	pcParameter;
	CArrayCPP65Types		apsParameters;
	SCPP65Type*				psType;
	size					i;

	if (!IsReturnable(psReturn, pcNode))
	{
		return NULL;
	}
	apsParameters.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		if (!psType || (psType->IsVoid() && !IsWholeVoid(pcSuffix, pcParameter)))
		{
			apsParameters.Kill();
			return NULL;
		}
		if (psType->IsVoid())
		{
			break;
		}
		apsParameters.Add(mpcUnit->ParameterType(psType));
	}
	psType = mpcUnit->GetTypes()->Function(psReturn, &apsParameters, pcSuffix->mbVariadic, pcSuffix->IsConst(), pcSuffix->IsVolatile());
	apsParameters.Kill();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of elements an initialiser gives an array of psElement without
// a size: the elements of a list (its values grouped as the elements take
// them: P a[] = { 1, 2, 3, 4 } is two of a P with two members), or a
// string's characters and its zero.  0 if it gives none.
//
//////////////////////////////////////////////////////////////////////////
int CCPPDeclarators::GetInitialiserCount(CCPPSyntax* pcInitialiser, SCPP65Type* psElement)
{
	CArrayUint8		auiBytes;
	int				iCount;

	if (!pcInitialiser)
	{
		return 0;
	}
	if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		return (int)mpcUnit->ListElements(mpcUnit->GetDesugarings()->BracedFor(mpcUnit->GetTypes()->Array(psElement, 0), pcInitialiser))->NumElements();
	}
	if (mpcUnit->IsStringLiteral(pcInitialiser))
	{
		auiBytes.Init();
		iCount = mpcUnit->GetStrings()->DecodeString(pcInitialiser, &auiBytes) ? ((int)auiBytes.NumElements() / mpcUnit->GetStrings()->StringElementBytes(pcInitialiser)) + 1 : 0;
		auiBytes.Kill();
		return iCount;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the specifiers are auto: the type is the initialiser's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeclarators::IsAuto(CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	return pcSpecifiers && !pcSpecifiers->mpcType && (pcSpecifiers->muiTypeKeywords & CPPTK_AUTO);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the specifiers are decltype(auto): the type is the initialiser's
// as decltype has it, a reference where that is one.  [dcl.spec.auto] 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDeclarators::IsDecltypeAuto(CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	CCPPSyntax*	pcOperand;

	if (!pcSpecifiers || !pcSpecifiers->mpcType || (pcSpecifiers->mpcType->GetKind() != CPPS_DecltypeSpecifier))
	{
		return false;
	}
	pcOperand = ((CCPPSyntaxDecltypeSpecifier*)pcSpecifiers->mpcType)->mpcOperand;
	return pcOperand && (pcOperand->GetKind() == CPPS_TypeId) && IsAuto(((CCPPSyntaxTypeId*)pcOperand)->mpcSpecifiers);
}


//////////////////////////////////////////////////////////////////////////
//
// The type auto is in the declaration of a variable (pcNode, with its
// declarator and initialiser): what the initialiser makes it, which must
// be the same for each variable the declaration declares.  [dcl.spec.auto] 7
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::AutoType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcFirst;
	SCPP65Type*						psType;
	SCPP65Type*						psFirst;

	psType = DeduceAuto(pcSpecifiers, pcDeclarator, pcInitialiser, pcNode);
	if (!psType || (pcNode->GetKind() != CPPS_InitDeclarator) || !pcNode->mpcParent || (pcNode->mpcParent->GetKind() != CPPS_SimpleDeclaration))
	{
		return psType;
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcNode->mpcParent;
	pcFirst = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(0);
	if (pcFirst != pcNode)
	{
		mpcDiagnostics->BeginQuiet();
		psFirst = DeduceAuto(pcSpecifiers, pcFirst->mpcDeclarator, pcFirst->mpcInitializer, pcFirst);
		mpcDiagnostics->EndQuiet();
		if (psFirst && !psFirst->Same(psType))
		{
			mpcDiagnostics->Error(pcNode, "auto is not the same type for each variable of the declaration");
			return NULL;
		}
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// What auto is for one declarator and its initialiser, as a template's T
// is for a parameter declared so and that argument: auto x = e is e's
// type without its const (an array's or a function's a pointer), auto* p
// what e points to, auto& r what e is, and auto&& r that or, for what has
// a name, a reference to it.  auto l = {1, 2} is a std::initializer_list
// of the list's elements, and auto x{e} auto x = e.  [dcl.spec.auto] 7,
// [temp.deduct.call]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::DeduceAuto(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	CCPP65Types*	pcTypes;
	SCPP65Type*		psType;
	size			uiPointers;
	ECPPPointer		eLast;

	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer) && (((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements.NumElements() == 1))
	{
		pcInitialiser = ((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements.GetPtr(0);
	}
	if (!pcInitialiser || (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer))
	{
		mpcDiagnostics->Error(pcNode, "auto needs an initialiser to take its type from");
		return NULL;
	}
	if ((pcInitialiser->GetKind() == CPPS_InitializerList) && pcNode && (pcNode->GetKind() == CPPS_InitDeclarator) && (((CCPPSyntaxInitDeclarator*)pcNode)->meInitializer == CPPI_Braces))
	{
		//auto x{e} is auto x = e (as C++17 has it, and MSVC).
		if (mpcUnit->ListElements(pcInitialiser)->NumElements() != 1)
		{
			mpcDiagnostics->Error(pcNode, "auto x{...} needs one value");
			return NULL;
		}
		pcInitialiser = mpcUnit->ListElements(pcInitialiser)->GetPtr(0);
	}
	if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		//auto l = {1, 2} is a std::initializer_list<int>.  [dcl.spec.auto] 7
		if (pcDeclarator && (pcDeclarator->NumPointers() > 0))
		{
			mpcDiagnostics->Error(pcNode, "auto with a pointer or reference from a list in braces is not supported by the compiler yet");
			return NULL;
		}
		psType = mpcUnit->BracedListType(pcInitialiser, pcNode);
		return psType ? mpcUnit->GetTypes()->Qualified(psType, (pcSpecifiers->muiFlags & CPPDS_CONST) != 0, false) : NULL;
	}

	pcTypes = mpcUnit->GetTypes();
	uiPointers = pcDeclarator ? pcDeclarator->NumPointers() : 0;
	eLast = uiPointers ? pcDeclarator->GetPointer(uiPointers - 1)->meKind : CPPP_Pointer;
	if ((eLast == CPPP_Reference) || (eLast == CPPP_RValueReference))
	{
		psType = mpcUnit->IsRvalue(pcInitialiser) ? NULL : mpcUnit->PlaceOf(pcInitialiser);
		if (psType && (eLast == CPPP_RValueReference) && (uiPointers == 1) && !(pcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
		{
			//auto&& of what has a name is a reference to it.
			return pcTypes->Reference(psType);
		}
		psType = psType ? psType : mpcUnit->ValueTypeOf(pcInitialiser);
		uiPointers--;
	}
	else
	{
		psType = mpcUnit->ValueTypeOf(pcInitialiser);
		if (psType && (psType->IsArray() || psType->IsFunction()))
		{
			psType = pcTypes->Pointer(psType->ElementType());
		}
		else if (psType && psType->IsPointer() && mpcUnit->IsStringLiteral(mpcUnit->SkipParentheses(pcInitialiser)))
		{
			//A string literal's characters are const.
			psType = pcTypes->Pointer(pcTypes->Qualified(psType->psBase, true, false));
		}
		psType = psType ? psType->Unqualified() : NULL;
	}
	if (!psType)
	{
		mpcUnit->ReportTypeOf(pcInitialiser);
		return NULL;
	}
	for (; uiPointers > 0; uiPointers--)
	{
		if ((pcDeclarator->GetPointer(uiPointers - 1)->meKind != CPPP_Pointer) || !psType->IsPointer())
		{
			mpcDiagnostics->Error(pcNode, "The initialiser is not what the declarator makes of auto");
			return NULL;
		}
		psType = psType->psBase;
	}

	//const auto takes the type without the const it has.
	return pcTypes->Qualified(psType, psType->bConst && !(pcSpecifiers->muiFlags & CPPDS_CONST), psType->bVolatile && !(pcSpecifiers->muiFlags & CPPDS_VOLATILE));
}


//////////////////////////////////////////////////////////////////////////
//
// __underlying_type(Enum): the integer type the enum's values are, its
// fixed one (enum E : short) or the one the compiler gave it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::UnderlyingType(CCPPSyntax* pcOperand, CCPPSyntax* pcNode)
{
	SCPP65Type*	psType;

	psType = (pcOperand && (pcOperand->GetKind() == CPPS_TypeId)) ? GetTypeIdType((CCPPSyntaxTypeId*)pcOperand) : NULL;
	if (!psType || !psType->pcEnum)
	{
		if (psType)
		{
			mpcDiagnostics->Error(pcNode, "__underlying_type is of an enum");
		}
		return NULL;
	}
	return psType->eInteger ? mpcUnit->GetTypes()->IntegerOfKind(psType->eInteger) : mpcUnit->GetTypes()->Integer(psType->iBytes, psType->bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
// decltype(operand).  A name, or a member of an object, with no
// parentheses round it is the type the thing it names was declared with
// (EntityType).  Any other expression is its type: a reference to it if
// the expression is an lvalue (so decltype((a)) is int& where decltype(a)
// is int), an rvalue reference if it is an xvalue, and for a prvalue the
// type alone, which has no const unless it is a class's.  Nothing is
// worked out.  [dcl.type.simple] 4, [expr] 6
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::DecltypeType(CCPPSyntax* pcOperand, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;

	//The operand isn't evaluated.
	mpcUnit->GetContext()->EnterUnevaluated();
	psType = OperandType(pcOperand);
	mpcUnit->GetContext()->LeaveUnevaluated();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// What DecltypeType gives.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::OperandType(CCPPSyntax* pcOperand)
{
	SCPP65Type*		psType;
	SCPP65Type*		psPlace;

	if (pcOperand->GetKind() == CPPS_TypeId)
	{
		//__typeof__(type).
		return GetTypeIdType((CCPPSyntaxTypeId*)pcOperand);
	}
	psType = EntityType(pcOperand);
	if (psType)
	{
		return psType;
	}

	psType = mpcUnit->DeclaredTypeOf(pcOperand);
	if (!psType)
	{
		mpcUnit->ReportTypeOf(pcOperand);
		return NULL;
	}
	if (psType->IsReference())
	{
		//A call or a cast that gives a reference.
		return psType;
	}
	psPlace = mpcUnit->PlaceOf(pcOperand);
	if (psPlace && !mpcUnit->IsRvalue(pcOperand))
	{
		return mpcUnit->GetTypes()->Reference(psPlace);
	}
	if (psPlace && (mpcUnit->SkipParentheses(pcOperand)->GetKind() == CPPS_MemberExpression))
	{
		//Make().m: a field of an object that is going, an xvalue.
		return mpcUnit->GetTypes()->Reference(psPlace, true);
	}
	psType = mpcUnit->ValueTypeOf(pcOperand);
	return (psType && !psType->IsClass()) ? psType->Unqualified() : psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The type the thing a name, or the member of object.member, names was
// declared with: a variable's, a field's (a reference field's is the
// reference, and a const object's field isn't const for that), a function's,
// and for what a lambda captured the variable's own.  NULL if the operand
// isn't a name or a member alone, or names none of them.
// [dcl.type.simple] 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::EntityType(CCPPSyntax* pcOperand)
{
	CCPPBoundName	cBound;
	SCPP65Type*		psType;

	cBound.meWhat = CPPBN_Nothing;
	if (pcOperand->GetKind() == CPPS_IdExpression)
	{
		psType = mpcUnit->QualifiedFieldType(pcOperand);
		if (psType)
		{
			return psType;
		}
		if (mpcUnit->NamesFunction(pcOperand))
		{
			psType = mpcUnit->TypeOf(pcOperand);
			return (psType && psType->IsPointer()) ? psType->psBase : NULL;
		}
		mpcUnit->GetBinder()->BindName((CCPPSyntaxIdExpression*)pcOperand, &cBound);
	}
	else if ((pcOperand->GetKind() != CPPS_MemberExpression) || !mpcUnit->BindMemberOf((CCPPSyntaxMemberExpression*)pcOperand, &cBound))
	{
		return NULL;
	}

	switch (cBound.meWhat)
	{
	case CPPBN_Variable:
		return cBound.mpcVariable->mpsType;
	case CPPBN_EnclosingConstant:
		//A constant of the function a lambda is in, which the lambda doesn't capture.
		return mpcUnit->GetClosures()->EnclosingConstantObjectType(cBound.mpcSymbol);
	case CPPBN_Field:
		return cBound.mpsField->psType;
	case CPPBN_Capture:
		//The closure's field is the variable's copy, or a reference to it.
		return cBound.mpsCapture->bReference ? cBound.mpsField->psType->WithoutReference() : cBound.mpsField->psType;
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Name<arguments> where Name is template<...> using Name = type: the type,
// with the template's parameters the arguments.  An alias template has no
// instances of its own: it is the type each time.  [temp.alias] 2
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::AliasTemplateType(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPSymbol* pcAliasSymbol)
{
	CCPPSyntaxAliasDeclaration*	pcAlias;
	CCPPTemplateArguments*		pcBindings;
	SCPP65Type*					psType;
	CChars						szArguments;

	pcAlias = (CCPPSyntaxAliasDeclaration*)(pcAliasSymbol ? pcAliasSymbol : pcName->mpcSymbol)->mpcDeclaration;
	if (!pcAlias || (pcAlias->GetKind() != CPPS_AliasDeclaration) || !pcAlias->mpcParent || (pcAlias->mpcParent->GetKind() != CPPS_TemplateDeclaration))
	{
		mpcDiagnostics->Error(pcNode, "This type is not supported by the compiler yet");
		return NULL;
	}
	szArguments.Init();
	pcBindings = mpcUnit->GetInstantiator()->BindArguments((CCPPSyntaxTemplateDeclaration*)pcAlias->mpcParent, &pcName->GetLast()->macpTemplateArguments, NULL, &szArguments, pcNode);
	szArguments.Kill();
	if (!pcBindings)
	{
		return NULL;
	}
	mpcUnit->GetContext()->EnterBindings(pcBindings);
	psType = GetTypeIdType(pcAlias->mpcType);
	mpcUnit->GetContext()->LeaveBindings();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The type the specifiers name.  long is 4 bytes, as on Windows.  char is signed.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::GetBaseType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntax* pcNode)
{
	CCPPSymbol*					pcMember;
	CCPPClass*				pcMemberClass;
	SCPP65Type*					psMemberType;
	uint32								uiKeywords;
	bool								bSigned;
	CCPPSyntaxTypeSpecifier*			pcType;
	CCPPSyntaxName*						pcName;
	CCPPSyntaxClassSpecifier*			pcClassSpecifier;
	CCPPSyntaxElaboratedTypeSpecifier*	pcElaborated;
	CCPPSymbol*							pcSymbol;
	CCPPClass*						pcClass;
	SCPPBinding*						psBinding;

	if (!pcSpecifiers)
	{
		return &mpcUnit->GetTypes()->msInt;
	}

	pcType = pcSpecifiers->mpcType;
	if (pcType)
	{
		if (pcType->GetKind() == CPPS_DecltypeSpecifier)
		{
			if (((CCPPSyntaxDecltypeSpecifier*)pcType)->mbUnderlyingType)
			{
				return UnderlyingType(((CCPPSyntaxDecltypeSpecifier*)pcType)->mpcOperand, pcNode);
			}
			return DecltypeType(((CCPPSyntaxDecltypeSpecifier*)pcType)->mpcOperand, pcNode);
		}
		if (pcType->GetKind() == CPPS_EnumSpecifier)
		{
			return mpcUnit->EnumType(((CCPPSyntaxEnumSpecifier*)pcType)->mpcSymbol, (CCPPSyntaxEnumSpecifier*)pcType);
		}
		if (pcType->GetKind() == CPPS_ClassSpecifier)
		{
			//The class is laid out when it is first needed whole.
			pcClassSpecifier = (CCPPSyntaxClassSpecifier*)pcType;
			pcClass = pcClassSpecifier->mpcSymbol ? mpcUnit->GetClass(pcClassSpecifier->mpcSymbol, pcNode) : mpcUnit->GetUnnamedClass(pcClassSpecifier);
			if (!pcClass->mpcSpecifier)
			{
				pcClass->mpcSpecifier = pcClassSpecifier;
			}
			return pcClass->mpsType;
		}
		if (pcType->GetKind() == CPPS_ElaboratedTypeSpecifier)
		{
			pcElaborated = (CCPPSyntaxElaboratedTypeSpecifier*)pcType;
			if (pcElaborated->meKey == CT_enum)
			{
				pcSymbol = pcElaborated->mpcName->mpcSymbol;
				return (pcSymbol && (pcSymbol->meKind == CPPSK_Enum)) ? mpcUnit->EnumType(pcSymbol, NULL) : &mpcUnit->GetTypes()->msInt;
			}
			//struct timsort names the class even when a function timsort was declared first.
			pcSymbol = pcElaborated->mpcName->mpcSymbol;
			while (pcSymbol && (pcSymbol->meKind != CPPSK_Class) && (pcSymbol->meKind != CPPSK_ClassTemplate))
			{
				pcSymbol = pcSymbol->mpcNext;
			}
			if (!pcSymbol && pcElaborated->mpcName->mpcSymbol && (pcElaborated->mpcName->mpcSymbol->meKind == CPPSK_Enum))
			{
				//struct E of an enum E.  [dcl.type.elab] 3
				mpcDiagnostics->Error(pcNode, "The name is an enum's: struct, class and union name a class");
				return NULL;
			}
			pcClass = mpcUnit->GetComponentClass(pcElaborated->mpcName->GetLast(), pcSymbol ? pcSymbol : pcElaborated->mpcName->mpcSymbol, pcNode);
			if (pcClass)
			{
				return pcClass->mpsType;
			}
		}
		else if (pcType->GetKind() == CPPS_NamedTypeSpecifier)
		{
			pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
			pcMember = mpcUnit->QualifiedMember(pcName, CPPSK_Typedef, pcNode, &pcMemberClass);
			mpcUnit->GetBinder()->CheckTypeName(pcMember ? pcMember : pcName->mpcSymbol, pcName);
			mpcUnit->WarnDeprecatedSymbol(pcMember ? pcMember : pcName->mpcSymbol, pcName);
			if (!pcMember && !mpcUnit->CheckBaseMemberName(pcName, pcNode))
			{
				return NULL;
			}
			if (pcMember)
			{
				mpcUnit->GetContext()->EnterBindings(pcMemberClass->mpcBindings);
				psMemberType = GetSymbolType(pcMember, pcNode);
				mpcUnit->GetContext()->LeaveBindings();
				return psMemberType;
			}
			if (pcName->mpcSymbol)
			{
				switch (pcName->mpcSymbol->meKind)
				{
				case CPPSK_Enum:
					return mpcUnit->EnumType(pcName->mpcSymbol, NULL);
				case CPPSK_Typedef:
					return GetSymbolType(pcName->mpcSymbol, pcNode);
				case CPPSK_AliasTemplate:
					return AliasTemplateType(pcName, pcNode);
				case CPPSK_TypeParameter:
					psBinding = mpcUnit->GetInstantiator()->FindBinding(pcName->mpcSymbol);
					if (!psBinding || !psBinding->psType)
					{
						mpcDiagnostics->Error(pcNode, "A template parameter used outside an instantiation");
						return NULL;
					}
					return psBinding->psType;
				case CPPSK_TemplateTemplateParameter:
					psBinding = mpcUnit->GetInstantiator()->FindBinding(pcName->mpcSymbol);
					if (psBinding && psBinding->pcAlias)
					{
						return AliasTemplateType(pcName, pcNode, psBinding->pcAlias);
					}
					pcClass = mpcUnit->GetNameClass(pcName, pcNode);
					return pcClass ? pcClass->mpsType : NULL;
				case CPPSK_Class:
				case CPPSK_ClassTemplate:
					pcClass = mpcUnit->GetNameClass(pcName, pcNode);
					return pcClass ? pcClass->mpsType : NULL;
				case CPPSK_BuiltinType:
					if (mpcUnit->GetSemanticPass())
					{
						//Only checking: the host's __builtin_va_list, a char* as MSVC's va_list is.
						return mpcUnit->GetTypes()->Pointer(&mpcUnit->GetTypes()->msChar);
					}
					break;
				default:
					break;
				}
			}
			else
			{
				//typename C::template In<long>, C bound to a class.
				pcClass = mpcUnit->QualifiedClass(pcName, pcNode);
				if (pcClass)
				{
					return pcClass->mpsType;
				}
			}
			if (pcName->GetQualifier())
			{
				//typename T::Missing with T int: no class with a member type of the name.  [temp.res] 3, [temp.inst] 1
				mpcDiagnostics->Error(pcNode, "What qualifies the name is no class with a member type of that name");
				return NULL;
			}
			if (!pcName->mpcSymbol)
			{
				//A class's member type is named through the class outside it (S::I).  [basic.lookup.unqual] 1
				mpcDiagnostics->Error(pcNode, "The type's name isn't declared where it is used");
				return NULL;
			}
		}
		mpcDiagnostics->Error(pcNode, "This type is not supported by the compiler yet");
		return NULL;
	}

	//long double is double, as MSVC.
	uiKeywords = pcSpecifiers->muiTypeKeywords;
	bSigned = (uiKeywords & CPPTK_UNSIGNED) == 0;
	if (uiKeywords & CPPTK_FLOAT)
	{
		return mpcUnit->GetTypes()->Float(4);
	}
	if (uiKeywords & CPPTK_DOUBLE)
	{
		return (pcSpecifiers->miLongs >= 1) ? mpcUnit->GetTypes()->LongDouble() : mpcUnit->GetTypes()->Float(8);
	}
	if ((pcSpecifiers->miLongs >= 2) || (uiKeywords & CPPTK_INT64))
	{
		return mpcUnit->GetTypes()->Integer(8, bSigned);
	}
	if (uiKeywords & CPPTK_AUTO)
	{
		mpcDiagnostics->Error(pcSpecifiers, "auto can only be the type of a variable with an initialiser");
		return NULL;
	}
	if (uiKeywords & CPPTK_VOID)
	{
		return &mpcUnit->GetTypes()->msVoid;
	}
	if (uiKeywords & CPPTK_BOOL)
	{
		return &mpcUnit->GetTypes()->msBool;
	}
	//Each a type of its own, though as wide as another.  [basic.fundamental]
	if (uiKeywords & (CPPTK_CHAR | CPPTK_INT8))
	{
		//char, signed char and unsigned char are three types (char is signed here).  MSVC's __int8 is char.
		return !bSigned ? &mpcUnit->GetTypes()->msUnsignedChar : ((uiKeywords & CPPTK_SIGNED) ? &mpcUnit->GetTypes()->msSignedChar : &mpcUnit->GetTypes()->msChar);
	}
	if (uiKeywords & CPPTK_WCHAR_T)
	{
		return &mpcUnit->GetTypes()->msWChar;
	}
	if (uiKeywords & CPPTK_CHAR16_T)
	{
		return &mpcUnit->GetTypes()->msChar16;
	}
	if (uiKeywords & CPPTK_CHAR32_T)
	{
		return &mpcUnit->GetTypes()->msChar32;
	}
	if (uiKeywords & (CPPTK_SHORT | CPPTK_INT16))
	{
		return bSigned ? &mpcUnit->GetTypes()->msShort : &mpcUnit->GetTypes()->msUnsignedShort;
	}
	if (uiKeywords & CPPTK_INT32)
	{
		//MSVC's __int32: int where int is 4 bytes, else long.
		return mpcUnit->GetTypes()->Integer(4, bSigned);
	}
	if (pcSpecifiers->miLongs == 1)
	{
		return mpcUnit->GetTypes()->Long(bSigned);
	}
	if (uiKeywords & (CPPTK_INT | CPPTK_SIGNED | CPPTK_UNSIGNED))
	{
		return bSigned ? &mpcUnit->GetTypes()->msInt : &mpcUnit->GetTypes()->msUnsignedInt;
	}
	mpcDiagnostics->Error(pcNode, "This type is not supported by the compiler yet");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::GetTypeIdType(CCPPSyntaxTypeId* pcTypeId)
{
	return GetType(pcTypeId->mpcSpecifiers, pcTypeId->mpcDeclarator, pcTypeId);
}


//////////////////////////////////////////////////////////////////////////
//
// The type a typedef, variable or parameter symbol was declared with.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::GetSymbolType(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	CCPPSyntax*					pcDeclaration;
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxParameter*		pcParameter;

	pcDeclaration = pcSymbol->mpcDeclaration;
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator) && ((CCPPSyntaxInitDeclarator*)pcDeclaration)->GetDeclaration())
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration;
		return GetType(pcInit->GetDeclaration()->mpcSpecifiers, pcInit->mpcDeclarator, pcNode, pcInit->mpcInitializer);
	}
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_Parameter))
	{
		pcParameter = (CCPPSyntaxParameter*)pcDeclaration;
		return GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcNode);
	}
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_AliasDeclaration))
	{
		return GetTypeIdType(((CCPPSyntaxAliasDeclaration*)pcDeclaration)->mpcType);
	}
	mpcDiagnostics->Error(pcNode, "This declaration is not supported by the compiler yet");
	return NULL;
}

