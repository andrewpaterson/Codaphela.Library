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
#include "BaseLib/ArrayInt.h"
#include "CPPScope.h"
#include "SourceFile.h"
#include "CPPSyntaxDeclarator.h"
#include "CPPSyntaxFunctionSuffix.h"
#include "CPPSyntaxParameter.h"
#include "CPPSyntaxName.h"
#include "CPPSyntaxNameComponent.h"
#include "CPPSyntaxKind.h"
#include "CPPSymbolKey.h"


//////////////////////////////////////////////////////////////////////////
//
// A pointer, reference, array or function part of a declarator.
//
//////////////////////////////////////////////////////////////////////////
bool IsDeclaratorToken(CCPPToken* pcToken)
{
	return pcToken->Is(CT_Multiply) || pcToken->Is(CT_BitwiseAnd) || pcToken->Is(CT_LogicalAnd) || pcToken->Is(CT_OpenSquareBracket) || pcToken->Is(CT_OpenRoundBracket);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolKey::Init(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree)
{
	mpcTokeniser = pcTokeniser;
	mpcTree = pcTree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolKey::Kill(void)
{
	mpcTokeniser = NULL;
	mpcTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol's key appended to pszDest.  A function's is from its first
// declaration; GetFunctionKey gives the key of the overload a particular
// declarator declares.  False for a symbol without a name or a
// declaration to key it by.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbolKey::GetKey(CCPPSymbol* pcSymbol, CChars* pszDest)
{
	SPreprocessedToken*	psSource;
	CCPPSyntax*			pcDeclaration;

	if ((pcSymbol == NULL) || (pcSymbol->mlliName == -1))
	{
		return false;
	}

	if (IsFunction(pcSymbol))
	{
		return GetFunctionKey(pcSymbol, GetDeclarator(pcSymbol), pszDest);
	}

	if (IsLocal(pcSymbol))
	{
		//A local is only the same in two units when it is in the same place, as in an inline function in a header.
		pcDeclaration = pcSymbol->mpcDeclaration;
		if ((pcDeclaration == NULL) || (pcDeclaration->miFirstToken < 0))
		{
			return false;
		}
		psSource = mpcTokeniser->GetSource(mpcTokeniser->Get((size)pcDeclaration->miFirstToken));
		pszDest->Append('@');
		pszDest->Append(psSource->pcFile ? psSource->pcFile->FullName() : "");
		pszDest->Append(':');
		pszDest->Append(psSource->iLine + 1);
		pszDest->Append(':');
		pszDest->Append(psSource->iColumn + 1);
		pszDest->Append(':');
		AppendName(pcSymbol, pszDest);
		return true;
	}

	GetQualifiedName(pcSymbol, pszDest);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's qualified name and the parameter types of the overload
// pcDeclarator declares.  Without a declarator (a function the parser
// only knows the name of) the key is the name with no signature.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbolKey::GetFunctionKey(CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator, CChars* pszDest)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;

	if ((pcSymbol == NULL) || (pcSymbol->mlliName == -1))
	{
		return false;
	}
	GetQualifiedName(pcSymbol, pszDest);
	pcSuffix = pcDeclarator ? pcDeclarator->GetFunctionSuffix() : NULL;
	if (pcSuffix)
	{
		GetSignature(pcSuffix, pszDest);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The names of the namespaces and classes the symbol is in, then its
// own: ::-separated, without a leading ::.  An unnamed namespace is
// (anonymous).
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolKey::GetQualifiedName(CCPPSymbol* pcSymbol, CChars* pszDest)
{
	CCPPScope*	pcScope;

	pcScope = pcSymbol->mpcScope;
	while (pcScope && (pcScope->mpcOwner == NULL) && (pcScope->meKind != CPPSC_Namespace))
	{
		pcScope = pcScope->mpcParent;
	}
	if (pcScope && pcScope->mpcOwner)
	{
		GetQualifiedName(pcScope->mpcOwner, pszDest);
		pszDest->Append("::");
	}
	else if (pcScope && pcScope->mpcParent)
	{
		pszDest->Append("(anonymous)::");
	}
	AppendName(pcSymbol, pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// (int, char const*) const: the parameter types without their names or
// default arguments, then the function's qualifiers.  (void) is ().
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolKey::GetSignature(CCPPSyntaxFunctionSuffix* pcSuffix, CChars* pszDest)
{
	size		i;
	size		uiStart;
	CCPPSyntax*	pcQualifier;

	pszDest->Append('(');
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		if (i > 0)
		{
			pszDest->Append(", ");
		}
		uiStart = pszDest->Length();
		AppendParameter((CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i), pszDest);
		if ((pcSuffix->macpParameters.NumElements() == 1) && pszDest->SubStringEquals(uiStart, "void") && (pszDest->Length() == uiStart + 4))
		{
			pszDest->SetLength(uiStart);
		}
	}
	if (pcSuffix->mbVariadic)
	{
		if (pcSuffix->macpParameters.NumElements() > 0)
		{
			pszDest->Append(", ");
		}
		pszDest->Append("...");
	}
	pszDest->Append(')');

	for (i = 0; i < pcSuffix->macpQualifiers.NumElements(); i++)
	{
		pcQualifier = pcSuffix->macpQualifiers.GetPtr(i);
		pszDest->Append(' ');
		AppendToken(pcQualifier->miFirstToken, pszDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter's type without its name or default argument.  Type names
// are spelled qualified and const and volatile come after the type they
// qualify (const char* is char const*).  Top level const and volatile
// don't change the function's type so they are left out.  [dcl.fct] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolKey::AppendParameter(CCPPSyntaxParameter* pcParameter, CChars* pszDest)
{
	int				iToken;
	int				iLast;
	int				iName;
	int				iDeclarator;
	int				iLastPointer;
	CCPPSyntaxName*	pcName;
	CCPPToken*		pcToken;
	bool			bTopLevel;
	CArrayInt		aiLeading;
	size			i;

	iName = -1;
	if (pcParameter->mpcDeclarator)
	{
		pcName = pcParameter->mpcDeclarator->GetName();
		if (pcName && pcName->GetLast())
		{
			iName = pcName->GetLast()->miName;
		}
	}

	iLast = pcParameter->miLastToken;
	if (pcParameter->mpcDefault)
	{
		//The '=' before the default argument.
		iLast = pcParameter->mpcDefault->miFirstToken - 2;
	}

	//The declarator's pointers, references and arrays start where the type ends.  A const before them is part of the
	//type pointed to; one after the last pointer qualifies the parameter itself, so it is top level, as is a const of a
	//parameter with no declarator.
	iDeclarator = iLast + 1;
	iLastPointer = -1;
	for (iToken = pcParameter->miFirstToken; iToken <= iLast; iToken++)
	{
		pcToken = mpcTokeniser->Get((size)iToken);
		if (IsDeclaratorToken(pcToken))
		{
			if (iDeclarator > iLast)
			{
				iDeclarator = iToken;
			}
			if (pcToken->Is(CT_Multiply))
			{
				iLastPointer = iToken;
			}
		}
	}
	bTopLevel = iDeclarator > iLast;

	aiLeading.Init();
	for (iToken = pcParameter->miFirstToken; iToken <= iLast; iToken++)
	{
		pcToken = mpcTokeniser->Get((size)iToken);
		if (iToken == iName)
		{
			continue;
		}
		if (pcToken->Is(CT_const) || pcToken->Is(CT_volatile))
		{
			if (bTopLevel || ((iLastPointer >= 0) && (iToken > iLastPointer)))
			{
				continue;
			}
			if (iToken < iDeclarator)
			{
				aiLeading.Add(iToken);
				continue;
			}
		}
		if (iToken == iDeclarator)
		{
			for (i = 0; i < aiLeading.NumElements(); i++)
			{
				AppendSpaced(aiLeading.GetValue(i), pszDest);
			}
			aiLeading.ReInit();
		}
		AppendSpaced(iToken, pszDest);
	}
	aiLeading.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A token's text after a space if it would run into the text before it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolKey::AppendSpaced(int iToken, CChars* pszDest)
{
	CChars	szToken;
	char	cLast;

	szToken.Init();
	AppendToken(iToken, &szToken);
	if (!szToken.Empty())
	{
		cLast = pszDest->Empty() ? ' ' : pszDest->GetChar(pszDest->Length() - 1);
		if (IsCPPNameCharacter(cLast) && IsCPPNameCharacter(szToken.GetChar(0)))
		{
			pszDest->Append(' ');
		}
		pszDest->Append(szToken);
	}
	szToken.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A token's text, or a type's qualified name if the token names one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolKey::AppendToken(int iToken, CChars* pszDest)
{
	SCPPTokenSymbol*	psSymbol;
	CCPPToken*			pcToken;

	if (iToken < 0)
	{
		return;
	}
	psSymbol = mpcTree->GetTokenSymbol(iToken);
	if (psSymbol && psSymbol->pcSymbol && psSymbol->pcSymbol->IsType() && !IsLocal(psSymbol->pcSymbol))
	{
		GetQualifiedName(psSymbol->pcSymbol, pszDest);
		return;
	}
	pcToken = mpcTokeniser->Get((size)iToken);
	pcToken->Append(pszDest, mpcTokeniser);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbolKey::AppendName(CCPPSymbol* pcSymbol, CChars* pszDest)
{
	char*	szName;
	int		iLength;

	iLength = 0;
	szName = mpcTokeniser->GetIdentifier(pcSymbol->mlliName, &iLength);
	if (szName)
	{
		pszDest->Append(szName, iLength);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The declarator that first declared the function, NULL if the parser
// didn't keep one (a builtin, or a declaration it only skimmed).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclarator* CCPPSymbolKey::GetDeclarator(CCPPSymbol* pcSymbol)
{
	if ((pcSymbol == NULL) || (pcSymbol->mpcDeclaration == NULL))
	{
		return NULL;
	}
	return FindDeclarator(pcSymbol->mpcDeclaration, pcSymbol);
}


//Looks for a symbol's declarator in each of a node's children until it is found (CCPPSymbolKey::FindDeclarator).
class CCPPSymbolKeyDeclaratorFinder : public CCPPSyntaxChildVisitor
{
public:
	CCPPSymbolKey*			mpcKeys;
	CCPPSymbol*				mpcSymbol;
	CCPPSyntaxDeclarator*	mpcFound;

	void	Visit(CCPPSyntax* pcChild) override
	{
		if (mpcFound == NULL)
		{
			mpcFound = mpcKeys->FindDeclarator(pcChild, mpcSymbol);
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The declarator under pcNode whose name declares pcSymbol.  Function
// bodies aren't searched: what they declare is local.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclarator* CCPPSymbolKey::FindDeclarator(CCPPSyntax* pcNode, CCPPSymbol* pcSymbol)
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPSyntaxName*			pcName;
	SCPPTokenSymbol*		psSymbol;
	CCPPSymbolKeyDeclaratorFinder	cChildren;

	if (pcNode->GetKind() == CPPS_Declarator)
	{
		pcDeclarator = (CCPPSyntaxDeclarator*)pcNode;
		pcName = pcDeclarator->GetName();
		if (pcName && pcName->GetLast())
		{
			psSymbol = mpcTree->GetTokenSymbol(pcName->GetLast()->miName);
			if ((pcName->mpcSymbol == pcSymbol) || (psSymbol && (psSymbol->pcSymbol == pcSymbol)))
			{
				return pcDeclarator;
			}
		}
	}
	if ((pcNode->GetKind() == CPPS_CompoundStatement) || (pcNode->GetKind() == CPPS_FunctionSuffix))
	{
		return NULL;
	}

	cChildren.mpcKeys = this;
	cChildren.mpcSymbol = pcSymbol;
	cChildren.mpcFound = NULL;
	pcNode->VisitFields(&cChildren);
	return cChildren.mpcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter, a template parameter, a label or a name declared in a
// function body: only the unit it is in can see it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbolKey::IsLocal(CCPPSymbol* pcSymbol)
{
	ECPPScopeKind	eKind;

	if ((pcSymbol->meKind == CPPSK_Parameter) || (pcSymbol->meKind == CPPSK_TypeParameter) || (pcSymbol->meKind == CPPSK_ValueParameter) || (pcSymbol->meKind == CPPSK_Label))
	{
		return true;
	}
	if (pcSymbol->mpcScope == NULL)
	{
		return false;
	}
	eKind = pcSymbol->mpcScope->meKind;
	return (eKind == CPPSC_Function) || (eKind == CPPSC_Block) || (eKind == CPPSC_Template);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbolKey::IsFunction(CCPPSymbol* pcSymbol)
{
	return (pcSymbol->meKind == CPPSK_Function) || (pcSymbol->meKind == CPPSK_FunctionTemplate);
}


//////////////////////////////////////////////////////////////////////////
//
// How many arguments a call to the function's first declaration can
// pass: the parameters without defaults up to them all, or any number
// past them for a variadic function (iMaximum -1).  False if the
// parser didn't keep its declarator.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbolKey::GetParameterCounts(CCPPSymbol* pcSymbol, int* piMinimum, int* piMaximum)
{
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxFunctionSuffix*	pcSuffix;

	pcDeclarator = GetDeclarator(pcSymbol);
	pcSuffix = pcDeclarator ? pcDeclarator->GetFunctionSuffix() : NULL;
	if (pcSuffix == NULL)
	{
		return false;
	}

	*piMinimum = (int)pcSuffix->RequiredParameters();
	*piMaximum = pcSuffix->mbVariadic ? -1 : (int)pcSuffix->NumParameters();
	return true;
}

