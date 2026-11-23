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
#include "CppTokeniser.h"
#include "CPPUnit.h"
#include "CPPTypeWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTypeWriter::Init(CCPPUnit* pcUnit, ECPPTypeWriting eWriting)
{
	mpcUnit = pcUnit;
	meWriting = eWriting;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::AppendType(SCPP65Type* psType, CChars* pszDest)
{
	return AppendDeclaration(psType, NULL, pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// A declaration of szName as a psType, or the type alone without a name.
// Returns false, with nothing appended, if the type can't be written in
// the source.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::AppendDeclaration(SCPP65Type* psType, const char* szName, CChars* pszDest)
{
	CChars	szOperators;
	CChars	szCore;
	CChars	szDeclaration;
	bool	bResult;

	szOperators.Init();
	szCore.Init(szName ? szName : "");
	szDeclaration.Init();
	bResult = Declaration(psType, &szOperators, &szCore, &szDeclaration);
	if (bResult || (meWriting == CPPTW_Shown))
	{
		pszDest->Append(&szDeclaration);
	}
	szDeclaration.Kill();
	szCore.Kill();
	szOperators.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A class by its name, as a type of it is written.  Returns false, with
// nothing appended, if it can't be written.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::AppendClass(CCPPClass* pcClass, CChars* pszDest)
{
	CChars	szName;
	bool	bResult;

	szName.Init();
	bResult = ClassName(pcClass, &szName);
	if (bResult)
	{
		pszDest->Append(&szName);
	}
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A function's name with the classes and namespaces it is in, as C++
// writes it: "NS::CShape::Area", "CShape::~CShape", "CShape::operator =".
// An operator's (or a conversion's) name is its declaration's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::AppendFunction(CCPPFunction* pcFunction, CChars* pszDest)
{
	CCPPSyntaxName*	pcName;
	bool			bResult;

	if (pcFunction->mpcClass)
	{
		bResult = ClassName(pcFunction->mpcClass, pszDest);
		pszDest->Append("::");
	}
	else
	{
		bResult = ScopeName(pcFunction->mpcNamespace, pszDest);
	}
	if ((pcFunction->meKind != CPPFK_Function) && pcFunction->mpcClass)
	{
		if (pcFunction->meKind == CPPFK_Destructor)
		{
			pszDest->Append('~');
		}
		return bResult && AppendIdentifier(pcFunction->mpcClass->mlliName, pszDest);
	}
	if (pcFunction->mlliName >= 0)
	{
		return bResult && AppendIdentifier(pcFunction->mlliName, pszDest);
	}
	pcName = pcFunction->mpcDeclarator ? pcFunction->mpcDeclarator->GetName() : NULL;
	if (pcName && pcName->GetLast())
	{
		pcName->GetLast()->PrettyPrint(pszDest, 0);
		return bResult;
	}
	pszDest->Append("operator");
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's name with the classes and namespaces it is in: "giCount",
// "NS::giCount", "CShape::gsCount".
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::AppendVariable(CCPPVariable* pcVariable, CChars* pszDest)
{
	bool	bResult;

	if (pcVariable->mpcClass)
	{
		bResult = ClassName(pcVariable->mpcClass, pszDest);
		pszDest->Append("::");
	}
	else
	{
		bResult = ScopeName(pcVariable->mpcSymbol->mpcScope, pszDest);
	}
	return bResult && AppendIdentifier(pcVariable->mpcSymbol->mlliName, pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// An identifier's text.  False for a name the tokeniser hasn't.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::AppendIdentifier(int64 lliName, CChars* pszDest)
{
	char*	szName;
	int		iLength;

	iLength = 0;
	szName = mpcUnit->GetTokeniser()->GetIdentifier(lliName, &iLength);
	if (!szName)
	{
		return false;
	}
	pszDest->Append(szName, iLength);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A declaration is written from the name outwards.  pszCore is the name
// with what follows it (its array sizes and parameters), pszOperators the
// pointers and references before it, which are written against the type:
// "char* const* p".  A pointer to an array or a function has them in
// parentheses with the name, "(*pf)", before what follows.  The type's
// name is followed by a space before a member pointer's class, a name or
// a parenthesis, and by none before *, & or an array's size: "int A::*",
// "int (*)[3]", "int[3]".
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::Declaration(SCPP65Type* psType, CChars* pszOperators, CChars* pszCore, CChars* pszDest)
{
	CChars	szOperator;
	size	i;
	bool	bResult;

	switch (psType->eKind)
	{
	case CPP65TK_Pointer:
	case CPP65TK_Reference:
	case CPP65TK_MemberPointer:
		szOperator.Init();
		bResult = true;
		if (psType->eKind == CPP65TK_MemberPointer)
		{
			bResult = ClassName(psType->pcClass, &szOperator);
			szOperator.Append("::*");
		}
		else
		{
			szOperator.Append((psType->eKind == CPP65TK_Pointer) ? "*" : (psType->bRvalue ? "&&" : "&"));
		}
		if (psType->eKind != CPP65TK_Reference)
		{
			szOperator.Append(psType->bConst ? " const" : "");
			szOperator.Append(psType->bVolatile ? " volatile" : "");
		}
		if (StartsWithName(pszOperators))
		{
			//int* S::*: a member pointer's class after another operator.
			szOperator.Append(' ');
		}
		pszOperators->Insert(0, &szOperator);
		szOperator.Kill();
		return Declaration(psType->psBase, pszOperators, pszCore, pszDest) && bResult;

	case CPP65TK_Array:
		Parenthesise(pszOperators, pszCore);
		pszCore->Append('[');
		if ((meWriting == CPPTW_Shown) || (psType->iCount > 0))
		{
			pszCore->Append(psType->iCount);
		}
		pszCore->Append(']');
		return Declaration(psType->psBase, pszOperators, pszCore, pszDest);

	case CPP65TK_Function:
		Parenthesise(pszOperators, pszCore);
		pszCore->Append('(');
		bResult = true;
		for (i = 0; i < psType->papsParameters->NumElements(); i++)
		{
			pszCore->Append((i == 0) ? "" : ", ");
			bResult &= AppendDeclaration(psType->papsParameters->GetPtr(i), NULL, pszCore);
		}
		if (psType->bVariadic)
		{
			pszCore->Append((i == 0) ? "..." : ", ...");
		}
		else if ((i == 0) && (meWriting == CPPTW_Source))
		{
			pszCore->Append("void");
		}
		pszCore->Append(psType->FunctionEnd());
		return Declaration(psType->psBase, pszOperators, pszCore, pszDest) && bResult;

	default:
		Qualifiers(psType, pszDest);
		bResult = Named(psType, pszDest);
		if (StartsWithName(pszOperators))
		{
			pszDest->Append(' ');
		}
		pszDest->Append(pszOperators);
		if (!pszCore->Empty())
		{
			if (pszCore->GetChar(0) != '[')
			{
				pszDest->Append(' ');
			}
			pszDest->Append(pszCore);
		}
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the text starts with a name, which a space goes before.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::StartsWithName(CChars* psz)
{
	char	c;

	if (psz->Empty())
	{
		return false;
	}
	c = psz->GetChar(0);
	return (c == '_') || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}


//////////////////////////////////////////////////////////////////////////
//
// Before an array's size or a function's parameters: the pointers and
// references so far go in parentheses with the name.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTypeWriter::Parenthesise(CChars* pszOperators, CChars* pszCore)
{
	if (pszOperators->Empty())
	{
		return;
	}
	pszCore->Insert(0, pszOperators);
	pszCore->Insert(0, '(');
	pszCore->Append(')');
	pszOperators->Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTypeWriter::Qualifiers(SCPP65Type* psType, CChars* pszDest)
{
	pszDest->Append(psType->bConst ? "const " : "");
	pszDest->Append(psType->bVolatile ? "volatile " : "");
}


//////////////////////////////////////////////////////////////////////////
//
// A type that is one name: void, bool, an integer, a float, an enum or a
// class.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::Named(SCPP65Type* psType, CChars* pszDest)
{
	bool	bResult;

	switch (psType->eKind)
	{
	case CPP65TK_Void:
		pszDest->Append("void");
		return true;
	case CPP65TK_Bool:
		pszDest->Append("bool");
		return true;
	case CPP65TK_NullPointer:
		pszDest->Append("std::nullptr_t");
		return true;
	case CPP65TK_Integer:
		if (psType->pcEnum)
		{
			bResult = ScopeName(psType->pcEnum->mpcScope, pszDest);
			if (psType->pcEnum->mlliName < 0)
			{
				return Unnamed("(unnamed enum)", pszDest) && bResult;
			}
			return AppendIdentifier(psType->pcEnum->mlliName, pszDest) && bResult;
		}
		pszDest->Append(IntegerKindName(psType->eInteger));
		return true;
	case CPP65TK_Float:
		pszDest->Append(psType->bLongDouble ? "long double" : ((psType->iBytes == 4) ? "float" : "double"));
		return true;
	case CPP65TK_Class:
		return ClassName(psType->pcClass, pszDest);
	default:
		return Unnamed("?", pszDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What has no name: shown as szShown; in the source it can't be written.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::Unnamed(const char* szShown, CChars* pszDest)
{
	if (meWriting == CPPTW_Source)
	{
		return false;
	}
	pszDest->Append(szShown);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A class by its qualified name, and a class template's instance with
// its arguments: "NS::CArray<CFoo*, 4>".  A member class template's
// instance is named after the instance it is a member of:
// "P<char>::Q<long>".
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::ClassName(CCPPClass* pcClass, CChars* pszDest)
{
	bool	bResult;

	if (!pcClass)
	{
		return false;
	}
	if (pcClass->mbPlaceholder)
	{
		//A template's type parameter, as itself.
		return ParameterName(pcClass->mpcSymbol, pszDest);
	}
	if (pcClass->mpsLambda)
	{
		return Unnamed("<lambda>", pszDest);
	}
	if (pcClass->IsUnnamed())
	{
		return Unnamed("<unnamed>", pszDest);
	}
	if (!pcClass->mpcSymbol && (meWriting == CPPTW_Source))
	{
		return false;
	}
	bResult = true;
	if (pcClass->mpcTemplate && pcClass->mpcTemplate->mpcPrimary->mpcClass)
	{
		bResult = ClassName(pcClass->mpcTemplate->mpcPrimary->mpcClass, pszDest);
		pszDest->Append("::");
	}
	else if (pcClass->mpcSymbol)
	{
		bResult = ScopeName(pcClass->mpcSymbol->mpcScope, pszDest);
	}
	bResult &= AppendIdentifier(pcClass->mlliName, pszDest);
	return Arguments(pcClass, pszDest) && bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A class template's instance's arguments, what its template's parameters
// are bound to (for an instance of a partial specialisation, what the
// specialisation's are), in < and >.  Nothing for a class that isn't an
// instance, nor for one with no bindings: an explicit specialisation
// (template<> struct S<0>) is written S, wrongly, where its arguments are
// its pattern's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::Arguments(CCPPClass* pcClass, CChars* pszDest)
{
	CCPPTemplateArguments*	pcBindings;
	SCPPBinding*			psBinding;
	CCPPSymbol*				pcTemplate;
	size					i;
	size					j;
	bool					bResult;
	bool					bFirst;

	pcBindings = pcClass->mpcBindings;
	if (!pcClass->mpcTemplate || !pcBindings || (pcBindings->macBindings.NumElements() == 0))
	{
		return true;
	}
	bResult = true;
	bFirst = true;
	pszDest->Append('<');
	for (i = 0; i < pcBindings->macBindings.NumElements(); i++)
	{
		psBinding = pcBindings->macBindings.Get(i);
		if (psBinding->IsPack())
		{
			for (j = 0; j < psBinding->PackSize(); j++)
			{
				pszDest->Append(bFirst ? "" : ", ");
				bFirst = false;
				if (psBinding->papsPack)
				{
					bResult &= AppendDeclaration(psBinding->papsPack->GetPtr(j), NULL, pszDest);
				}
				else
				{
					pszDest->Append(psBinding->palliPack->GetValue(j));
				}
			}
			continue;
		}
		pszDest->Append(bFirst ? "" : ", ");
		bFirst = false;
		pcTemplate = (psBinding->pcTemplate && psBinding->pcTemplate->mpcPrimary) ? psBinding->pcTemplate->mpcPrimary->mpcSymbol : NULL;
		if (psBinding->psType)
		{
			bResult &= AppendDeclaration(psBinding->psType, NULL, pszDest);
		}
		else if (psBinding->bSelf)
		{
			//A value parameter as itself: its name.
			bResult &= ParameterName(psBinding->pcParameter, pszDest);
		}
		else if (psBinding->pcArgument && (meWriting == CPPTW_Source))
		{
			psBinding->pcArgument->PrettyPrint(pszDest, 0);
		}
		else if (pcTemplate)
		{
			bResult &= ScopeName(pcTemplate->mpcScope, pszDest);
			bResult &= (pcTemplate->mlliName >= 0) && AppendIdentifier(pcTemplate->mlliName, pszDest);
		}
		else
		{
			pszDest->Append(psBinding->lliValue);
		}
	}
	pszDest->Append('>');
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The named namespaces and classes pcScope is in, outermost first, each
// followed by ::.  A function's or a block's scope ends them: what is
// declared in a function is named as if in nothing.  A namespace without a
// name is shown as (anonymous namespace)::, and in the source what is in
// it is written as if in the one around it.  A class without a name has
// none to write: in the source that is false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::ScopeName(CCPPScope* pcScope, CChars* pszDest)
{
	bool	bResult;

	if (!pcScope || ((pcScope->meKind != CPPSC_Class) && (pcScope->meKind != CPPSC_Namespace)))
	{
		return true;
	}
	bResult = ScopeName(pcScope->mpcParent, pszDest);
	if (pcScope->mpcOwner && (pcScope->mpcOwner->mlliName >= 0))
	{
		bResult &= AppendIdentifier(pcScope->mpcOwner->mlliName, pszDest);
		pszDest->Append("::");
	}
	else if (pcScope->IsUnnamedNamespace())
	{
		if (meWriting == CPPTW_Shown)
		{
			pszDest->Append("(anonymous namespace)::");
		}
	}
	else if (pcScope->mpcOwner && (meWriting == CPPTW_Source))
	{
		bResult = false;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A template parameter's name, which nothing qualifies.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::ParameterName(CCPPSymbol* pcSymbol, CChars* pszDest)
{
	if (!pcSymbol || (pcSymbol->mlliName < 0))
	{
		return false;
	}
	return AppendIdentifier(pcSymbol->mlliName, pszDest);
}

