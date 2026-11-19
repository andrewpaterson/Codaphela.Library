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
// Whether an identifier (lliName) is szName.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsNamed(int64 lliName, const char* szName)
{
	char*	szIdentifier;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szIdentifier = mpcTokeniser->GetIdentifier(lliName, &iLength);
	return ((size)iLength == strlen(szName)) && (memcmp(szIdentifier, szName, iLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name declared in pcScope is std::szName: the library's, which
// the compiler knows (std::initializer_list, std::type_info).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsStdName(int64 lliName, CCPPScope* pcScope, const char* szName)
{
	return pcScope && pcScope->mpcOwner && IsNamed(lliName, szName) && IsNamed(pcScope->mpcOwner->mlliName, "std") && (!pcScope->mpcParent || !pcScope->mpcParent->mpcOwner);
}


//////////////////////////////////////////////////////////////////////////
//
// The class std::type_info, if the program has declared it (<typeinfo>), or
// NULL.  [type.info]
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::TypeInfoClass(void)
{
	CCPPClass*	pcClass;
	size		i;

	for (i = 0; !mpcTypeInfo && (i < mapcClasses.NumElements()); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (!pcClass->mpcTemplate && pcClass->mpcSymbol && IsStdName(pcClass->mlliName, pcClass->mpcSymbol->mpcScope, "type_info"))
		{
			mpcTypeInfo = pcClass;
		}
	}
	return mpcTypeInfo;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function is a member of std::type_info that the program declares
// and nothing defines (<typeinfo>'s), which the IR front end works out where
// it is called (CCPP65IRFrontEnd::TypeInfoCall).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsTypeInfoMember(CCPPFunction* pcFunction)
{
	return pcFunction->mpcClass && (pcFunction->mpcClass == TypeInfoClass()) && !pcFunction->mpcDefinition && (pcFunction->meKind == CPPFK_Function);
}


//////////////////////////////////////////////////////////////////////////
//
// The type typeid gives the std::type_info of: a type's, or an expression's
// (its type without a reference), without const and volatile.  *pbDynamic
// when it is the object's own (an lvalue of a polymorphic class), found
// from its vtable.  [expr.typeid]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::TypeidType(CCPPSyntaxTypeidExpression* pcTypeid, bool* pbDynamic)
{
	SCPP65Type*	psType;

	*pbDynamic = false;
	if (pcTypeid->mpcType)
	{
		psType = mcDeclarators.GetTypeIdType(pcTypeid->mpcType);
	}
	else
	{
		psType = TypeOf(pcTypeid->mpcOperand);
		psType = psType ? psType->WithoutReference() : NULL;
		*pbDynamic = psType && psType->IsClass() && CompleteClass(psType->pcClass, pcTypeid) && psType->pcClass->IsPolymorphic() && IsObjectLvalue(pcTypeid->mpcOperand);
	}
	return psType ? psType->WithoutReference()->Unqualified() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The label of a type's type info, the address typeid gives, written if it
// isn't yet: a class's is _Class__3 (EnsureTypeInfo), any other type's _3
// and the type mangled, in a COMDAT section, so its address is the type's in
// the whole program (no name starts with a digit).  Its name and kind, then
// for a pointer its pointee's type info, for a pointer to a member the
// member's type's and the class's (CPP65_TYPE_INFO_NAME_OFFSET); the kind
// word of both says whether what is pointed to is const or volatile.  The type
// infos it points to are written first, each a section of its own.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendTypeidLabel(CChars* psz, SCPP65Type* psType)
{
	CChars				szLabel;
	CChars				szName;
	CChars				szFirst;
	CChars				szSecond;
	ECPP65TypeInfoKind	eKind;
	uint32				uiPointee;

	if (psType->IsClass())
	{
		EnsureTypeInfo(psType->pcClass);
		mcMangler.AppendTypeInfoLabel(psz, psType->pcClass);
		return;
	}
	szLabel.Init("_3");
	mcMangler.AppendType(&szLabel, psType);
	if (Emit(&szLabel))
	{
		szFirst.Init();
		szSecond.Init();
		uiPointee = 0;
		switch (psType->eKind)
		{
		case CPP65TK_Pointer:
			eKind = CPP65TIK_Pointer;
			uiPointee = (psType->psBase->bConst ? CPP65_TYPE_INFO_POINTEE_CONST : 0) | (psType->psBase->bVolatile ? CPP65_TYPE_INFO_POINTEE_VOLATILE : 0);
			AppendTypeidLabel(&szFirst, psType->psBase->Unqualified());
			break;
		case CPP65TK_MemberPointer:
			eKind = CPP65TIK_MemberPointer;
			uiPointee = (psType->psBase->bConst ? CPP65_TYPE_INFO_POINTEE_CONST : 0) | (psType->psBase->bVolatile ? CPP65_TYPE_INFO_POINTEE_VOLATILE : 0);
			AppendTypeidLabel(&szFirst, psType->psBase->Unqualified());
			AppendTypeidLabel(&szSecond, psType->pcClass->mpsType);
			break;
		case CPP65TK_Array:		eKind = CPP65TIK_Array;		break;
		case CPP65TK_Function:	eKind = CPP65TIK_Function;	break;
		case CPP65TK_NullPointer:	eKind = CPP65TIK_NullPointer;	break;
		default:				eKind = psType->pcEnum ? CPP65TIK_Enum : CPP65TIK_Fundamental;	break;
		}
		szName.Init();
		AppendTypeInfoName(&szLabel, psType, true, &szName);
		CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(mcData.GetText(), &szLabel);
		mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
		mcData.GetText()->AppendNewLine();
		mcData.GetText()->Append(&szLabel);
		mcData.GetText()->Append(':');
		mcData.GetText()->AppendNewLine();
		AppendTypeInfoHeader(&szName, (uint32)eKind | uiPointee);
		if (szFirst.Length() > 0)
		{
			mcData.GetText()->Append("\t\t.long ")->Append(&szFirst);
			mcData.GetText()->AppendNewLine();
		}
		if (szSecond.Length() > 0)
		{
			mcData.GetText()->Append("\t\t.long ")->Append(&szSecond);
			mcData.GetText()->AppendNewLine();
		}
		szName.Kill();
		szSecond.Kill();
		szFirst.Kill();
	}
	psz->Append(&szLabel);
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A type as a program writes it, for type_info::name ([type.info] 9 leaves
// the string to the implementation): "int", "unsigned char", "N::A",
// "N::Box<long>", "const char*", "char* const*", "int (*)[3]",
// "void (*)(int)", "int A::*".  An integer is named by its size as the
// compiler has its types, so long is int where they are as wide and
// wchar_t is unsigned short (CppParserTodo.txt question 13).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendTypeName(CChars* psz, SCPP65Type* psType)
{
	CChars	szInner;

	szInner.Init();
	AppendTypeNameAround(psz, psType, &szInner);
	szInner.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// psType around pszInner, what is declared of it so far (a declarator
// without its name): int around * is int*, int[3] around (*) is
// int (*)[3], and a pointer to an array or a function is put in brackets.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendTypeNameAround(CChars* psz, SCPP65Type* psType, CChars* pszInner)
{
	CChars		szInner;
	size		i;

	szInner.Init();
	switch (psType->eKind)
	{
	case CPP65TK_Pointer:
	case CPP65TK_Reference:
	case CPP65TK_MemberPointer:
		if (psType->eKind == CPP65TK_MemberPointer)
		{
			AppendClassName(&szInner, psType->pcClass);
			szInner.Append("::*");
		}
		else if (psType->eKind == CPP65TK_Pointer)
		{
			szInner.Append('*');
		}
		else
		{
			szInner.Append(psType->bRvalue ? "&&" : "&");
		}
		if (psType->eKind != CPP65TK_Reference)
		{
			szInner.Append(psType->bConst ? " const" : "");
			szInner.Append(psType->bVolatile ? " volatile" : "");
		}
		szInner.Append(pszInner);
		if (psType->psBase->IsArray() || psType->psBase->IsFunction())
		{
			szInner.Insert(0, '(');
			szInner.Append(')');
		}
		AppendTypeNameAround(psz, psType->psBase, &szInner);
		szInner.Kill();
		return;

	case CPP65TK_Array:
		szInner.Append(pszInner);
		szInner.Append('[')->Append(psType->iCount)->Append(']');
		AppendTypeNameAround(psz, psType->psBase, &szInner);
		szInner.Kill();
		return;

	case CPP65TK_Function:
		szInner.Append(pszInner);
		szInner.Append('(');
		for (i = 0; i < psType->papsParameters->NumElements(); i++)
		{
			szInner.Append((i == 0) ? "" : ", ");
			AppendTypeName(&szInner, psType->papsParameters->GetPtr(i));
		}
		if (psType->bVariadic)
		{
			szInner.Append((i == 0) ? "..." : ", ...");
		}
		szInner.Append(')');
		AppendTypeNameAround(psz, psType->psBase, &szInner);
		szInner.Kill();
		return;

	default:
		break;
	}
	szInner.Kill();

	//The type's own name, then what is declared of it: a space before (*) or A::*, none before * or &.
	psz->Append(psType->bConst ? "const " : "");
	psz->Append(psType->bVolatile ? "volatile " : "");
	switch (psType->eKind)
	{
	case CPP65TK_Void:
		psz->Append("void");
		break;
	case CPP65TK_Bool:
		psz->Append("bool");
		break;
	case CPP65TK_NullPointer:
		psz->Append("std::nullptr_t");
		break;
	case CPP65TK_Integer:
		if (psType->pcEnum)
		{
			AppendScopeName(psz, psType->pcEnum->mpcScope);
			if (psType->pcEnum->mlliName >= 0)
			{
				AppendIdentifier(psz, psType->pcEnum->mlliName);
			}
			else
			{
				psz->Append("(unnamed enum)");
			}
			break;
		}
		psz->Append(IntegerKindName(psType->eInteger));
		break;
	case CPP65TK_Float:
		psz->Append(psType->bLongDouble ? "long double" : ((psType->iBytes == 4) ? "float" : "double"));
		break;
	case CPP65TK_Class:
		AppendClassName(psz, psType->pcClass);
		break;
	default:
		psz->Append("?");
		break;
	}
	if (!pszInner->Empty())
	{
		if ((pszInner->GetChar(0) == '(') || (pszInner->GetChar(0) == '_') || ((pszInner->GetChar(0) >= 'A') && (pszInner->GetChar(0) <= 'Z')) || ((pszInner->GetChar(0) >= 'a') && (pszInner->GetChar(0) <= 'z')))
		{
			psz->Append(' ');
		}
		psz->Append(pszInner);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A class's name as the program writes it: N::Outer::Inner, N::Box<long>,
// P<char>::Q<long>.  A template instance's arguments are what its template's
// parameters are bound to (for an instance of a partial specialisation,
// what the specialisation's are).  A closure is <lambda>, a class with no
// name <unnamed>.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendClassName(CChars* psz, CCPPClass* pcClass)
{
	SCPPBinding*	psBinding;
	size			i;
	size			j;
	bool			bFirst;

	if (pcClass->mpsLambda)
	{
		psz->Append("<lambda>");
		return;
	}
	if (pcClass->IsUnnamed())
	{
		psz->Append("<unnamed>");
		return;
	}
	if (pcClass->mpcTemplate && pcClass->mpcTemplate->mpcPrimary->mpcClass)
	{
		AppendClassName(psz, pcClass->mpcTemplate->mpcPrimary->mpcClass);
		psz->Append("::");
	}
	else if (pcClass->mpcSymbol)
	{
		AppendScopeName(psz, pcClass->mpcSymbol->mpcScope);
	}
	AppendIdentifier(psz, pcClass->mlliName);
	if (!pcClass->mpcTemplate || !pcClass->mpcBindings)
	{
		return;
	}
	psz->Append('<');
	bFirst = true;
	for (i = 0; i < pcClass->mpcBindings->macBindings.NumElements(); i++)
	{
		psBinding = pcClass->mpcBindings->macBindings.Get(i);
		if (psBinding->papsPack)
		{
			for (j = 0; j < psBinding->papsPack->NumElements(); j++)
			{
				psz->Append(bFirst ? "" : ", ");
				AppendTypeName(psz, psBinding->papsPack->GetPtr(j));
				bFirst = false;
			}
			continue;
		}
		if (psBinding->palliPack)
		{
			for (j = 0; j < psBinding->palliPack->NumElements(); j++)
			{
				psz->Append(bFirst ? "" : ", ");
				psz->Append(psBinding->palliPack->GetValue(j));
				bFirst = false;
			}
			continue;
		}
		psz->Append(bFirst ? "" : ", ");
		bFirst = false;
		if (psBinding->psType)
		{
			AppendTypeName(psz, psBinding->psType);
		}
		else if (psBinding->pcTemplate && psBinding->pcTemplate->mpcPrimary && psBinding->pcTemplate->mpcPrimary->mpcSymbol)
		{
			AppendScopeName(psz, psBinding->pcTemplate->mpcPrimary->mpcSymbol->mpcScope);
			AppendIdentifier(psz, psBinding->pcTemplate->mpcPrimary->mpcSymbol->mlliName);
		}
		else
		{
			psz->Append(psBinding->lliValue);
		}
	}
	psz->Append('>');
}


//////////////////////////////////////////////////////////////////////////
//
// The named namespaces and classes pcScope is in, outermost first, each
// followed by ::.  A namespace without a name is (anonymous namespace)::.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendScopeName(CChars* psz, CCPPScope* pcScope)
{
	if (!pcScope || ((pcScope->meKind != CPPSC_Class) && (pcScope->meKind != CPPSC_Namespace)))
	{
		return;
	}
	AppendScopeName(psz, pcScope->mpcParent);
	if (pcScope->mpcOwner && (pcScope->mpcOwner->mlliName >= 0))
	{
		AppendIdentifier(psz, pcScope->mpcOwner->mlliName);
		psz->Append("::");
	}
	else if (pcScope->IsUnnamedNamespace())
	{
		psz->Append("(anonymous namespace)::");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An identifier's text.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendIdentifier(CChars* psz, int64 lliName)
{
	char*	szName;
	int		iLength;

	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	psz->Append(szName, iLength);
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
	size			i;

	for (i = 0; !mpcInitializerList && (i < mapcTemplates.NumElements()); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->mpcSymbol || (pcTemplate->mpcSymbol->mlliName < 0))
		{
			continue;
		}
		if (IsStdName(pcTemplate->mpcSymbol->mlliName, pcTemplate->mpcNamespace, "initializer_list"))
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
	if (IsUncapturedConstant(pcArgument))
	{
		//A constant a lambda doesn't capture has no object in it to refer to: binding a reference to it odr-uses it.
		//[basic.def.odr] 3
		ReportOnce(pcArgument, "The lambda doesn't capture the variable");
		return true;
	}
	psPlace = PlaceOf(pcArgument);
	return !psPlace || !psReference->psBase->Equals(psPlace->WithoutReference());
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression names a constant of the function a lambda being
// written is in, which the lambda doesn't capture (CPPBN_EnclosingConstant):
// only its value can be used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsUncapturedConstant(CCPPSyntax* pcExpression)
{
	CCPPBoundName	cBound;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression || (pcExpression->GetKind() != CPPS_IdExpression))
	{
		return false;
	}
	mcBinder.BindName((CCPPSyntaxIdExpression*)pcExpression, &cBound);
	return cBound.meWhat == CPPBN_EnclosingConstant;
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


