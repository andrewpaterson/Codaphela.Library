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
			mpcUnit->Error(pcNode, "decltype(auto) needs an initialiser to take its type from");
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
	return GetDeclaratorType(psType, pcDeclarator, pcNode, pcInitialiser);
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
			psType = mpcUnit->QualifiedPointer(mpcUnit->GetTypes()->Pointer(psType), pcPointer);
			break;
		case CPPP_Reference:
		case CPPP_RValueReference:
			psType = mpcUnit->GetTypes()->Reference(psType, pcPointer->meKind == CPPP_RValueReference);
			break;
		case CPPP_MemberPointer:
			pcClass = pcPointer->mpcClass ? mpcUnit->GetNameClass(pcPointer->mpcClass, pcNode) : NULL;
			if (!pcClass)
			{
				mpcUnit->Error(pcNode, "The class of a pointer to a member is not supported by the compiler yet");
				return NULL;
			}
			psType = mpcUnit->GetTypes()->MemberPointer(pcClass, psType);
			break;
		default:
			mpcUnit->Error(pcNode, "This declarator is not supported by the compiler yet");
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
		if (!pcArray->mpcSize && (i == 1) && !pcDeclarator->mpcNested && pcDeclarator->mpcParent && (pcDeclarator->mpcParent->GetKind() == CPPS_TypeId) && !pcInitialiser)
		{
			//A<int[]>: an array of unknown bound, an incomplete type, its count 0.  [dcl.array] 5
			lliCount = 0;
		}
		else if (!pcArray->mpcSize)
		{
			lliCount = ((i == 1) && !pcDeclarator->mpcNested) ? GetInitialiserCount(mpcUnit->StringInitialiser(pcInitialiser, psType)) : 0;
			if (lliCount <= 0)
			{
				mpcUnit->Error(pcNode, "An array needs a size or an initialiser");
				return NULL;
			}
		}
		else if (!mpcUnit->Constant(pcArray->mpcSize, &lliCount))
		{
			mpcUnit->ConstantError(pcArray->mpcSize, "An array size must be a positive constant");
			return NULL;
		}
		else if (lliCount <= 0)
		{
			mpcUnit->Error(pcArray->mpcSize, "An array size must be a positive constant");
			return NULL;
		}
		if (psType->IsClass() && !mpcUnit->CompleteClass(psType->pcClass, pcNode))
		{
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
// A function type: what it returns and its parameters' types.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::GetFunctionType(SCPP65Type* psReturn, CCPPSyntaxFunctionSuffix* pcSuffix, CCPPSyntax* pcNode)
{
	CCPPSyntaxParameter*	pcParameter;
	CArrayCPP65Types		apsParameters;
	SCPP65Type*				psType;
	size					i;

	apsParameters.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		if (!psType)
		{
			apsParameters.Kill();
			return NULL;
		}
		if (psType->IsVoid() && (pcSuffix->macpParameters.NumElements() == 1))
		{
			break;
		}
		apsParameters.Add(mpcUnit->ParameterType(psType));
	}
	psType = mpcUnit->GetTypes()->Function(psReturn, &apsParameters, pcSuffix->mbVariadic);
	apsParameters.Kill();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of elements an initialiser gives an array without a size: the
// elements of a list, or a string's characters and its zero.  0 if it gives none.
//
//////////////////////////////////////////////////////////////////////////
int CCPPDeclarators::GetInitialiserCount(CCPPSyntax* pcInitialiser)
{
	CArrayUint8		auiBytes;
	int				iCount;

	if (!pcInitialiser)
	{
		return 0;
	}
	if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		return (int)mpcUnit->ListElements(pcInitialiser)->NumElements();
	}
	if (pcInitialiser->GetKind() == CPPS_StringExpression)
	{
		auiBytes.Init();
		iCount = mpcUnit->DecodeString(pcInitialiser, &auiBytes) ? ((int)auiBytes.NumElements() / mpcUnit->StringElementBytes(pcInitialiser)) + 1 : 0;
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
		mpcUnit->BeginQuiet();
		psFirst = DeduceAuto(pcSpecifiers, pcFirst->mpcDeclarator, pcFirst->mpcInitializer, pcFirst);
		mpcUnit->EndQuiet();
		if (psFirst && !psFirst->Same(psType))
		{
			mpcUnit->Error(pcNode, "auto is not the same type for each variable of the declaration");
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
// a name, a reference to it.  [dcl.spec.auto] 7, [temp.deduct.call]
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
		mpcUnit->Error(pcNode, "auto needs an initialiser to take its type from");
		return NULL;
	}
	if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		mpcUnit->Error(pcNode, "auto from a list in braces is not supported by the compiler yet");
		return NULL;
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
			psType = pcTypes->Pointer(psType->IsArray() ? psType->psBase : psType);
		}
		else if (psType && psType->IsPointer() && (mpcUnit->SkipParentheses(pcInitialiser)->GetKind() == CPPS_StringExpression))
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
			mpcUnit->Error(pcNode, "The initialiser is not what the declarator makes of auto");
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
			mpcUnit->Error(pcNode, "__underlying_type is of an enum");
		}
		return NULL;
	}
	return mpcUnit->GetTypes()->Integer(psType->iBytes, psType->bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
// decltype(operand).  A name, or a member of an object, with no
// parentheses round it is the type it was declared with; any other
// expression is its type, a reference to it if the expression is an
// lvalue (so decltype((a)) is int& where decltype(a) is int).  Nothing
// is worked out.  [dcl.type.simple] 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::DecltypeType(CCPPSyntax* pcOperand, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;

	//The operand isn't evaluated.
	mpcUnit->BeginUnevaluated();
	psType = OperandType(pcOperand);
	mpcUnit->EndUnevaluated();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// What DecltypeType gives.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::OperandType(CCPPSyntax* pcOperand)
{
	CCPPBoundName	cBound;
	SCPP65Type*		psType;

	if (pcOperand->GetKind() == CPPS_TypeId)
	{
		//__typeof__(type).
		return GetTypeIdType((CCPPSyntaxTypeId*)pcOperand);
	}
	if (pcOperand->GetKind() == CPPS_IdExpression)
	{
		mpcUnit->GetBinder()->BindName((CCPPSyntaxIdExpression*)pcOperand, &cBound);
		if (cBound.meWhat == CPPBN_Variable)
		{
			return cBound.mpcVariable->mpsType;
		}
		if (cBound.meWhat == CPPBN_Field)
		{
			return cBound.mpsField->psType;
		}
	}
	else if (pcOperand->GetKind() == CPPS_MemberExpression)
	{
		psType = mpcUnit->PlaceOf(pcOperand);
		if (psType)
		{
			//The field's type, not const because its object is.
			return mpcUnit->IsConstObject(((CCPPSyntaxMemberExpression*)pcOperand)->mpcObject) ? psType->Unqualified() : psType;
		}
	}

	psType = mpcUnit->DeclaredTypeOf(pcOperand);
	if (!psType)
	{
		mpcUnit->ReportTypeOf(pcOperand);
		return NULL;
	}
	if (psType->IsReference() || mpcUnit->IsRvalue(pcOperand))
	{
		return psType;
	}
	psType = mpcUnit->PlaceOf(pcOperand);
	return psType ? mpcUnit->GetTypes()->Reference(psType) : mpcUnit->ValueTypeOf(pcOperand);
}


//////////////////////////////////////////////////////////////////////////
//
// Name<arguments> where Name is template<...> using Name = type: the type,
// with the template's parameters the arguments.  An alias template has no
// instances of its own: it is the type each time.  [temp.alias] 2
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPDeclarators::AliasTemplateType(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSyntaxAliasDeclaration*	pcAlias;
	CCPPTemplateArguments*		pcBindings;
	SCPP65Type*					psType;
	CChars						szArguments;

	pcAlias = (CCPPSyntaxAliasDeclaration*)pcName->mpcSymbol->mpcDeclaration;
	if (!pcAlias || (pcAlias->GetKind() != CPPS_AliasDeclaration) || !pcAlias->mpcParent || (pcAlias->mpcParent->GetKind() != CPPS_TemplateDeclaration))
	{
		mpcUnit->Error(pcNode, "This type is not supported by the compiler yet");
		return NULL;
	}
	szArguments.Init();
	pcBindings = mpcUnit->BindArguments((CCPPSyntaxTemplateDeclaration*)pcAlias->mpcParent, &pcName->GetLast()->macpTemplateArguments, NULL, &szArguments, pcNode);
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
					psBinding = mpcUnit->FindBinding(pcName->mpcSymbol);
					if (!psBinding || !psBinding->psType)
					{
						mpcUnit->Error(pcNode, "A template parameter used outside an instantiation");
						return NULL;
					}
					return psBinding->psType;
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
		}
		mpcUnit->Error(pcNode, "This type is not supported by the compiler yet");
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
		mpcUnit->Error(pcSpecifiers, "auto can only be the type of a variable with an initialiser");
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
	if (uiKeywords & (CPPTK_CHAR | CPPTK_INT8))
	{
		return mpcUnit->GetTypes()->Integer(1, bSigned);
	}
	if (uiKeywords & (CPPTK_WCHAR_T | CPPTK_CHAR16_T))
	{
		return mpcUnit->GetTypes()->Integer(2, false);
	}
	if (uiKeywords & CPPTK_CHAR32_T)
	{
		return mpcUnit->GetTypes()->Integer(4, false);
	}
	if (uiKeywords & (CPPTK_SHORT | CPPTK_INT16))
	{
		return mpcUnit->GetTypes()->Integer(2, bSigned);
	}
	if ((uiKeywords & CPPTK_INT32) || (pcSpecifiers->miLongs == 1))
	{
		return mpcUnit->GetTypes()->Long(bSigned);
	}
	if (uiKeywords & (CPPTK_INT | CPPTK_SIGNED | CPPTK_UNSIGNED))
	{
		return bSigned ? &mpcUnit->GetTypes()->msInt : &mpcUnit->GetTypes()->msUnsignedInt;
	}
	mpcUnit->Error(pcNode, "This type is not supported by the compiler yet");
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
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator) && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_SimpleDeclaration))
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration;
		return GetType(((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers, pcInit->mpcDeclarator, pcNode, pcInit->mpcInitializer);
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
	mpcUnit->Error(pcNode, "This declaration is not supported by the compiler yet");
	return NULL;
}

