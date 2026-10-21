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
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	mpcTree = pcTree;
	mpcParent = pcParent;
	miFirstToken = -1;
	miLastToken = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::Kill(void)
{
	mpcTree = NULL;
	mpcParent = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::TypePrint(CChars* pszDest, int iDepth)
{
	size		i;
	size		uiNumChildren;
	CCPPSyntax*	pcChild;

	pszDest->Append(' ', iDepth * 2);
	pszDest->Append(GetType());
	pszDest->Append(" [");
	pszDest->Append(miFirstToken);
	pszDest->Append(", ");
	pszDest->Append(miLastToken);
	pszDest->Append("]\n");

	uiNumChildren = NumChildren();
	for (i = 0; i < uiNumChildren; i++)
	{
		pcChild = GetChild(i);
		if (pcChild)
		{
			pcChild->TypePrint(pszDest, iDepth + 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntax::IsDeclaration(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntax::IsStatement(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntax::IsExpression(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntax::IsTypeSpecifier(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::SetTokens(int iFirstToken, int iLastToken)
{
	miFirstToken = iFirstToken;
	miLastToken = iLastToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::Dump(void)
{
	CChars	sz;

	sz.Init();
	PrettyPrint(&sz, 0);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::Indent(CChars* pszDest, int iDepth)
{
	pszDest->Append('\t', iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::AppendToken(CChars* pszDest, int iToken)
{
	CCPPTokeniser*	pcTokeniser;
	CCPPToken*		pcToken;

	pcTokeniser = mpcTree->GetTokeniser();
	pcToken = pcTokeniser->Get(iToken);
	if (pcToken)
	{
		pcToken->Append(pszDest, pcTokeniser);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::AppendTokens(CChars* pszDest, int iFirstToken, int iLastToken)
{
	CCPPTokeniser*		pcTokeniser;
	CCPPToken*			pcToken;
	SPreprocessedToken*	psSource;
	SPreprocessedToken*	psPrevious;
	int					i;

	//The tokens with a space where the source had space, or where they would run together.
	pcTokeniser = mpcTree->GetTokeniser();
	psPrevious = NULL;
	for (i = iFirstToken; i <= iLastToken; i++)
	{
		pcToken = pcTokeniser->Get(i);
		if (!pcToken)
		{
			break;
		}
		psSource = pcTokeniser->GetSource(pcToken);
		if (psPrevious && (psSource->HasLeadingSpace() || psSource->IsStartOfLine() || PreprocessedTokensNeedSpace(psPrevious, psSource)))
		{
			pszDest->Append(' ');
		}
		pcToken->Append(pszDest, pcTokeniser);
		psPrevious = psSource;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::PrintKeyword(CChars* pszDest, ECppTokens eToken)
{
	pszDest->Append(mpcTree->GetTokeniser()->GetReservedWord(eToken));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::PrintNode(CChars* pszDest, CCPPSyntax* pcNode, int iDepth)
{
	if (pcNode)
	{
		pcNode->PrettyPrint(pszDest, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::PrintList(CChars* pszDest, CArraySyntax* papcNodes, char* szSeparator, int iDepth)
{
	size	i;

	for (i = 0; i < papcNodes->NumElements(); i++)
	{
		if (i > 0)
		{
			pszDest->Append(szSeparator);
		}
		papcNodes->GetPtr(i)->PrettyPrint(pszDest, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::PrintDeclarations(CChars* pszDest, CArraySyntax* papcNodes, int iDepth)
{
	size	i;

	for (i = 0; i < papcNodes->NumElements(); i++)
	{
		papcNodes->GetPtr(i)->PrettyPrint(pszDest, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::PrintStatements(CChars* pszDest, CArraySyntax* papcNodes, int iDepth)
{
	PrintDeclarations(pszDest, papcNodes, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::PrintCondition(CChars* pszDest, CCPPSyntax* pcCondition, int iDepth)
{
	//A condition is an expression or a declaration, as in if (int x = f()).  [stmt.select]
	PrintNode(pszDest, pcCondition, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::PrintSubStatement(CChars* pszDest, CCPPSyntax* pcStatement, int iDepth)
{
	if (!pcStatement)
	{
		return;
	}
	if (pcStatement->GetKind() == CPPS_CompoundStatement)
	{
		pcStatement->PrettyPrint(pszDest, iDepth);
	}
	else
	{
		pcStatement->PrettyPrint(pszDest, iDepth + 1);
	}
}

