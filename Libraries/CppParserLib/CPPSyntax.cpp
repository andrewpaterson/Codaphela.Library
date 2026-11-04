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
#include "BaseLib/SystemAllocator.h"
#include "CPPSyntaxes.h"


//The name of each kind of node, indexed by ECPPSyntax, for TypePrint.
char* gaszSyntaxKindNames[] =
{
	"TranslationUnit",
	"Namespace",
	"LinkageSpecification",
	"SimpleDeclaration",
	"FunctionDefinition",
	"InitDeclarator",
	"TemplateDeclaration",
	"TypeParameter",
	"Parameter",
	"UsingDirective",
	"UsingDeclaration",
	"AliasDeclaration",
	"StaticAssert",
	"ExtensionDeclaration",
	"EmptyDeclaration",
	"AccessSpecifier",
	"SkippedDeclaration",
	"Enumerator",
	"BaseSpecifier",
	"MemberInitializer",
	"Attribute",
	"DeclSpecifiers",
	"KeywordSpecifier",
	"AlignasSpecifier",
	"NamedTypeSpecifier",
	"ClassSpecifier",
	"EnumSpecifier",
	"ElaboratedTypeSpecifier",
	"DecltypeSpecifier",
	"Declarator",
	"PointerOperator",
	"FunctionSuffix",
	"ArraySuffix",
	"ExceptionSpecification",
	"TypeId",
	"Name",
	"NameComponent",
	"CompoundStatement",
	"DeclarationStatement",
	"ExpressionStatement",
	"IfStatement",
	"SwitchStatement",
	"CaseStatement",
	"LabelStatement",
	"AttributedStatement",
	"WhileStatement",
	"DoStatement",
	"ForStatement",
	"RangeForStatement",
	"BreakStatement",
	"ContinueStatement",
	"ReturnStatement",
	"GotoStatement",
	"TryStatement",
	"CatchHandler",
	"SkippedStatement",
	"LiteralExpression",
	"StringExpression",
	"IdExpression",
	"ThisExpression",
	"ParenthesisedExpression",
	"UnaryExpression",
	"BinaryExpression",
	"ConditionalExpression",
	"CallExpression",
	"SubscriptExpression",
	"MemberExpression",
	"CastExpression",
	"ConstructExpression",
	"NamedCastExpression",
	"SizeofExpression",
	"TypeidExpression",
	"NewExpression",
	"DeleteExpression",
	"ThrowExpression",
	"InitializerList",
	"ParenthesisedInitializer",
	"TypeTraitExpression",
	"PackExpansionExpression",
	"NoexceptExpression",
	"LazyDeclaration",
};
static_assert(sizeof(gaszSyntaxKindNames) / sizeof(char*) == CPPS_LazyDeclaration + 1, "gaszSyntaxKindNames must have a name for every ECPPSyntax.");


//////////////////////////////////////////////////////////////////////////
//
// What a node's own arrays are made with: its tree's (an arena when the
// tree is given one, CCPPSyntaxTree::Init).  Init the node first.
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CCPPSyntax::NodeArrayMallocator(void)
{
	if (mpcTree)
	{
		return mpcTree->GetNodeArrayMallocator();
	}
	return &gcSystemAllocator;
}


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
char* CCPPSyntax::GetType(void)
{
	//The kind's name.  One table instead of an override in every node class.
	return gaszSyntaxKindNames[GetKind()];
}


//Counts a node's children (CCPPSyntax::NumChildren): one for each Child field (even if NULL), and the nodes in each
//Children field.
class CCPPSyntaxChildCounter : public CCPPSyntaxFields
{
public:
	size	muiChildren;

	void	NodeField(CCPPSyntax** ppcNode) override		{ muiChildren++; }
	void	NodesField(CArraySyntax* papcNodes) override	{ muiChildren += papcNodes->NumElements(); }
};


//Finds a node's child by index (CCPPSyntax::GetChild), counting as CCPPSyntaxChildCounter does.
class CCPPSyntaxChildFinder : public CCPPSyntaxFields
{
public:
	size		muiIndex;		//The children still to pass.
	bool		mbFound;
	CCPPSyntax*	mpcChild;

	void	NodeField(CCPPSyntax** ppcNode) override
	{
		if (!mbFound)
		{
			if (muiIndex == 0)
			{
				mpcChild = *ppcNode;
				mbFound = true;
			}
			else
			{
				muiIndex--;
			}
		}
	}

	void	NodesField(CArraySyntax* papcNodes) override
	{
		if (!mbFound)
		{
			if (muiIndex < papcNodes->NumElements())
			{
				mpcChild = papcNodes->GetPtr(muiIndex);
				mbFound = true;
			}
			else
			{
				muiIndex -= papcNodes->NumElements();
			}
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Gives the node's fields to pcFields, in the order they are declared.
// A node with no fields of its own gives none.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntax::VisitFields(CCPPSyntaxFields* pcFields)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The node's children, from its fields: each Child field is one (even if
// it is NULL), and each Children field is as many as it holds.
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntax::NumChildren(void)
{
	CCPPSyntaxChildCounter	cCounter;

	cCounter.muiChildren = 0;
	VisitFields(&cCounter);
	return cCounter.muiChildren;
}


//////////////////////////////////////////////////////////////////////////
//
// The node's iIndex'th child (NULL if its field is NULL, or there are
// fewer children).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntax::GetChild(size iIndex)
{
	CCPPSyntaxChildFinder	cFinder;

	cFinder.muiIndex = iIndex;
	cFinder.mbFound = false;
	cFinder.mpcChild = NULL;
	VisitFields(&cFinder);
	return cFinder.mpcChild;
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

