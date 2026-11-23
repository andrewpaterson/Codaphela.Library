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
// Numbers, characters, true, false and nullptr.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::Literal(CCPPSyntaxLiteralExpression* pcLiteral)
{
	CCPPToken*		pcToken;

	pcToken = mpcTokeniser->Get(pcLiteral->miLiteral);
	if (pcToken->IsUserLiteral())
	{
		mcDiagnostics.Error(pcLiteral, "No literal operator takes the literal");
		return NULL;
	}
	if (pcToken->IsNumber())
	{
		switch (((CCTNumber*)pcToken)->meType)
		{
		case CTNT_Int:
			return ((CCTInt*)pcToken)->mbChar32 ? &mcTypes.msChar32 : IntegerLiteralType((CCTInt*)pcToken);
		case CTNT_Short:
			//L'x' and u'x'.
			return ((CCTShort*)pcToken)->mbChar16 ? &mcTypes.msChar16 : &mcTypes.msWChar;
		case CTNT_LongLong:
			return ((CCTLongLong*)pcToken)->IsSigned() ? &mcTypes.msLongLong : &mcTypes.msUnsignedLongLong;
		case CTNT_Float:
			return &mcTypes.msFloat;
		case CTNT_Double:
			return &mcTypes.msDouble;
		case CTNT_LongDouble:
			return &mcTypes.msLongDouble;
		default:
			mcDiagnostics.Error(pcLiteral, "This literal is not supported by the compiler yet");
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
	//nullptr is a std::nullptr_t.  [lex.nullptr]
	return &mcTypes.msNullPointer;
}


//////////////////////////////////////////////////////////////////////////
//
// An int literal is int (or unsigned int) if it fits and has no L suffix, otherwise long (or unsigned long): only
// different where int is narrower than 32 bits (the W65C816).  A hex or octal one too big for an int is an unsigned
// int if it fits one ([lex.icon]).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::IntegerLiteralType(CCTInt* pcInt)
{
	uint64	ulliValue;
	uint64	ulliIntMax;

	//An L literal is long (unsigned long if it's unsigned), whatever int is.
	ulliValue = (uint32)pcInt->mcValue;
	ulliIntMax = (uint64)mcTypes.msInt.Most();
	if (pcInt->IsSigned())
	{
		if (!pcInt->mbLong && (ulliValue <= ulliIntMax))
		{
			return &mcTypes.msInt;
		}
		if (!pcInt->mbLong && !pcInt->mbDecimal && (ulliValue <= ulliIntMax * 2 + 1))
		{
			return &mcTypes.msUnsignedInt;
		}
		return &mcTypes.msLong;
	}
	if (!pcInt->mbLong && (ulliValue <= ulliIntMax * 2 + 1))
	{
		return &mcTypes.msUnsignedInt;
	}
	return &mcTypes.msUnsignedLong;
}


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
// Whether an expression is a string literal, its characters: not a
// user-defined one ("text"_s), which is its literal operator's call.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsStringLiteral(CCPPSyntax* pcExpression)
{
	return pcExpression && (pcExpression->GetKind() == CPPS_StringExpression) && !mcDesugarings.UserLiteralToken(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a character literal's token (char, wchar_t, char16_t or
// char32_t), or NULL if it is no character literal of one character.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::CharacterTokenType(CCPPToken* pcToken)
{
	if (pcToken->IsChar())
	{
		return &mcTypes.msChar;
	}
	if (pcToken->IsNumber() && (((CCTNumber*)pcToken)->meType == CTNT_Short))
	{
		return ((CCTShort*)pcToken)->mbChar16 ? &mcTypes.msChar16 : &mcTypes.msWChar;
	}
	if (pcToken->IsNumber() && (((CCTNumber*)pcToken)->meType == CTNT_Int) && ((CCTInt*)pcToken)->mbChar32)
	{
		return &mcTypes.msChar32;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name that isn't a function's is ambiguous: lookup finds two
// different things where it finds it (CCPPScope::IsAmbiguous, or for a
// name qualified by a namespace IsAmbiguousMember).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsAmbiguousName(CCPPSyntaxName* pcName)
{
	CCPPSyntaxNameComponent*	pcQualifier;
	int64						lliName;

	if (!pcName->mpcSymbol || pcName->mpcSymbol->IsFunction() || (pcName->GetLast()->meKind != CPPNC_Identifier))
	{
		return false;
	}
	lliName = pcName->mpcSymbol->mlliName;
	pcQualifier = pcName->GetQualifier();
	if (pcQualifier)
	{
		return pcQualifier->mpcSymbol && (pcQualifier->mpcSymbol->meKind == CPPSK_Namespace) && pcQualifier->mpcSymbol->mpcMembers && pcQualifier->mpcSymbol->mpcMembers->IsAmbiguousMember(lliName);
	}
	return !pcName->mbGlobal && pcName->mpcLookup && pcName->mpcLookup->IsAmbiguous(lliName);
}


//////////////////////////////////////////////////////////////////////////
//
// What a new expression makes: its type-id's type (auto's is its
// initialiser's), and for new Type[count][N]... arrays of the inner bounds
// (constants).  A type-id that is an array by itself (NewTypeIdArray) makes
// the array's elements.  NULL after an error.  [expr.new] 2, 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::NewElementType(CCPPSyntaxNewExpression* pcNew)
{
	CArraySyntax*	papcElements;
	CCPPSyntax*		pcInitialiser;
	SCPP65Type*		psType;
	int64			lliCount;
	size			i;

	//new auto(x) is of x's type: the one thing in its brackets.  [expr.new] 2
	pcInitialiser = NULL;
	if (mcDeclarators.IsAuto(pcNew->mpcType->mpcSpecifiers) && pcNew->mpcInitializer)
	{
		papcElements = ListElements(pcNew->mpcInitializer);
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : NULL;
	}
	psType = pcInitialiser ? mcDeclarators.GetType(pcNew->mpcType->mpcSpecifiers, pcNew->mpcType->mpcDeclarator, pcNew->mpcType, pcInitialiser) : mcDeclarators.GetTypeIdType(pcNew->mpcType);
	for (i = pcNew->macpArraySizes.NumElements(); psType && (i > 1); i--)
	{
		if (!mcFolder.Constant(pcNew->macpArraySizes.GetPtr(i - 1), &lliCount) || (lliCount <= 0))
		{
			mcFolder.ConstantError(pcNew->macpArraySizes.GetPtr(i - 1), "A new array's bounds after the first must be positive constants");
			return NULL;
		}
		psType = mcTypes.Array(psType, (int)lliCount);
	}
	return (psType && psType->IsArray() && (pcNew->macpArraySizes.NumElements() == 0)) ? psType->psBase : psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The array a new expression's type-id is by itself, with no bound after
// it: new (int[3]), or new A where A is a typedef of an array.  It makes the
// array's elements, as new int[3] does, and gives a pointer to the first.
// NULL if it isn't one.  [expr.new] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::NewTypeIdArray(CCPPSyntaxNewExpression* pcNew)
{
	SCPP65Type*	psType;

	if ((pcNew->macpArraySizes.NumElements() > 0) || mcDeclarators.IsAuto(pcNew->mpcType->mpcSpecifiers))
	{
		return NULL;
	}
	mcDiagnostics.BeginQuiet();
	psType = mcDeclarators.GetTypeIdType(pcNew->mpcType);
	mcDiagnostics.EndQuiet();
	return (psType && psType->IsArray()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a new expression makes an array: by operator new[], its elements
// each constructed.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsArrayNew(CCPPSyntaxNewExpression* pcNew)
{
	return (pcNew->macpArraySizes.NumElements() > 0) || (NewTypeIdArray(pcNew) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// A new expression's allocation arguments: its size (NewSize), then its
// placement arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::NewArguments(CCPPSyntaxNewExpression* pcNew, CArraySyntax* papcArguments)
{
	size	i;

	papcArguments->Add(NewSize(pcNew));
	for (i = 0; i < pcNew->macpPlacement.NumElements(); i++)
	{
		papcArguments->Add(pcNew->macpPlacement.GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The size a new expression's allocation function is given: a size_t.
// [expr.new] 9
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::NewSize(CCPPSyntaxNewExpression* pcNew)
{
	return mcDesugarings.ImplicitValue(pcNew, mcTypes.Size());
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
// Whether the class or a base has a member function of the name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasMember(CCPPClass* pcClass, int64 lliName)
{
	CCPPFunction*	pcFunction;
	size			i;

	if (!CompleteClass(pcClass, NULL))
	{
		return false;
	}
	for (pcFunction = FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->mpcClass == pcClass) && (pcFunction->meKind == CPPFK_Function))
		{
			return true;
		}
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (HasMember(pcClass->macBases.Get(i)->pcClass, lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcClass (or a base) has an operator function named lliName: a member function, or a member template taking
// uiArguments.  The class is laid out to find out, if it hasn't been: its members and its using-declarations are
// known only then.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasOperator(CCPPClass* pcClass, int64 lliName, size uiArguments)
{
	size	i;

	if (lliName == GetOperatorName(CT_Assignment, false))
	{
		//A base's operator= is hidden by the copy assignment operator every class declares: only the class's own
		//are, and those a using-declaration in it names (using Base::operator=;).  [over.ass] 1, [namespace.udecl] 4
		if (!CompleteClass(pcClass, NULL))
		{
			return false;
		}
		for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
		{
			if (pcClass->mapcMembers.GetPtr(i)->mlliName == lliName)
			{
				return true;
			}
		}
		for (i = 0; i < pcClass->macUsings.NumElements(); i++)
		{
			if (pcClass->macUsings.Get(i)->lliName == lliName)
			{
				return true;
			}
		}
		return mcBinder.HasOwnMemberTemplate(pcClass, lliName, uiArguments);
	}
	return HasMember(pcClass, lliName) || mcBinder.HasMemberTemplate(pcClass, lliName, uiArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// The enumerator Class::Name names.  See QualifiedMember.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPClass** ppcClass)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = QualifiedMember(pcName, CPPSK_Enumerator, pcNode, ppcClass);
	if (!pcSymbol)
	{
		*ppcClass = NULL;
		pcSymbol = DecltypeEnumerator(pcName);
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// decltype(e)::name where e is of an enumeration: the enumeration's
// enumerator of the name, or NULL.  [dcl.type.simple] 4, [dcl.enum] 11
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::DecltypeEnumerator(CCPPSyntaxName* pcName)
{
	SCPP65Type*	psType;
	CCPPSymbol*	pcEnum;
	CCPPSymbol*	pcSymbol;

	if (!pcName->IsQualifiedByDecltype() || (pcName->macpComponents.NumElements() != 2))
	{
		return NULL;
	}
	mcDiagnostics.BeginSilent();
	psType = mcDeclarators.GetTypeIdType(((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0))->mpcType);
	mcDiagnostics.EndSilent();
	pcEnum = psType ? psType->WithoutReference()->pcEnum : NULL;
	if (!pcEnum)
	{
		return NULL;
	}

	//A scoped enumeration's enumerators are its members; one that isn't scoped has them in the scope it is in too.
	pcSymbol = pcEnum->mpcMembers ? pcEnum->mpcMembers->FindHere(GetComponentName(pcName->GetLast())) : NULL;
	pcSymbol = pcSymbol ? pcSymbol : (pcEnum->mpcScope ? pcEnum->mpcScope->FindHere(GetComponentName(pcName->GetLast())) : NULL);
	psType = (pcSymbol && (pcSymbol->meKind == CPPSK_Enumerator)) ? EnumeratorType(pcSymbol) : NULL;
	return (psType && (psType->pcEnum == pcEnum)) ? pcSymbol : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type sizeof or typeid is asked of: its type-id's (pcType), or its
// expression's, which isn't evaluated.  decltype(d)::name, which the parser
// takes for a type where one can be written (only the type of d says which
// it is), is the member it names when it names no type.  NULL after an
// error.  [expr.sizeof] 1, [expr.typeid] 3, 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::OperandType(CCPPSyntaxTypeId* pcType, CCPPSyntax* pcOperand)
{
	SCPP65Type*	psType;
	CCPPSyntax*	pcMember;

	if (!pcType)
	{
		return mcTypeChecker.UnevaluatedTypeOf(pcOperand);
	}
	pcMember = mcDesugarings.DecltypeMember(pcType);
	if (!pcMember)
	{
		return mcDeclarators.GetTypeIdType(pcType);
	}
	mcDiagnostics.BeginSilent();
	psType = mcDeclarators.GetTypeIdType(pcType);
	mcDiagnostics.EndSilent();
	return psType ? psType : mcTypeChecker.UnevaluatedTypeOf(pcMember);
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
// The member of the kind eKind Class::Name names, where Class is a class, an instance of a class template, a typedef of
// one or a template parameter, and in ppsClass the class that declares it: a member of an instance can depend on its
// template arguments, so it is worked out with that class's bindings.  NULL if it isn't one.  A qualifier after a
// class is found in that class (QualifiersClass): TEMPLATE::SizePolicy::NotFound and W<int>::Type::N.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, CCPPClass** ppcClass)
{
	CCPPSyntaxNameComponent*	pcQualifier;
	CCPPClass*				pcClass;
	CCPPSymbol*					pcSymbol;
	bool						bAmbiguous;

	if (pcName->macpComponents.NumElements() < 2)
	{
		return NULL;
	}
	pcQualifier = pcName->GetQualifier();
	if (!pcQualifier)
	{
		return NULL;
	}
	if (pcQualifier->mpcSymbol)
	{
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
	}
	else if ((pcName->macpComponents.NumElements() < 3) && (pcQualifier->meKind != CPPNC_Decltype))
	{
		return NULL;
	}

	mcDiagnostics.BeginSilent();
	pcClass = (pcQualifier->mpcSymbol && (pcName->macpComponents.NumElements() == 2)) ? GetComponentClass(pcQualifier, NULL, pcNode) : QualifiersClass(pcName, pcNode);
	pcSymbol = (pcClass && CompleteClass(pcClass, pcNode)) ? pcClass->FindMemberSymbol(GetComponentName(pcName->GetLast()), eKind, ppcClass) : NULL;
	bAmbiguous = pcSymbol && (pcClass->macBases.NumElements() > 1) && IsAmbiguousInBases(pcClass, GetComponentName(pcName->GetLast()));
	mcDiagnostics.EndSilent();
	if (bAmbiguous)
	{
		//C::T, where two of C's bases each have a T.  [class.member.lookup] 6
		mcDiagnostics.ReportOnce(pcNode, "The name is a member of more than one of the class's bases");
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name looked up in a class is found as different things in
// its bases, neither hidden by one of the class's own (or of a class
// nearer than both): struct C : A, B { } where A and B each have a T, or
// are two instances of the template that has it.  The same class's member
// reached through two bases is one thing, and one in a virtual base of
// another class that has the name is hidden by that class's (there is one
// of the base).  [class.member.lookup] 3 - 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsAmbiguousInBases(CCPPClass* pcClass, int64 lliName)
{
	CArrayCPPClasses	apcFound;
	CCPPClass*			pcFound;
	size				uiMeanings;
	size				i;
	size				j;
	bool				bHidden;

	mpcStatistics->Count(CPPCC_BaseWalks);
	if (pcClass->macBases.NumElements() == 0)
	{
		return false;
	}
	apcFound.Init();
	FindInBases(pcClass, lliName, &apcFound);
	uiMeanings = 0;
	for (i = 0; i < apcFound.NumElements(); i++)
	{
		pcFound = apcFound.GetPtr(i);
		bHidden = false;
		for (j = 0; !bHidden && (j < apcFound.NumElements()); j++)
		{
			bHidden = (j != i) && apcFound.GetPtr(j)->GetVirtualBase(pcFound);
		}
		uiMeanings += bHidden ? 0 : 1;
	}
	apcFound.Kill();
	return uiMeanings > 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A name not qualified, in a member function, that the parser found as a
// member of a base of the function's class (a type, a static member, an
// enumerator): an error if more than one of the class's bases has it, as
// lookup in the class then finds more than one.  Fields and functions are
// checked where they are bound (CCPPBinder::HasOneMeaning).
// [class.member.lookup] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckBaseMemberName(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSymbol*		pcSymbol;
	CCPPScope*		pcScope;
	CCPPFunction*	pcFunction;
	CCPPClass*		pcClass;

	pcSymbol = pcName->mpcSymbol;
	pcFunction = mcContext.GetFunction();
	pcClass = pcFunction ? pcFunction->mpcClass : NULL;
	if (!pcSymbol || !pcClass || (pcName->macpComponents.NumElements() != 1) || !pcSymbol->mpcScope || (pcClass->macBases.NumElements() < 2))
	{
		return true;
	}
	pcScope = pcSymbol->mpcScope;
	if ((pcScope->meKind == CPPSC_Enum) && pcScope->mpcParent)
	{
		//An unscoped enum's enumerator is its class's member.
		pcScope = pcScope->mpcParent;
	}
	if ((pcScope->meKind != CPPSC_Class) || !pcScope->mpcOwner || (pcScope->mpcOwner->meKind != CPPSK_Class) || (pcClass->mpcSymbol == pcScope->mpcOwner))
	{
		return true;
	}
	return !IsAmbiguousInBases(pcClass, pcSymbol->mlliName) || mcDiagnostics.Error(pcNode, "The name is a member of more than one of the class's bases");
}


//////////////////////////////////////////////////////////////////////////
//
// The classes whose declaration of a name lookup in a class finds: the
// class itself, whose own hides its bases', or else each base's.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::FindInBases(CCPPClass* pcClass, int64 lliName, CArrayCPPClasses* papcFound)
{
	CCPPScope*	pcScope;
	size		i;

	mpcStatistics->Count(CPPCC_BaseWalks);
	pcScope = pcClass->mpcSpecifier ? pcClass->mpcSpecifier->mpcScope : ((pcClass->mpcSymbol) ? pcClass->mpcSymbol->mpcMembers : NULL);
	if (pcScope && pcScope->FindHere(lliName))
	{
		if (papcFound->Find(pcClass) == ARRAY_ELEMENT_NOT_FOUND)
		{
			papcFound->Add(pcClass);
		}
		return;
	}
	if (!ResolveBases(pcClass, NULL))
	{
		return;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		FindInBases(pcClass->macBases.Get(i)->pcClass, lliName, papcFound);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified name the parser couldn't resolve names: typename
// C::In or typename C::template In<long>, with C bound to a class, is that
// class's member (MemberClass).  NULL if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::QualifiedClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPClass*	pcClass;

	pcClass = (pcName->macpComponents.NumElements() > 1) ? QualifiersClass(pcName, pcNode) : NULL;
	return pcClass ? MemberClass(pcClass, pcName->GetLast(), pcNode) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The member typedef or class of pcClass a name component names, worked out
// with the bindings of the class that declares it, or the instance of its
// member class template (In<long>, its arguments as they are where the name
// is), or the base of pcClass it names.  NULL if it is none of them.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::MemberClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	CCPPClass*		pcDeclaring;
	CCPPClass*		pcBase;
	CCPPSymbol*		pcMember;
	CCPPTemplate*	pcTemplate;
	SCPP65Type*		psType;
	int64			lliName;

	if (!CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	lliName = GetComponentName(pcComponent);
	pcDeclaring = NULL;
	pcMember = pcClass->FindMemberSymbol(lliName, CPPSK_ClassTemplate, &pcDeclaring);
	pcTemplate = (pcMember && pcComponent->mbTemplateArguments) ? mcInstantiator.MemberClassTemplate(pcMember, pcDeclaring) : NULL;
	if (pcTemplate)
	{
		return mcInstantiator.InstantiateClass(pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
	}
	pcMember = pcMember ? NULL : pcClass->FindMemberSymbol(lliName, CPPSK_Typedef, &pcDeclaring);
	if (!pcMember)
	{
		pcMember = pcClass->FindMemberSymbol(lliName, CPPSK_Class, &pcDeclaring);
	}
	if (!pcMember || (pcMember == pcDeclaring->mpcSymbol))
	{
		//C::X<char>::Type, where X<char> is C's base: a base's name is found in the class (its injected class name).
		//[class.qual] 1, [temp.local] 1
		pcMember = pcComponent->mpcSymbol;
		pcBase = (pcMember && ((pcMember->meKind == CPPSK_Class) || (pcMember->meKind == CPPSK_ClassTemplate))) ? GetComponentClass(pcComponent, NULL, pcNode) : NULL;
		return (pcBase && IsDerived(pcClass, pcBase, NULL)) ? pcBase : NULL;
	}
	mcContext.EnterBindings(pcDeclaring->mpcBindings);
	if (pcMember->meKind == CPPSK_Typedef)
	{
		psType = mcDeclarators.GetSymbolType(pcMember, pcNode);
		pcClass = (psType && psType->IsClass()) ? psType->pcClass : NULL;
	}
	else
	{
		pcClass = GetComponentClass(pcComponent, pcMember, pcNode);
	}
	mcContext.LeaveBindings();
	return pcClass;
}




//////////////////////////////////////////////////////////////////////////
//
// The class pcName's qualifiers (its components before the last) name.  A component after a class is the member
// typedef, class or class template instance of that name of the class (MemberClass): in
// TEMPLATE::SizePolicy, SizePolicy is a typedef of the class TEMPLATE is bound to, or of one of its bases (the parser
// leaves it unresolved), and in W<int>::Type, Type is W<int>'s typedef, whatever W's T is (the parser found W's).
// NULL if one isn't a class.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::QualifiersClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPClass*					pcClass;
	size						i;

	pcClass = NULL;
	for (i = 0; i + 1 < pcName->macpComponents.NumElements(); i++)
	{
		pcComponent = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(i);
		if (pcComponent->mpcSymbol && (pcComponent->mpcSymbol->meKind == CPPSK_Namespace))
		{
			pcClass = NULL;
			continue;
		}
		pcClass = pcClass ? MemberClass(pcClass, pcComponent, pcNode) : GetComponentClass(pcComponent, NULL, pcNode);
		if (!pcClass)
		{
			return NULL;
		}
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified name's last qualifier names (GetComponentClass).
// One the parser couldn't resolve is found in the class before it
// (TEXT::SizePolicy::Copy(...)), and so is a member class named through a
// class template's instance, which is that instance's own
// (Outer<char>::Inner::Count()): QualifiersClass.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::NameQualifierClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSyntaxNameComponent*	pcQualifier;

	pcQualifier = pcName->GetQualifier();
	if (pcQualifier->mpcSymbol && !((pcQualifier->mpcSymbol->meKind == CPPSK_Class) && pcName->IsQualifiedByInstance()))
	{
		return GetComponentClass(pcQualifier, NULL, pcNode);
	}
	return QualifiersClass(pcName, pcNode);
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
// A call to a function whose declaration could not be generated: what
// declaring it said is said now (as if said here: in quiet code it only
// fails).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ReportFailedDeclaration(int64 lliName)
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	size					i;

	for (i = 0; i < masFailed.NumElements(); i++)
	{
		pcDeclarator = masFailed.Get(i)->pcDeclarator;
		if (pcDeclarator->GetName() && (GetFunctionName(pcDeclarator->GetName()->GetLast()) == lliName))
		{
			mcDiagnostics.Repeat(masFailed.Get(i)->pcErrors);
			return;
		}
	}
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
		return ListElements(pcInitialiser);
	}
	return NULL;
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
	CPP65TT_TriviallyCopyable,
	CPP65TT_Pod,
	CPP65TT_StandardLayout,
	CPP65TT_LiteralType,
	CPP65TT_Final,
	CPP65TT_BaseOf,
	CPP65TT_ConvertibleTo,
	CPP65TT_Constructible,
	CPP65TT_Assignable,
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
	{ "__is_pod", CPP65TT_Pod },
	{ "__is_trivial", CPP65TT_Trivial },
	{ "__is_trivially_copyable", CPP65TT_TriviallyCopyable },
	{ "__has_trivial_copy", CPP65TT_TriviallyCopyable },
	{ "__is_standard_layout", CPP65TT_StandardLayout },
	{ "__is_literal_type", CPP65TT_LiteralType },
	{ "__is_literal", CPP65TT_LiteralType },
	{ "__is_final", CPP65TT_Final },
	{ "__is_sealed", CPP65TT_Final },
	{ "__is_base_of", CPP65TT_BaseOf },
	{ "__is_convertible_to", CPP65TT_ConvertibleTo },
	{ "__is_convertible", CPP65TT_ConvertibleTo },
	{ "__is_constructible", CPP65TT_Constructible },
	{ "__is_assignable", CPP65TT_Assignable },
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
// A compiler type trait's value (see gasCPP65TypeTraits), of its types.  A
// class is abstract when one of its virtual functions is still pure; it is
// trivially copyable when copying, assigning and destroying it is only its
// bytes (nothing the compiler writes runs, and it has no vtable), trivial
// when making one is too, standard layout when its fields are laid out as
// C's are, and a POD when both.  [meta.unary.prop]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::TypeTrait(CCPPSyntaxTypeTraitExpression* pcTrait, int64* plliValue)
{
	CArrayCPP65Types	apsTypes;
	SCPP65Type*			psType;
	CCPPClass*			pcClass;
	CChars				szTrait;
	ECPP65TypeTrait		eTrait;
	size				i;
	bool				bResult;

	szTrait.Init();
	mpcTokeniser->GetSource(mpcTokeniser->Get(pcTrait->miName))->Append(&szTrait);
	eTrait = CPP65FindTypeTrait(&szTrait);
	szTrait.Kill();
	if (eTrait == CPP65TT_None)
	{
		return false;
	}

	apsTypes.Init();
	bResult = pcTrait->macpArguments.NumElements() >= 1;
	for (i = 0; bResult && (i < pcTrait->macpArguments.NumElements()); i++)
	{
		psType = (pcTrait->macpArguments.GetPtr(i)->GetKind() == CPPS_TypeId) ? mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)pcTrait->macpArguments.GetPtr(i)) : NULL;
		pcClass = (psType && psType->WithoutReference()->IsClass()) ? psType->WithoutReference()->pcClass : NULL;
		bResult = psType && (!pcClass || CompleteClass(pcClass, pcTrait));
		if (bResult)
		{
			apsTypes.Add(psType);
		}
	}
	if (bResult && (apsTypes.NumElements() < (size)(((eTrait == CPP65TT_BaseOf) || (eTrait == CPP65TT_ConvertibleTo) || (eTrait == CPP65TT_Assignable)) ? 2 : 1)))
	{
		bResult = mcDiagnostics.Error(pcTrait, "The type trait needs more types");
	}
	if (bResult)
	{
		*plliValue = TypeTraitValue(eTrait, &apsTypes);
	}
	apsTypes.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A type trait's value for its types (each complete).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::TypeTraitValue(int iTrait, CArrayCPP65Types* papsTypes)
{
	SCPP65Type*			psType;
	SCPP65Type*			psOther;
	CCPPClass*			pcClass;
	CArrayCPP65Types	apsArguments;
	size				i;
	bool				bResult;

	psType = papsTypes->GetPtr(0);
	psOther = (papsTypes->NumElements() > 1) ? papsTypes->GetPtr(1) : NULL;
	pcClass = psType->IsClass() ? psType->pcClass : NULL;
	switch ((ECPP65TypeTrait)iTrait)
	{
	case CPP65TT_Class:					return pcClass && !pcClass->mbUnion;
	case CPP65TT_Union:					return pcClass && pcClass->mbUnion;
	case CPP65TT_Enum:					return psType->pcEnum != NULL;
	case CPP65TT_Empty:					return pcClass && pcClass->mbEmpty;
	case CPP65TT_Polymorphic:			return pcClass && pcClass->IsPolymorphic();
	case CPP65TT_Abstract:				return pcClass && pcClass->IsAbstract();
	case CPP65TT_Final:					return pcClass && pcClass->mpcSpecifier && pcClass->mpcSpecifier->mbFinal;
	case CPP65TT_TrivialConstructor:	return !pcClass || !pcClass->NeedsConstructor();
	case CPP65TT_TrivialDestructor:		return !pcClass || !pcClass->NeedsDestructor();
	case CPP65TT_TriviallyCopyable:		return IsTriviallyCopyableType(psType);
	case CPP65TT_Trivial:				return IsTriviallyCopyableType(psType) && (!pcClass || !pcClass->NeedsConstructor());
	case CPP65TT_StandardLayout:		return IsStandardLayoutType(psType);
	case CPP65TT_Pod:					return IsTriviallyCopyableType(psType) && (!pcClass || !pcClass->NeedsConstructor()) && IsStandardLayoutType(psType);
	case CPP65TT_LiteralType:			return mpcConstants->IsLiteralType(psType);
	case CPP65TT_BaseOf:
		//A class is its own base here.  [meta.rel] Table 49
		return pcClass && psOther->IsClass() && ((pcClass == psOther->pcClass) || IsDerived(psOther->pcClass, pcClass, NULL));
	case CPP65TT_ConvertibleTo:
		return mcBinder.IsConvertible(psOther, psType);
	case CPP65TT_Constructible:
		apsArguments.Init();
		for (i = 1; i < papsTypes->NumElements(); i++)
		{
			apsArguments.Add(papsTypes->GetPtr(i));
		}
		bResult = mcBinder.IsConstructible(psType, &apsArguments);
		apsArguments.Kill();
		return bResult;
	case CPP65TT_Assignable:
		return mcBinder.IsAssignable(psType, psOther);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if copying, assigning and destroying an object of the type is only
// its bytes.  [basic.types] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsTriviallyCopyableType(SCPP65Type* psType)
{
	CCPPClass*	pcClass;

	while (psType->IsArray())
	{
		psType = psType->psBase;
	}
	if (!psType->IsClass())
	{
		return !psType->IsReference();
	}
	pcClass = psType->pcClass;
	return pcClass->IsTriviallyCopyable() && pcClass->IsTriviallyAssignable() && !pcClass->NeedsDestructor() && !pcClass->IsPolymorphic();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the type is laid out as C's types are: no vtable, no reference,
// standard layout fields and bases, and fields in only one class of its
// hierarchy.  [class] 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsStandardLayoutType(SCPP65Type* psType)
{
	CCPPClass*	pcClass;
	CCPPClass*	pcFields;
	size		i;

	while (psType->IsArray())
	{
		psType = psType->psBase;
	}
	if (!psType->IsClass())
	{
		return !psType->IsReference();
	}
	pcClass = psType->pcClass;
	if (pcClass->IsPolymorphic())
	{
		return false;
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		if (!IsStandardLayoutType(pcClass->macFields.Get(i)->psType))
		{
			return false;
		}
	}
	pcFields = (pcClass->macFields.NumElements() > 0) ? pcClass : NULL;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (!IsStandardLayoutType(pcClass->macBases.Get(i)->pcClass->mpsType))
		{
			return false;
		}
		if (!pcClass->macBases.Get(i)->pcClass->mbEmpty)
		{
			if (pcFields)
			{
				return false;
			}
			pcFields = pcClass->macBases.Get(i)->pcClass;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An enumerator's type: its enum's (an unnamed enum's is its underlying type).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::EnumeratorType(CCPPSymbol* pcSymbol)
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
SCPP65Type* CCPPUnit::EnumType(CCPPSymbol* pcEnum, CCPPSyntaxEnumSpecifier* pcSpecifier)
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
	int						iBytes;
	bool					bSigned;
	bool					bNonNegative;
	size					i;

	bNonNegative = false;
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
	psBase = NULL;
	if (pcSpecifier && pcSpecifier->mpcBase)
	{
		//Fixed: enum E : short is a short underneath, though int is as wide.  Each value given is one of that type's.
		//[dcl.enum] 5
		psBase = mcDeclarators.GetTypeIdType(pcSpecifier->mpcBase);
		psBase = (psBase && psBase->IsInteger()) ? psBase->Unqualified() : NULL;
		for (i = 0; psBase && (i < pcSpecifier->macpEnumerators.NumElements()); i++)
		{
			pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
			if (pcEnumerator->mpcValue && mcFolder.Constant(pcEnumerator->mpcValue, &lliValue))
			{
				CheckEnumeratorFits(pcSpecifier, pcEnumerator, lliValue);
			}
		}
	}
	else if (pcSpecifier && !pcSpecifier->mbScoped)
	{
		lliValue = 0;
		lliMin = 0;
		lliMax = 0;
		bNonNegative = true;
		for (i = 0; i < pcSpecifier->macpEnumerators.NumElements(); i++)
		{
			pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
			if (pcEnumerator->mpcValue && !QuietConstant(pcEnumerator->mpcValue, &lliValue))
			{
				//One that can't be worked out here (sizeof(T) in a class template's enum, with no T) leaves it an int:
				//what is wrong with it is said where it is checked (CheckEnumerators).
				break;
			}
			lliMin = (lliValue < lliMin) ? lliValue : lliMin;
			lliMax = (lliValue > lliMax) ? lliValue : lliMax;
			lliValue++;
		}
		bNonNegative = (i == pcSpecifier->macpEnumerators.NumElements()) && (lliMin >= 0);

		if ((lliMin >= mcTypes.msInt.Least()) && (lliMax <= mcTypes.msInt.Most()))
		{
			//int.
		}
		else if ((lliMin >= 0) && (lliMax <= mcTypes.msUnsignedInt.Most()))
		{
			bSigned = false;
		}
		else if ((lliMin >= CPPIntegerLeast(4, true)) && (lliMax <= CPPIntegerMost(4, true)))
		{
			iBytes = 4;
		}
		else if ((lliMin >= 0) && (lliMax <= CPPIntegerMost(4, false)))
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
		psType = mcTypes.Enum(pcEnum, psBase ? psBase : mcTypes.Integer(iBytes, bSigned), pcSpecifier && pcSpecifier->mbScoped);
		psType->bNonNegativeEnum = bNonNegative;
		mpcObserver->EnumMade(this, psType);
		return psType;
	}
	return psBase ? psBase : mcTypes.Integer(iBytes, bSigned);
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
bool CCPPUnit::EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue)
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
			if (!mcFolder.Constant(pcOther->mpcValue, &lliValue))
			{
				return false;
			}
			CheckEnumeratorFits(pcEnum, pcOther, lliValue);
			uiValued = i - 1;
			break;
		}
	}
	*plliValue = lliValue + (int64)(uiIndex - uiValued);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value an enumerator is given in an enum with a fixed underlying type
// (enum E : unsigned char) is one of that type's: an error if not.
// [dcl.enum] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckEnumeratorFits(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64 lliValue)
{
	SCPP65Type*	psBase;

	psBase = pcEnum->mpcBase ? mcDeclarators.GetTypeIdType(pcEnum->mpcBase) : NULL;
	if (psBase && !mcInstantiator.ConstantFits(psBase->Unqualified(), lliValue))
	{
		mcDiagnostics.ReportOnce(pcEnumerator->mpcValue, "The constant's value isn't one of the type it is converted to");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the field Class::field names, with no object: what sizeof
// and the like may be given, as nothing is read.  NULL if the expression
// isn't that.  [expr.prim.general] 13
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::QualifiedFieldType(CCPPSyntax* pcExpression)
{
	CCPPSyntaxName*	pcName;
	CCPPClass*		pcClass;
	SCPPField*		psField;
	int				iOffset;

	if (!pcExpression || (pcExpression->GetKind() != CPPS_IdExpression))
	{
		return NULL;
	}
	pcName = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName;
	if (pcName->IsQualifiedByDecltype() && !pcName->mpcSymbol)
	{
		//decltype(d)::m: the parser doesn't know d's class, so the field is found by its name.
		pcClass = QualifiersClass(pcName, pcExpression);
		psField = (pcClass && CompleteClass(pcClass, pcExpression)) ? pcClass->FindField(GetComponentName(pcName->GetLast()), &iOffset) : NULL;
		return psField ? psField->psType : NULL;
	}
	if (!pcName->mpcSymbol || (pcName->mpcSymbol->meKind != CPPSK_Field) || !pcName->IsQualifiedByClass())
	{
		return NULL;
	}
	pcClass = NameQualifierClass(pcName, pcExpression);
	psField = (pcClass && CompleteClass(pcClass, pcExpression)) ? pcClass->FindFieldSymbol(pcName->mpcSymbol, &iOffset) : NULL;
	return psField ? psField->psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the expression is a null pointer constant: nullptr, or an
// integer literal whose value is 0.  [conv.ptr] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsNullPointerConstant(CCPPSyntax* pcExpression)
{
	CCPPToken*	pcToken;
	int64		lliValue;

	pcExpression = pcExpression ? pcExpression->WithoutParentheses() : NULL;
	if (!pcExpression || (pcExpression->GetKind() != CPPS_LiteralExpression))
	{
		return false;
	}
	pcToken = mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcExpression)->miLiteral);
	if (pcToken->Is(CT_nullptr))
	{
		return true;
	}
	return pcToken->IsNumber() && !pcToken->IsUserLiteral() && (((CCTNumber*)pcToken)->meType != CTNT_Float) && (((CCTNumber*)pcToken)->meType != CTNT_Double) &&
		   (((CCTNumber*)pcToken)->meType != CTNT_LongDouble) && mcFolder.Constant(pcExpression, &lliValue) && (lliValue == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::SkipParentheses(CCPPSyntax* pcExpression)
{
	return pcExpression ? pcExpression->WithoutParentheses() : NULL;
}


