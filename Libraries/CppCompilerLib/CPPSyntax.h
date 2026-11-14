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
#ifndef __C_P_P_SYNTAX_H__
#define __C_P_P_SYNTAX_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "CppTokens.h"
#include "CPPSyntaxKind.h"
#include "CPPSyntaxEnums.h"


class CCPPSyntax;
class CCPPSyntaxTree;
class CCPPSyntaxVisitor;
class CCPPScope;
class CCPPSymbol;


typedef CArrayTemplatePtr<CCPPSyntax>	CArraySyntax;


//What a node's fields are given to, one at a time in the order they are declared (CCPPSyntax::VisitFields): each kind
//of field has its own function, which does nothing unless overridden.  A node's children are its Child and Children
//fields; a Link is a node that isn't its child.  A Scope is one the node made (its members, parameters or block's);
//a UsedScope one it refers to (the scope it is in, or another node's).  Tokens are indices into the tree's tokeniser.
//Its tree, parent and tokens (CCPPSyntax's own) aren't given.
class CCPPSyntaxFields
{
public:
	virtual void			NodeField(CCPPSyntax** ppcNode) {}
	virtual void			NodesField(CArraySyntax* papcNodes) {}
	virtual void			LinkField(CCPPSyntax** ppcNode) {}
	virtual void			SymbolField(CCPPSymbol** ppcSymbol) {}
	virtual void			ScopeField(CCPPScope** ppcScope, bool bMade) {}
	virtual void			TokenField(int32* piToken) {}
	virtual void			ValueField(void* pvValue, size uiSize) {}

	template<class M> void	Child(M** ppcNode)					{ NodeField((CCPPSyntax**)ppcNode); }
	void					Children(CArraySyntax* papcNodes)	{ NodesField(papcNodes); }
	template<class M> void	Link(M** ppcNode)					{ LinkField((CCPPSyntax**)ppcNode); }
	void					Symbol(CCPPSymbol** ppcSymbol)		{ SymbolField(ppcSymbol); }
	void					Scope(CCPPScope** ppcScope)			{ ScopeField(ppcScope, true); }
	void					UsedScope(CCPPScope** ppcScope)		{ ScopeField(ppcScope, false); }
	void					Token(int32* piToken)				{ TokenField(piToken); }
	void					Bool(bool* pbValue)					{ ValueField(pbValue, sizeof(bool)); }
	template<class E> void	Enum(E* peValue)					{ ValueField(peValue, sizeof(E)); }
	template<class I> void	Integer(I* piValue)					{ ValueField(piValue, sizeof(I)); }
};


//Given each of a node's children that isn't NULL, in order (pcNode->VisitFields(this)): a walk over every child visits
//the fields once, where GetChild(i) for each child visits them from the start each time.
class CCPPSyntaxChildVisitor : public CCPPSyntaxFields
{
public:
	virtual void	Visit(CCPPSyntax* pcChild) =0;

	void	NodeField(CCPPSyntax** ppcNode) override
	{
		if (*ppcNode)
		{
			Visit(*ppcNode);
		}
	}
	void	NodesField(CArraySyntax* papcNodes) override
	{
		size		i;
		CCPPSyntax*	pcChild;

		for (i = 0; i < papcNodes->NumElements(); i++)
		{
			pcChild = papcNodes->GetPtr(i);
			if (pcChild)
			{
				Visit(pcChild);
			}
		}
	}
};


//A node of the abstract syntax tree.  Nodes know the tokens they were parsed from, so they can be found in the source.
//Each kind of node gives its fields to a CCPPSyntaxFields (VisitFields); its children are found from them.
class CCPPSyntax
{
public:
	CCPPSyntaxTree*		mpcTree;
	CCPPSyntax*			mpcParent;
	int32				miFirstToken;
	int32				miLastToken;		//Inclusive.

	virtual void		Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent);
	virtual void		Kill(void);

	virtual ECPPSyntax	GetKind(void) =0;
	virtual void		Accept(CCPPSyntaxVisitor* pcVisitor) =0;
	virtual CCPPSyntax*	WithoutParentheses(void) { return this; }
	virtual void		PrettyPrint(CChars* pszDest, int iDepth) =0;
	virtual void		TypePrint(CChars* pszDest, int iDepth);
	virtual void		VisitFields(CCPPSyntaxFields* pcFields);
			size		NumChildren(void);
			CCPPSyntax*	GetChild(size iIndex);

			char*		GetType(void);
			void		SetTokens(int iFirstToken, int iLastToken);
			void		Dump(void);

protected:
			CMallocator*	NodeArrayMallocator(void);
			void		Indent(CChars* pszDest, int iDepth);
			void		AppendToken(CChars* pszDest, int iToken);
			void		AppendTokens(CChars* pszDest, int iFirstToken, int iLastToken);
			void		PrintKeyword(CChars* pszDest, ECppTokens eToken);
			void		PrintNode(CChars* pszDest, CCPPSyntax* pcNode, int iDepth);
			void		PrintList(CChars* pszDest, CArraySyntax* papcNodes, char* szSeparator, int iDepth);
			void		PrintDeclarations(CChars* pszDest, CArraySyntax* papcNodes, int iDepth);
			void		PrintStatements(CChars* pszDest, CArraySyntax* papcNodes, int iDepth);
			void		PrintCondition(CChars* pszDest, CCPPSyntax* pcCondition, int iDepth);
			void		PrintSubStatement(CChars* pszDest, CCPPSyntax* pcStatement, int iDepth);
};


#endif // __C_P_P_SYNTAX_H__

