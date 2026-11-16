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
void CCPPTypeWriter::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
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
// Returns false, with nothing appended, if the type can't be written.
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
	if (bResult)
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
// A declaration is written from the name outwards.  pszCore is the name
// with what follows it (its array sizes and parameters), pszOperators the
// pointers and references before it, which are written against the type:
// "char* const* p".  A pointer to an array or a function has them in
// parentheses with the name, "(*pf)", before what follows.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::Declaration(SCPP65Type* psType, CChars* pszOperators, CChars* pszCore, CChars* pszDest)
{
	CChars	szOperator;
	CChars	szParameter;
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
			szOperator.Append(' ');
			bResult = ClassName(psType->pcClass, &szOperator);
			szOperator.Append("::*");
		}
		else
		{
			szOperator.Append((psType->eKind == CPP65TK_Pointer) ? "*" : (psType->bRvalue ? "&&" : "&"));
		}
		if (psType->bConst)
		{
			szOperator.Append(" const");
		}
		if (psType->bVolatile)
		{
			szOperator.Append(" volatile");
		}
		pszOperators->Insert(0, &szOperator);
		szOperator.Kill();
		return bResult && Declaration(psType->psBase, pszOperators, pszCore, pszDest);

	case CPP65TK_Array:
		Parenthesise(pszOperators, pszCore);
		pszCore->Append('[');
		if (psType->iCount > 0)
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
			if (i != 0)
			{
				pszCore->Append(", ");
			}
			szParameter.Init();
			bResult &= AppendDeclaration(psType->papsParameters->GetPtr(i), NULL, &szParameter);
			pszCore->Append(&szParameter);
			szParameter.Kill();
		}
		if (psType->bVariadic)
		{
			pszCore->Append((psType->papsParameters->NumElements() != 0) ? ", ..." : "...");
		}
		else if (psType->papsParameters->NumElements() == 0)
		{
			pszCore->Append("void");
		}
		pszCore->Append(')');
		return bResult && Declaration(psType->psBase, pszOperators, pszCore, pszDest);

	default:
		Qualifiers(psType, pszDest);
		if (!Named(psType, pszDest))
		{
			return false;
		}
		pszDest->Append(pszOperators);
		if (!pszCore->Empty())
		{
			pszDest->Append(' ');
			pszDest->Append(pszCore);
		}
		return true;
	}
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
	if (psType->bConst)
	{
		pszDest->Append("const ");
	}
	if (psType->bVolatile)
	{
		pszDest->Append("volatile ");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A type that is one name: void, bool, an integer, a float, an enum or a
// class.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::Named(SCPP65Type* psType, CChars* pszDest)
{
	switch (psType->eKind)
	{
	case CPP65TK_Void:
		pszDest->Append("void");
		return true;
	case CPP65TK_Bool:
		pszDest->Append("bool");
		return true;
	case CPP65TK_Integer:
		if (psType->pcEnum)
		{
			return QualifiedName(psType->pcEnum, pszDest);
		}
		if (!psType->bSigned)
		{
			pszDest->Append("unsigned ");
		}
		if (psType->iBytes == 1)
		{
			pszDest->Append("char");
		}
		else if (psType->iBytes == mpcUnit->GetTypes()->msInt.iBytes)
		{
			pszDest->Append("int");
		}
		else if (psType->iBytes == 2)
		{
			pszDest->Append("short");
		}
		else if (psType->iBytes == 4)
		{
			pszDest->Append("long");
		}
		else
		{
			pszDest->Append("long long");
		}
		return true;
	case CPP65TK_Float:
		pszDest->Append(psType->bLongDouble ? "long double" : ((psType->iBytes == 4) ? "float" : "double"));
		return true;
	case CPP65TK_Class:
		return ClassName(psType->pcClass, pszDest);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A class by its qualified name, and a class template's instance with
// its arguments: "NS::CArray<CFoo*, 4>".
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::ClassName(CCPPClass* pcClass, CChars* pszDest)
{
	CCPPTemplateArguments*	pcBindings;
	SCPPBinding*			psBinding;
	size					i;
	size					j;
	bool					bResult;
	bool					bFirst;

	if (pcClass && pcClass->mbPlaceholder)
	{
		//A template's type parameter, as itself.
		return ParameterName(pcClass->mpcSymbol, pszDest);
	}
	if (!pcClass || !pcClass->mpcSymbol || pcClass->IsUnnamed() || !QualifiedName(pcClass->mpcSymbol, pszDest))
	{
		return false;
	}
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
		if (psBinding->psType)
		{
			bResult &= AppendDeclaration(psBinding->psType, NULL, pszDest);
		}
		else if (psBinding->bSelf)
		{
			//A value parameter as itself: its name.
			bResult &= ParameterName(psBinding->pcParameter, pszDest);
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
// A template parameter's name, which nothing qualifies.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::ParameterName(CCPPSymbol* pcSymbol, CChars* pszDest)
{
	char*	szName;
	int		iLength;

	if (!pcSymbol || (pcSymbol->mlliName < 0))
	{
		return false;
	}
	iLength = 0;
	szName = mpcUnit->GetTokeniser()->GetIdentifier(pcSymbol->mlliName, &iLength);
	if (!szName)
	{
		return false;
	}
	pszDest->Append(szName, iLength);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol's name after the names of the namespaces and classes it is
// in.  A namespace without a name has none to write: what is in it is
// written as if in the one around it.  False for a symbol with no name,
// or in a class or enum with none.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTypeWriter::QualifiedName(CCPPSymbol* pcSymbol, CChars* pszDest)
{
	CCPPScope*	pcScope;
	char*		szName;
	int			iLength;

	if (!pcSymbol || (pcSymbol->mlliName < 0))
	{
		return false;
	}
	pcScope = pcSymbol->mpcScope;
	while (pcScope && !pcScope->mpcOwner)
	{
		pcScope = pcScope->mpcParent;
	}
	if (pcScope)
	{
		if (!QualifiedName(pcScope->mpcOwner, pszDest))
		{
			return false;
		}
		pszDest->Append("::");
	}

	iLength = 0;
	szName = mpcUnit->GetTokeniser()->GetIdentifier(pcSymbol->mlliName, &iLength);
	if (!szName)
	{
		return false;
	}
	pszDest->Append(szName, iLength);
	return true;
}
