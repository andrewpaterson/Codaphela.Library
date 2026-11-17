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
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLambdaExpression::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxExpression::Init(pcTree, pcParent);
	meDefault = CPPLC_None;
	macpCaptures.Init();
	mpcDefinition = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLambdaExpression::Kill(void)
{
	macpCaptures.Kill();
	CCPPSyntaxExpression::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLambdaExpression::PrettyPrint(CChars* pszDest, int iDepth)
{
	size	i;

	pszDest->Append('[');
	if (meDefault != CPPLC_None)
	{
		pszDest->Append((meDefault == CPPLC_Reference) ? '&' : '=');
	}
	for (i = 0; i < macpCaptures.NumElements(); i++)
	{
		if ((i > 0) || (meDefault != CPPLC_None))
		{
			pszDest->Append(", ");
		}
		PrintNode(pszDest, macpCaptures.GetPtr(i), iDepth);
	}
	pszDest->Append(']');
	if (mpcDefinition)
	{
		PrintNode(pszDest, mpcDefinition->mpcDeclarator, iDepth);
		pszDest->Append('\n');
		PrintNode(pszDest, mpcDefinition->mpcBody, iDepth);

		//The body is a statement, which ends its line: what follows the lambda goes after its }.
		if (pszDest->EndsWith('\n'))
		{
			pszDest->RemoveLastCharacter();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxLambdaExpression::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Enum(&meDefault);
	pcFields->Children(&macpCaptures);
	pcFields->Child(&mpcDefinition);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the lambda was declared mutable: its copies of what it captures
// can be changed, and its operator() is not const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxLambdaExpression::IsMutable(void) const
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntax*					pcQualifier;
	size						i;

	pcSuffix = (mpcDefinition && mpcDefinition->mpcDeclarator) ? mpcDefinition->mpcDeclarator->GetFunctionSuffix() : NULL;
	for (i = 0; pcSuffix && (i < pcSuffix->macpQualifiers.NumElements()); i++)
	{
		pcQualifier = pcSuffix->macpQualifiers.GetPtr(i);
		if ((pcQualifier->GetKind() == CPPS_KeywordSpecifier) && (((CCPPSyntaxKeywordSpecifier*)pcQualifier)->meKeyword == CT_mutable))
		{
			return true;
		}
	}
	return false;
}
