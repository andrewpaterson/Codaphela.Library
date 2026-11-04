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
#include "BaseLib/MapLongTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPSymbolTable.h"
#include "CPPSyntaxBlock.h"


//The kinds of a block's externals.
#define CPP_SYNTAX_BLOCK_GLOBAL		0		//The global scope.
#define CPP_SYNTAX_BLOCK_MEMBERS	1		//The members of a symbol.
#define CPP_SYNTAX_BLOCK_SYMBOL		2		//A name in a scope (the first, second... symbol of the name there).


//A symbol or scope outside the block that it refers to.
struct SCPPSyntaxBlockExternal
{
	uint8	uiKind;
	uint16	uiScope;	//The scope of a symbol.
	uint16	uiSymbol;	//The symbol whose members a scope is.
	uint16	uiName;
	uint16	uiWhich;	//Which of the symbols of the name in the scope (0 the first).
};


typedef CArrayTemplate<SCPPSyntaxBlockExternal>	CArraySyntaxBlockExternals;
typedef CArrayTemplatePtr<CCPPSymbol>			CArraySymbols;


//A declaration being written as a block: its nodes, scopes, symbols and externals, each given its index as found.
class CCPPSyntaxBlockWriting
{
public:
	CCPPSyntaxTree*				mpcTree;
	CCPPScope*					mpcGlobal;
	int							miFirstToken;
	int							miTokens;
	CArraySyntax				mapcNodes;
	CMapLongTemplate<uint16>	mcNodes;		//By address: its index.
	CArrayScopes				mapcScopes;		//The block's own.
	CArraySymbols				mapcSymbols;	//The block's own.
	CMapLongTemplate<uint16>	mcScopes;		//By address: a reference (the block's own index, or an external's).
	CMapLongTemplate<uint16>	mcSymbols;
	CMapLongTemplate<uint8>		mcOwnScopes;	//The scopes the block's nodes have: the block's own.
	CArraySyntaxBlockExternals	masExternals;
	CArrayTemplate<int64>		malliNames;
	CMapLongTemplate<uint16>	mcNames;
	ECPPSyntaxBlock				meResult;
	CArrayUint8*				mpauiDest;

	void	Init(CCPPSyntaxTree* pcTree, CCPPScope* pcGlobal, CArrayUint8* pauiDest);
	void	Kill(void);

	void	CollectNodes(CCPPSyntax* pcDeclaration);
	uint16	NodeIndex(CCPPSyntax* pcNode);
	bool	IsOwn(CCPPScope* pcScope);
	bool	IsOwn(CCPPSymbol* pcSymbol);
	uint16	Name(int64 lliName);
	uint16	Scope(CCPPScope* pcScope);
	uint16	Symbol(CCPPSymbol* pcSymbol);
	uint16	Token(int iToken);
	uint16	Add(uint16 uiCount);

	void	U8(uint8 ui);
	void	U16(uint16 ui);
	void	U32(uint32 ui);
	void	Bytes(void* pv, size uiSize);
	void	WriteAll(void);
};


//Gives a node's children (CCPPSyntaxBlockWriting::CollectNodes).
class CCPPSyntaxBlockChildren : public CCPPSyntaxFields
{
public:
	CArraySyntax*	mpapcChildren;

	void	NodeField(CCPPSyntax** ppcNode) override		{ if (*ppcNode) { mpapcChildren->Add(*ppcNode); } }
	void	NodesField(CArraySyntax* papcNodes) override	{ for (size i = 0; i < papcNodes->NumElements(); i++) { mpapcChildren->Add(papcNodes->GetPtr(i)); } }
};


//Gives the block's nodes' scopes and symbols their references (CCPPSyntaxBlockWriting::Scope and Symbol), or finds
//the scopes that are the block's own.
class CCPPSyntaxBlockReferences : public CCPPSyntaxFields
{
public:
	CCPPSyntaxBlockWriting*	mpcWriting;
	bool					mbOwnScopes;

	void	SymbolField(CCPPSymbol** ppcSymbol) override	{ if (!mbOwnScopes) { mpcWriting->Symbol(*ppcSymbol); } }
	void	ScopeField(CCPPScope** ppcScope) override
	{
		uint8	uiOwn;

		if (mbOwnScopes)
		{
			uiOwn = 1;
			if (*ppcScope)
			{
				mpcWriting->mcOwnScopes.Put((int64)(ptr)*ppcScope, &uiOwn);
			}
		}
		else
		{
			mpcWriting->Scope(*ppcScope);
		}
	}
};


//Writes a node's fields (CCPPSyntaxBlockWriting::WriteAll).
class CCPPSyntaxBlockFieldWriter : public CCPPSyntaxFields
{
public:
	CCPPSyntaxBlockWriting*	mpcWriting;

	void	NodeField(CCPPSyntax** ppcNode) override		{ mpcWriting->U16(mpcWriting->NodeIndex(*ppcNode)); }
	void	NodesField(CArraySyntax* papcNodes) override
	{
		size	i;

		mpcWriting->U16(mpcWriting->Add((uint16)papcNodes->NumElements()));
		for (i = 0; i < papcNodes->NumElements(); i++)
		{
			mpcWriting->U16(mpcWriting->NodeIndex(papcNodes->GetPtr(i)));
		}
	}
	void	LinkField(CCPPSyntax** ppcNode) override		{ mpcWriting->U16(mpcWriting->NodeIndex(*ppcNode)); }
	void	SymbolField(CCPPSymbol** ppcSymbol) override	{ mpcWriting->U16(mpcWriting->Symbol(*ppcSymbol)); }
	void	ScopeField(CCPPScope** ppcScope) override		{ mpcWriting->U16(mpcWriting->Scope(*ppcScope)); }
	void	TokenField(int32* piToken) override				{ mpcWriting->U16(mpcWriting->Token(*piToken)); }
	void	ValueField(void* pvValue, size uiSize) override	{ mpcWriting->Bytes(pvValue, uiSize); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::Init(CCPPSyntaxTree* pcTree, CCPPScope* pcGlobal, CArrayUint8* pauiDest)
{
	mpcTree = pcTree;
	mpcGlobal = pcGlobal;
	miFirstToken = -1;
	miTokens = 0;
	mapcNodes.Init();
	mcNodes.Init(true);
	mapcScopes.Init();
	mapcSymbols.Init();
	mcScopes.Init(true);
	mcSymbols.Init(true);
	mcOwnScopes.Init(true);
	masExternals.Init();
	malliNames.Init();
	mcNames.Init(true);
	meResult = CPPSB_Written;
	mpauiDest = pauiDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::Kill(void)
{
	mcNames.Kill();
	malliNames.Kill();
	masExternals.Kill();
	mcOwnScopes.Kill();
	mcSymbols.Kill();
	mcScopes.Kill();
	mapcSymbols.Kill();
	mapcScopes.Kill();
	mcNodes.Kill();
	mapcNodes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The declaration's nodes, each once (a node may be the child of more
// than one), parents before their children.  Its tokens are from its first.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::CollectNodes(CCPPSyntax* pcDeclaration)
{
	CArraySyntax				apcStack;
	CArraySyntax				apcChildren;
	CCPPSyntaxBlockChildren		cChildren;
	CCPPSyntax*					pcNode;
	uint16						uiIndex;
	size						i;

	miFirstToken = pcDeclaration->miFirstToken;
	miTokens = (pcDeclaration->miFirstToken >= 0) ? (pcDeclaration->miLastToken - pcDeclaration->miFirstToken + 1) : 0;
	if (miTokens > CPP_SYNTAX_BLOCK_MOST)
	{
		meResult = CPPSB_TooBig;
		return;
	}

	apcStack.Init();
	apcChildren.Init();
	cChildren.mpapcChildren = &apcChildren;
	apcStack.Add(pcDeclaration);
	while (apcStack.NumElements() > 0)
	{
		pcNode = apcStack.GetPtr(apcStack.NumElements() - 1);
		apcStack.RemoveTail();
		if (mcNodes.Get((int64)(ptr)pcNode))
		{
			continue;
		}
		if (mapcNodes.NumElements() >= CPP_SYNTAX_BLOCK_MOST)
		{
			meResult = CPPSB_TooBig;
			break;
		}
		uiIndex = (uint16)mapcNodes.NumElements();
		mcNodes.Put((int64)(ptr)pcNode, &uiIndex);
		mapcNodes.Add(pcNode);

		apcChildren.ReInit();
		pcNode->VisitFields(&cChildren);
		for (i = apcChildren.NumElements(); i > 0; i--)
		{
			apcStack.Add(apcChildren.GetPtr(i - 1));
		}
	}
	apcChildren.Kill();
	apcStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// pcNode's index, CPP_SYNTAX_BLOCK_NULL for none, or CPP_SYNTAX_BLOCK_
// OUTSIDE if it isn't in the block.
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::NodeIndex(CCPPSyntax* pcNode)
{
	uint16*	puiIndex;

	if (!pcNode)
	{
		return CPP_SYNTAX_BLOCK_NULL;
	}
	puiIndex = mcNodes.Get((int64)(ptr)pcNode);
	return puiIndex ? *puiIndex : CPP_SYNTAX_BLOCK_OUTSIDE;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of a name's string (CPP_SYNTAX_BLOCK_NULL for a name that
// isn't an identifier).
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::Name(int64 lliName)
{
	uint16*	puiIndex;
	uint16	uiIndex;

	if ((lliName < 0) || !mpcTree->GetTokeniser()->GetIdentifier(lliName))
	{
		return CPP_SYNTAX_BLOCK_NULL;
	}
	puiIndex = mcNames.Get(lliName);
	if (puiIndex)
	{
		return *puiIndex;
	}
	uiIndex = Add((uint16)malliNames.NumElements());
	malliNames.Add(&lliName);
	mcNames.Put(lliName, &uiIndex);
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// uiCount, or CPP_SYNTAX_BLOCK_NULL (the block is too big) if it is more
// than a block can have.
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::Add(uint16 uiCount)
{
	if (uiCount >= CPP_SYNTAX_BLOCK_MOST)
	{
		meResult = CPPSB_TooBig;
		return CPP_SYNTAX_BLOCK_NULL;
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the scope is the block's own: one its nodes have, or the members
// of its own symbol.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxBlockWriting::IsOwn(CCPPScope* pcScope)
{
	if (mcOwnScopes.Get((int64)(ptr)pcScope))
	{
		return true;
	}
	return pcScope->mpcOwner && (pcScope->mpcOwner->mpcMembers == pcScope) && pcScope->mpcOwner->mpcDeclaration && mcNodes.Get((int64)(ptr)pcScope->mpcOwner->mpcDeclaration);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the symbol is the block's own: one its nodes declare, or one in
// its own scope.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxBlockWriting::IsOwn(CCPPSymbol* pcSymbol)
{
	if (pcSymbol->mpcDeclaration && mcNodes.Get((int64)(ptr)pcSymbol->mpcDeclaration))
	{
		return true;
	}
	return pcSymbol->mpcScope && IsOwn(pcSymbol->mpcScope);
}


//////////////////////////////////////////////////////////////////////////
//
// The reference of pcScope: the block's own scope (IsOwn), an external's
// (the global scope, or the members of a symbol), or CPP_SYNTAX_BLOCK_
// NULL.  The block's own scope's parent, owner, bases, usings, qualifier
// and names get references too.
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::Scope(CCPPScope* pcScope)
{
	uint16*						puiReference;
	uint16						uiReference;
	SCPPSyntaxBlockExternal		sExternal;
	SMapIterator				sIterator;
	CCPPSymbol**				ppcSymbol;
	size						i;
	bool						bMore;

	if (!pcScope)
	{
		return CPP_SYNTAX_BLOCK_NULL;
	}
	puiReference = mcScopes.Get((int64)(ptr)pcScope);
	if (puiReference)
	{
		return *puiReference;
	}

	if (IsOwn(pcScope))
	{
		uiReference = Add((uint16)mapcScopes.NumElements());
		mcScopes.Put((int64)(ptr)pcScope, &uiReference);
		mapcScopes.Add(pcScope);
		Scope(pcScope->mpcParent);
		Symbol(pcScope->mpcOwner);
		Scope(pcScope->mpcQualifier);
		for (i = 0; i < pcScope->mapcBases.NumElements(); i++)
		{
			Scope(pcScope->mapcBases.GetPtr(i));
		}
		for (i = 0; i < pcScope->mapcUsings.NumElements(); i++)
		{
			Scope(pcScope->mapcUsings.GetPtr(i));
		}
		for (bMore = pcScope->mcSymbols.StartIteration(&sIterator, NULL, NULL, (void**)&ppcSymbol, NULL); bMore; bMore = pcScope->mcSymbols.Iterate(&sIterator, NULL, NULL, (void**)&ppcSymbol, NULL))
		{
			Symbol(*ppcSymbol);
		}
		return uiReference;
	}

	memset(&sExternal, 0, sizeof(SCPPSyntaxBlockExternal));
	if (pcScope == mpcGlobal)
	{
		sExternal.uiKind = CPP_SYNTAX_BLOCK_GLOBAL;
	}
	else if (pcScope->mpcOwner && (pcScope->mpcOwner->mpcMembers == pcScope))
	{
		sExternal.uiKind = CPP_SYNTAX_BLOCK_MEMBERS;
		sExternal.uiSymbol = Symbol(pcScope->mpcOwner);
	}
	else
	{
		meResult = CPPSB_UnnamedScope;
		return CPP_SYNTAX_BLOCK_NULL;
	}
	uiReference = Add((uint16)masExternals.NumElements()) | CPP_SYNTAX_BLOCK_EXTERNAL;
	mcScopes.Put((int64)(ptr)pcScope, &uiReference);
	masExternals.Add(&sExternal);
	return uiReference;
}


//////////////////////////////////////////////////////////////////////////
//
// The reference of pcSymbol: the block's own symbol (IsOwn), an external's
// (found by its name in its scope), or CPP_SYNTAX_BLOCK_NULL.  The block's
// own symbol's scopes and next get references too.
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::Symbol(CCPPSymbol* pcSymbol)
{
	uint16*						puiReference;
	uint16						uiReference;
	SCPPSyntaxBlockExternal		sExternal;
	CCPPSymbol**				ppcFirst;
	CCPPSymbol*					pcOther;

	if (!pcSymbol)
	{
		return CPP_SYNTAX_BLOCK_NULL;
	}
	puiReference = mcSymbols.Get((int64)(ptr)pcSymbol);
	if (puiReference)
	{
		return *puiReference;
	}

	if (IsOwn(pcSymbol))
	{
		uiReference = Add((uint16)mapcSymbols.NumElements());
		mcSymbols.Put((int64)(ptr)pcSymbol, &uiReference);
		mapcSymbols.Add(pcSymbol);
		Name(pcSymbol->mlliName);
		Scope(pcSymbol->mpcScope);
		Scope(pcSymbol->mpcMembers);
		Symbol(pcSymbol->mpcNext);
		return uiReference;
	}

	memset(&sExternal, 0, sizeof(SCPPSyntaxBlockExternal));
	sExternal.uiKind = CPP_SYNTAX_BLOCK_SYMBOL;
	sExternal.uiScope = Scope(pcSymbol->mpcScope);
	sExternal.uiName = Name(pcSymbol->mlliName);
	ppcFirst = pcSymbol->mpcScope ? pcSymbol->mpcScope->mcSymbols.Get(pcSymbol->mlliName) : NULL;
	for (pcOther = ppcFirst ? *ppcFirst : NULL; pcOther && (pcOther != pcSymbol); pcOther = pcOther->mpcNext)
	{
		sExternal.uiWhich++;
	}
	if (!pcOther || (sExternal.uiScope == CPP_SYNTAX_BLOCK_NULL) || (sExternal.uiName == CPP_SYNTAX_BLOCK_NULL))
	{
		if (meResult == CPPSB_Written)
		{
			meResult = CPPSB_UnnamedSymbol;
		}
		return CPP_SYNTAX_BLOCK_NULL;
	}
	uiReference = Add((uint16)masExternals.NumElements()) | CPP_SYNTAX_BLOCK_EXTERNAL;
	mcSymbols.Put((int64)(ptr)pcSymbol, &uiReference);
	masExternals.Add(&sExternal);
	return uiReference;
}


//////////////////////////////////////////////////////////////////////////
//
// iToken from the declaration's first token, or CPP_SYNTAX_BLOCK_NULL for
// none (-1).
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::Token(int iToken)
{
	if (iToken < 0)
	{
		return CPP_SYNTAX_BLOCK_NULL;
	}
	if ((iToken < miFirstToken) || (iToken - miFirstToken >= miTokens))
	{
		meResult = CPPSB_Tokens;
		return CPP_SYNTAX_BLOCK_NULL;
	}
	return (uint16)(iToken - miFirstToken);
}


//////////////////////////////////////////////////////////////////////////
//
// Little endian.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::U8(uint8 ui)
{
	mpauiDest->Add(ui);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::U16(uint16 ui)
{
	U8((uint8)ui);
	U8((uint8)(ui >> 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::U32(uint32 ui)
{
	U16((uint16)ui);
	U16((uint16)(ui >> 16));
}


//////////////////////////////////////////////////////////////////////////
//
// A field's value as it is in memory (bools, enums and integers, which
// are little endian on the hosts the compiler runs on).
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::Bytes(void* pv, size uiSize)
{
	size	i;

	for (i = 0; i < uiSize; i++)
	{
		U8(((uint8*)pv)[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The block's bytes, every reference given (CollectNodes, then the nodes'
// scopes and symbols, and the tokens').
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::WriteAll(void)
{
	CCPPSyntaxBlockFieldWriter	cFields;
	CCPPSyntax*					pcNode;
	CCPPScope*					pcScope;
	CCPPSymbol*					pcSymbol;
	SCPPSyntaxBlockExternal*	psExternal;
	SCPPTokenSymbol*			psTokenSymbol;
	SMapIterator				sIterator;
	CCPPSymbol**				ppcSymbol;
	int64*						plliName;
	char*						szName;
	int							iLength;
	size						uiStart;
	size						uiEntries;
	size						uiCount;
	size						i;
	size						j;
	bool						bMore;

	U32(CPP_SYNTAX_BLOCK_MAGIC);
	U16((uint16)mapcNodes.NumElements());
	U16((uint16)malliNames.NumElements());
	U16((uint16)masExternals.NumElements());
	U16((uint16)mapcScopes.NumElements());
	U16((uint16)mapcSymbols.NumElements());
	U16((uint16)miTokens);

	for (i = 0; i < mapcNodes.NumElements(); i++)
	{
		U8((uint8)mapcNodes.GetPtr(i)->GetKind());
	}

	for (i = 0; i < malliNames.NumElements(); i++)
	{
		szName = mpcTree->GetTokeniser()->GetIdentifier(*malliNames.Get(i), &iLength);
		U16((uint16)iLength);
		Bytes(szName, iLength);
	}

	for (i = 0; i < masExternals.NumElements(); i++)
	{
		psExternal = masExternals.Get(i);
		U8(psExternal->uiKind);
		if (psExternal->uiKind == CPP_SYNTAX_BLOCK_MEMBERS)
		{
			U16(psExternal->uiSymbol);
		}
		else if (psExternal->uiKind == CPP_SYNTAX_BLOCK_SYMBOL)
		{
			U16(psExternal->uiScope);
			U16(psExternal->uiName);
			U16(psExternal->uiWhich);
		}
	}

	for (i = 0; i < mapcScopes.NumElements(); i++)
	{
		pcScope = mapcScopes.GetPtr(i);
		U8((uint8)pcScope->meKind);
		U16(Scope(pcScope->mpcParent));
		U16(Symbol(pcScope->mpcOwner));
		U8(pcScope->mbDependent ? 1 : 0);
		U16(Scope(pcScope->mpcQualifier));
		U16(Add((uint16)pcScope->mapcBases.NumElements()));
		for (j = 0; j < pcScope->mapcBases.NumElements(); j++)
		{
			U16(Scope(pcScope->mapcBases.GetPtr(j)));
		}
		U16(Add((uint16)pcScope->mapcUsings.NumElements()));
		for (j = 0; j < pcScope->mapcUsings.NumElements(); j++)
		{
			U16(Scope(pcScope->mapcUsings.GetPtr(j)));
		}

		//The names in the scope, the count written over when they have been.
		uiStart = mpauiDest->NumElements();
		U16(0);
		uiEntries = 0;
		for (bMore = pcScope->mcSymbols.StartIteration(&sIterator, (void**)&plliName, NULL, (void**)&ppcSymbol, NULL); bMore; bMore = pcScope->mcSymbols.Iterate(&sIterator, (void**)&plliName, NULL, (void**)&ppcSymbol, NULL))
		{
			U16(Name(*plliName));
			U16(Symbol(*ppcSymbol));
			uiEntries++;
		}
		uiCount = Add((uint16)uiEntries);
		*mpauiDest->Get(uiStart) = (uint8)uiCount;
		*mpauiDest->Get(uiStart + 1) = (uint8)(uiCount >> 8);
	}

	for (i = 0; i < mapcSymbols.NumElements(); i++)
	{
		pcSymbol = mapcSymbols.GetPtr(i);
		U16(Name(pcSymbol->mlliName));
		U8((uint8)pcSymbol->meKind);
		U16(NodeIndex(pcSymbol->mpcDeclaration));
		U16(Scope(pcSymbol->mpcScope));
		U16(Scope(pcSymbol->mpcMembers));
		U16(Symbol(pcSymbol->mpcNext));
		U16(NodeIndex(pcSymbol->mpcLazy));
	}

	cFields.mpcWriting = this;
	for (i = 0; i < mapcNodes.NumElements(); i++)
	{
		pcNode = mapcNodes.GetPtr(i);
		U16((i == 0) ? CPP_SYNTAX_BLOCK_OUTSIDE : NodeIndex(pcNode->mpcParent));
		U16(Token(pcNode->miFirstToken));
		U16(Token(pcNode->miLastToken));
		pcNode->VisitFields(&cFields);
	}

	for (i = 0; i < (size)miTokens; i++)
	{
		psTokenSymbol = mpcTree->GetTokenSymbol(miFirstToken + (int)i);
		U16(psTokenSymbol ? Symbol(psTokenSymbol->pcSymbol) : CPP_SYNTAX_BLOCK_NULL);
		U8(psTokenSymbol ? psTokenSymbol->uiRole : 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes pcDeclaration (a node of pcTree) to pauiDest, appended, as a
// block whose externals are found from pcGlobal.  CPPSB_Written if it could
// be (otherwise pauiDest is as it was).
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntaxBlock CCPPSyntaxBlockWriter::Write(CCPPSyntaxTree* pcTree, CCPPSyntax* pcDeclaration, CCPPScope* pcGlobal, CArrayUint8* pauiDest)
{
	CCPPSyntaxBlockWriting		cWriting;
	CCPPSyntaxBlockReferences	cReferences;
	SCPPTokenSymbol*			psTokenSymbol;
	ECPPSyntaxBlock				eResult;
	size						uiStart;
	size						i;

	uiStart = pauiDest->NumElements();
	cWriting.Init(pcTree, pcGlobal, pauiDest);
	cWriting.CollectNodes(pcDeclaration);

	//The scopes the nodes have are the block's own, then every scope and symbol the nodes and tokens refer to gets
	//its reference (so the bytes, written in order, can refer to any of them).
	cReferences.mpcWriting = &cWriting;
	cReferences.mbOwnScopes = true;
	for (i = 0; (cWriting.meResult == CPPSB_Written) && (i < cWriting.mapcNodes.NumElements()); i++)
	{
		cWriting.mapcNodes.GetPtr(i)->VisitFields(&cReferences);
	}
	cReferences.mbOwnScopes = false;
	for (i = 0; (cWriting.meResult == CPPSB_Written) && (i < cWriting.mapcNodes.NumElements()); i++)
	{
		cWriting.mapcNodes.GetPtr(i)->VisitFields(&cReferences);
	}
	for (i = 0; (cWriting.meResult == CPPSB_Written) && (i < (size)cWriting.miTokens); i++)
	{
		psTokenSymbol = pcTree->GetTokenSymbol(cWriting.miFirstToken + (int)i);
		cWriting.Symbol(psTokenSymbol ? psTokenSymbol->pcSymbol : NULL);
	}

	if (cWriting.meResult == CPPSB_Written)
	{
		cWriting.WriteAll();
	}
	eResult = cWriting.meResult;
	if (eResult != CPPSB_Written)
	{
		pauiDest->SetUsedElements(uiStart);
	}
	cWriting.Kill();
	return eResult;
}


//A block being read: its bytes, and what its indices are.
class CCPPSyntaxBlockReading
{
public:
	uint8*					mpuiBlock;
	size					muiLength;
	size					muiPosition;
	bool					mbError;
	CCPPSyntaxTree*			mpcTree;
	int						miFirstToken;
	int						miTokens;
	CArraySyntax			mapcNodes;
	CArrayTemplate<int64>	malliNames;
	CArrayScopes			mapcScopes;
	CArraySymbols			mapcSymbols;
	CArrayScopes			mapcExternalScopes;		//By external: the scope it is, or NULL.
	CArraySymbols			mapcExternalSymbols;	//By external: the symbol it is, or NULL.

	void			Init(uint8* puiBlock, size uiLength, CCPPSyntaxTree* pcTree, int iFirstToken);
	void			Kill(void);

	uint8			U8(void);
	uint16			U16(void);
	uint32			U32(void);
	void			Bytes(void* pv, size uiSize);
	CCPPSyntax*		Node(uint16 uiIndex);
	int64			Name(uint16 uiIndex);
	CCPPScope*		Scope(uint16 uiReference);
	CCPPSymbol*		Symbol(uint16 uiReference);
	int				Token(uint16 uiToken);
};


//Reads a node's fields (CCPPSyntaxBlockReader::Read).
class CCPPSyntaxBlockFieldReader : public CCPPSyntaxFields
{
public:
	CCPPSyntaxBlockReading*	mpcReading;

	void	NodeField(CCPPSyntax** ppcNode) override		{ *ppcNode = mpcReading->Node(mpcReading->U16()); }
	void	NodesField(CArraySyntax* papcNodes) override
	{
		uint16							uiCount;
		uint16							i;
		CCPPSyntax*						pcNode;
		CCPPSyntaxLazyDeclaration*		pcLazy;

		uiCount = mpcReading->U16();
		for (i = 0; (i < uiCount) && !mpcReading->mbError; i++)
		{
			pcNode = mpcReading->Node(mpcReading->U16());
			if (pcNode)
			{
				papcNodes->Add(pcNode);
				if (pcNode->GetKind() == CPPS_LazyDeclaration)
				{
					//A lazy declaration knows where it is in its owner's list, to be put there when it is parsed.
					pcLazy = (CCPPSyntaxLazyDeclaration*)pcNode;
					pcLazy->mpapcOwner = papcNodes;
					pcLazy->muiIndex = papcNodes->NumElements() - 1;
				}
			}
		}
	}
	void	LinkField(CCPPSyntax** ppcNode) override		{ *ppcNode = mpcReading->Node(mpcReading->U16()); }
	void	SymbolField(CCPPSymbol** ppcSymbol) override	{ *ppcSymbol = mpcReading->Symbol(mpcReading->U16()); }
	void	ScopeField(CCPPScope** ppcScope) override		{ *ppcScope = mpcReading->Scope(mpcReading->U16()); }
	void	TokenField(int32* piToken) override				{ *piToken = mpcReading->Token(mpcReading->U16()); }
	void	ValueField(void* pvValue, size uiSize) override	{ mpcReading->Bytes(pvValue, uiSize); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockReading::Init(uint8* puiBlock, size uiLength, CCPPSyntaxTree* pcTree, int iFirstToken)
{
	mpuiBlock = puiBlock;
	muiLength = uiLength;
	muiPosition = 0;
	mbError = false;
	mpcTree = pcTree;
	miFirstToken = iFirstToken;
	miTokens = 0;
	mapcNodes.Init();
	malliNames.Init();
	mapcScopes.Init();
	mapcSymbols.Init();
	mapcExternalScopes.Init();
	mapcExternalSymbols.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockReading::Kill(void)
{
	mapcExternalSymbols.Kill();
	mapcExternalScopes.Kill();
	mapcSymbols.Kill();
	mapcScopes.Kill();
	malliNames.Kill();
	mapcNodes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Little endian.  Reading past the end is an error (and reads 0).
//
//////////////////////////////////////////////////////////////////////////
uint8 CCPPSyntaxBlockReading::U8(void)
{
	if (muiPosition >= muiLength)
	{
		mbError = true;
		return 0;
	}
	return mpuiBlock[muiPosition++];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockReading::U16(void)
{
	uint16	uiLow;

	uiLow = U8();
	return (uint16)(uiLow | ((uint16)U8() << 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPPSyntaxBlockReading::U32(void)
{
	uint32	uiLow;

	uiLow = U16();
	return uiLow | ((uint32)U16() << 16);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockReading::Bytes(void* pv, size uiSize)
{
	size	i;

	for (i = 0; i < uiSize; i++)
	{
		((uint8*)pv)[i] = U8();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The node with the index, NULL for CPP_SYNTAX_BLOCK_NULL or CPP_SYNTAX_
// BLOCK_OUTSIDE (a link outside the block isn't kept).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxBlockReading::Node(uint16 uiIndex)
{
	if ((uiIndex == CPP_SYNTAX_BLOCK_NULL) || (uiIndex == CPP_SYNTAX_BLOCK_OUTSIDE))
	{
		return NULL;
	}
	if (uiIndex >= mapcNodes.NumElements())
	{
		mbError = true;
		return NULL;
	}
	return mapcNodes.GetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPSyntaxBlockReading::Name(uint16 uiIndex)
{
	if (uiIndex == CPP_SYNTAX_BLOCK_NULL)
	{
		return -1;
	}
	if (uiIndex >= malliNames.NumElements())
	{
		mbError = true;
		return -1;
	}
	return *malliNames.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPSyntaxBlockReading::Scope(uint16 uiReference)
{
	CArrayScopes*	papcScopes;

	if (uiReference == CPP_SYNTAX_BLOCK_NULL)
	{
		return NULL;
	}
	papcScopes = (uiReference & CPP_SYNTAX_BLOCK_EXTERNAL) ? &mapcExternalScopes : &mapcScopes;
	uiReference &= ~CPP_SYNTAX_BLOCK_EXTERNAL;
	if ((uiReference >= papcScopes->NumElements()) || !papcScopes->GetPtr(uiReference))
	{
		mbError = true;
		return NULL;
	}
	return papcScopes->GetPtr(uiReference);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPSyntaxBlockReading::Symbol(uint16 uiReference)
{
	CArraySymbols*	papcSymbols;

	if (uiReference == CPP_SYNTAX_BLOCK_NULL)
	{
		return NULL;
	}
	papcSymbols = (uiReference & CPP_SYNTAX_BLOCK_EXTERNAL) ? &mapcExternalSymbols : &mapcSymbols;
	uiReference &= ~CPP_SYNTAX_BLOCK_EXTERNAL;
	if ((uiReference >= papcSymbols->NumElements()) || !papcSymbols->GetPtr(uiReference))
	{
		mbError = true;
		return NULL;
	}
	return papcSymbols->GetPtr(uiReference);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPSyntaxBlockReading::Token(uint16 uiToken)
{
	if (uiToken == CPP_SYNTAX_BLOCK_NULL)
	{
		return -1;
	}
	if (uiToken >= miTokens)
	{
		mbError = true;
		return -1;
	}
	return miFirstToken + uiToken;
}


//////////////////////////////////////////////////////////////////////////
//
// A node of the kind, in pcMemory, with no parent yet.
//
//////////////////////////////////////////////////////////////////////////
static CCPPSyntax* CPPSyntaxBlockCreate(CCPPSyntaxMemory* pcMemory, ECPPSyntax eKind)
{
	switch (eKind)
	{
	case CPPS_TranslationUnit:
		return pcMemory->Create<CCPPSyntaxTranslationUnit>(NULL);
	case CPPS_Namespace:
		return pcMemory->Create<CCPPSyntaxNamespace>(NULL);
	case CPPS_LinkageSpecification:
		return pcMemory->Create<CCPPSyntaxLinkageSpecification>(NULL);
	case CPPS_SimpleDeclaration:
		return pcMemory->Create<CCPPSyntaxSimpleDeclaration>(NULL);
	case CPPS_FunctionDefinition:
		return pcMemory->Create<CCPPSyntaxFunctionDefinition>(NULL);
	case CPPS_InitDeclarator:
		return pcMemory->Create<CCPPSyntaxInitDeclarator>(NULL);
	case CPPS_TemplateDeclaration:
		return pcMemory->Create<CCPPSyntaxTemplateDeclaration>(NULL);
	case CPPS_TypeParameter:
		return pcMemory->Create<CCPPSyntaxTypeParameter>(NULL);
	case CPPS_Parameter:
		return pcMemory->Create<CCPPSyntaxParameter>(NULL);
	case CPPS_UsingDirective:
		return pcMemory->Create<CCPPSyntaxUsingDirective>(NULL);
	case CPPS_UsingDeclaration:
		return pcMemory->Create<CCPPSyntaxUsingDeclaration>(NULL);
	case CPPS_AliasDeclaration:
		return pcMemory->Create<CCPPSyntaxAliasDeclaration>(NULL);
	case CPPS_StaticAssert:
		return pcMemory->Create<CCPPSyntaxStaticAssert>(NULL);
	case CPPS_ExtensionDeclaration:
		return pcMemory->Create<CCPPSyntaxExtensionDeclaration>(NULL);
	case CPPS_EmptyDeclaration:
		return pcMemory->Create<CCPPSyntaxEmptyDeclaration>(NULL);
	case CPPS_AccessSpecifier:
		return pcMemory->Create<CCPPSyntaxAccessSpecifier>(NULL);
	case CPPS_SkippedDeclaration:
		return pcMemory->Create<CCPPSyntaxSkippedDeclaration>(NULL);
	case CPPS_Enumerator:
		return pcMemory->Create<CCPPSyntaxEnumerator>(NULL);
	case CPPS_BaseSpecifier:
		return pcMemory->Create<CCPPSyntaxBaseSpecifier>(NULL);
	case CPPS_MemberInitializer:
		return pcMemory->Create<CCPPSyntaxMemberInitializer>(NULL);
	case CPPS_Attribute:
		return pcMemory->Create<CCPPSyntaxAttribute>(NULL);
	case CPPS_DeclSpecifiers:
		return pcMemory->Create<CCPPSyntaxDeclSpecifiers>(NULL);
	case CPPS_KeywordSpecifier:
		return pcMemory->Create<CCPPSyntaxKeywordSpecifier>(NULL);
	case CPPS_AlignasSpecifier:
		return pcMemory->Create<CCPPSyntaxAlignasSpecifier>(NULL);
	case CPPS_NamedTypeSpecifier:
		return pcMemory->Create<CCPPSyntaxNamedTypeSpecifier>(NULL);
	case CPPS_ClassSpecifier:
		return pcMemory->Create<CCPPSyntaxClassSpecifier>(NULL);
	case CPPS_EnumSpecifier:
		return pcMemory->Create<CCPPSyntaxEnumSpecifier>(NULL);
	case CPPS_ElaboratedTypeSpecifier:
		return pcMemory->Create<CCPPSyntaxElaboratedTypeSpecifier>(NULL);
	case CPPS_DecltypeSpecifier:
		return pcMemory->Create<CCPPSyntaxDecltypeSpecifier>(NULL);
	case CPPS_Declarator:
		return pcMemory->Create<CCPPSyntaxDeclarator>(NULL);
	case CPPS_PointerOperator:
		return pcMemory->Create<CCPPSyntaxPointerOperator>(NULL);
	case CPPS_FunctionSuffix:
		return pcMemory->Create<CCPPSyntaxFunctionSuffix>(NULL);
	case CPPS_ArraySuffix:
		return pcMemory->Create<CCPPSyntaxArraySuffix>(NULL);
	case CPPS_ExceptionSpecification:
		return pcMemory->Create<CCPPSyntaxExceptionSpecification>(NULL);
	case CPPS_TypeId:
		return pcMemory->Create<CCPPSyntaxTypeId>(NULL);
	case CPPS_Name:
		return pcMemory->Create<CCPPSyntaxName>(NULL);
	case CPPS_NameComponent:
		return pcMemory->Create<CCPPSyntaxNameComponent>(NULL);
	case CPPS_CompoundStatement:
		return pcMemory->Create<CCPPSyntaxCompoundStatement>(NULL);
	case CPPS_DeclarationStatement:
		return pcMemory->Create<CCPPSyntaxDeclarationStatement>(NULL);
	case CPPS_ExpressionStatement:
		return pcMemory->Create<CCPPSyntaxExpressionStatement>(NULL);
	case CPPS_IfStatement:
		return pcMemory->Create<CCPPSyntaxIfStatement>(NULL);
	case CPPS_SwitchStatement:
		return pcMemory->Create<CCPPSyntaxSwitchStatement>(NULL);
	case CPPS_CaseStatement:
		return pcMemory->Create<CCPPSyntaxCaseStatement>(NULL);
	case CPPS_LabelStatement:
		return pcMemory->Create<CCPPSyntaxLabelStatement>(NULL);
	case CPPS_AttributedStatement:
		return pcMemory->Create<CCPPSyntaxAttributedStatement>(NULL);
	case CPPS_WhileStatement:
		return pcMemory->Create<CCPPSyntaxWhileStatement>(NULL);
	case CPPS_DoStatement:
		return pcMemory->Create<CCPPSyntaxDoStatement>(NULL);
	case CPPS_ForStatement:
		return pcMemory->Create<CCPPSyntaxForStatement>(NULL);
	case CPPS_RangeForStatement:
		return pcMemory->Create<CCPPSyntaxRangeForStatement>(NULL);
	case CPPS_BreakStatement:
		return pcMemory->Create<CCPPSyntaxBreakStatement>(NULL);
	case CPPS_ContinueStatement:
		return pcMemory->Create<CCPPSyntaxContinueStatement>(NULL);
	case CPPS_ReturnStatement:
		return pcMemory->Create<CCPPSyntaxReturnStatement>(NULL);
	case CPPS_GotoStatement:
		return pcMemory->Create<CCPPSyntaxGotoStatement>(NULL);
	case CPPS_TryStatement:
		return pcMemory->Create<CCPPSyntaxTryStatement>(NULL);
	case CPPS_CatchHandler:
		return pcMemory->Create<CCPPSyntaxCatchHandler>(NULL);
	case CPPS_SkippedStatement:
		return pcMemory->Create<CCPPSyntaxSkippedStatement>(NULL);
	case CPPS_LiteralExpression:
		return pcMemory->Create<CCPPSyntaxLiteralExpression>(NULL);
	case CPPS_StringExpression:
		return pcMemory->Create<CCPPSyntaxStringExpression>(NULL);
	case CPPS_IdExpression:
		return pcMemory->Create<CCPPSyntaxIdExpression>(NULL);
	case CPPS_ThisExpression:
		return pcMemory->Create<CCPPSyntaxThisExpression>(NULL);
	case CPPS_ParenthesisedExpression:
		return pcMemory->Create<CCPPSyntaxParenthesisedExpression>(NULL);
	case CPPS_UnaryExpression:
		return pcMemory->Create<CCPPSyntaxUnaryExpression>(NULL);
	case CPPS_BinaryExpression:
		return pcMemory->Create<CCPPSyntaxBinaryExpression>(NULL);
	case CPPS_ConditionalExpression:
		return pcMemory->Create<CCPPSyntaxConditionalExpression>(NULL);
	case CPPS_CallExpression:
		return pcMemory->Create<CCPPSyntaxCallExpression>(NULL);
	case CPPS_SubscriptExpression:
		return pcMemory->Create<CCPPSyntaxSubscriptExpression>(NULL);
	case CPPS_MemberExpression:
		return pcMemory->Create<CCPPSyntaxMemberExpression>(NULL);
	case CPPS_CastExpression:
		return pcMemory->Create<CCPPSyntaxCastExpression>(NULL);
	case CPPS_ConstructExpression:
		return pcMemory->Create<CCPPSyntaxConstructExpression>(NULL);
	case CPPS_NamedCastExpression:
		return pcMemory->Create<CCPPSyntaxNamedCastExpression>(NULL);
	case CPPS_SizeofExpression:
		return pcMemory->Create<CCPPSyntaxSizeofExpression>(NULL);
	case CPPS_TypeidExpression:
		return pcMemory->Create<CCPPSyntaxTypeidExpression>(NULL);
	case CPPS_NewExpression:
		return pcMemory->Create<CCPPSyntaxNewExpression>(NULL);
	case CPPS_DeleteExpression:
		return pcMemory->Create<CCPPSyntaxDeleteExpression>(NULL);
	case CPPS_ThrowExpression:
		return pcMemory->Create<CCPPSyntaxThrowExpression>(NULL);
	case CPPS_InitializerList:
		return pcMemory->Create<CCPPSyntaxInitializerList>(NULL);
	case CPPS_ParenthesisedInitializer:
		return pcMemory->Create<CCPPSyntaxParenthesisedInitializer>(NULL);
	case CPPS_TypeTraitExpression:
		return pcMemory->Create<CCPPSyntaxTypeTraitExpression>(NULL);
	case CPPS_PackExpansionExpression:
		return pcMemory->Create<CCPPSyntaxPackExpansionExpression>(NULL);
	case CPPS_NoexceptExpression:
		return pcMemory->Create<CCPPSyntaxNoexceptExpression>(NULL);
	case CPPS_LazyDeclaration:
		return pcMemory->Create<CCPPSyntaxLazyDeclaration>(NULL);
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Reads a block (CCPPSyntaxBlockWriter::Write) into pcTree, its tokens
// from iFirstToken, its externals found from pcGlobal: its declaration,
// whose parent is pcParent, or NULL if the block is wrong or an external
// isn't found.  Its lazy declarations are parsed by pcLoader.  Its own
// symbols are in its own scopes (and not added to the scopes outside it).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxBlockReader::Read(uint8* puiBlock, size uiLength, CCPPSyntaxTree* pcTree, int iFirstToken, CCPPScope* pcGlobal, CCPPSyntax* pcParent, CCPPLazyLoader* pcLoader)
{
	CCPPSyntaxBlockReading		cReading;
	CCPPSyntaxBlockFieldReader	cFields;
	CCPPSymbolTable*			pcSymbols;
	CCPPScope*					pcScope;
	CCPPSymbol*					pcSymbol;
	CCPPSymbol**				ppcFirst;
	CCPPSyntax*					pcNode;
	CCPPSyntax*					pcDeclaration;
	CChars						szName;
	uint16						uiNodes;
	uint16						uiNames;
	uint16						uiExternals;
	uint16						uiScopes;
	uint16						uiSymbols;
	uint16						uiCount;
	uint16						uiWhich;
	uint16						uiParent;
	uint8						uiKind;
	uint8						uiRole;
	int64						lliName;
	size						i;
	size						j;

	cReading.Init(puiBlock, uiLength, pcTree, iFirstToken);
	pcSymbols = pcTree->GetSymbols();
	if (cReading.U32() != CPP_SYNTAX_BLOCK_MAGIC)
	{
		cReading.Kill();
		return NULL;
	}
	uiNodes = cReading.U16();
	uiNames = cReading.U16();
	uiExternals = cReading.U16();
	uiScopes = cReading.U16();
	uiSymbols = cReading.U16();
	cReading.miTokens = cReading.U16();

	//Every node is made before any record that refers to one is read.
	for (i = 0; (i < uiNodes) && !cReading.mbError; i++)
	{
		pcNode = CPPSyntaxBlockCreate(pcTree->GetMemory(), (ECPPSyntax)cReading.U8());
		cReading.mbError |= !pcNode;
		cReading.mapcNodes.Add(pcNode);
	}

	//The names, as the tree's tokeniser's identifiers.
	for (i = 0; (i < uiNames) && !cReading.mbError; i++)
	{
		uiCount = cReading.U16();
		if (cReading.muiPosition + uiCount > uiLength)
		{
			cReading.mbError = true;
			break;
		}
		szName.Init((char*)&puiBlock[cReading.muiPosition], 0, uiCount);
		lliName = pcTree->GetTokeniser()->GetIdentifierID(szName.Text());
		szName.Kill();
		cReading.malliNames.Add(&lliName);
		cReading.muiPosition += uiCount;
	}

	//The block's own scopes and symbols, their fields read below.
	for (i = 0; i < uiScopes; i++)
	{
		cReading.mapcScopes.Add(pcSymbols->CreateScope(CPPSC_Namespace, NULL, NULL));
	}
	for (i = 0; i < uiSymbols; i++)
	{
		cReading.mapcSymbols.Add(pcSymbols->CreateSymbol(-1, CPPSK_Variable, NULL, NULL));
	}

	//The externals, each found from those before it.
	for (i = 0; (i < uiExternals) && !cReading.mbError; i++)
	{
		pcScope = NULL;
		pcSymbol = NULL;
		uiKind = cReading.U8();
		if (uiKind == CPP_SYNTAX_BLOCK_GLOBAL)
		{
			pcScope = pcGlobal;
		}
		else if (uiKind == CPP_SYNTAX_BLOCK_MEMBERS)
		{
			pcSymbol = cReading.Symbol(cReading.U16());
			pcScope = pcSymbol ? pcSymbol->mpcMembers : NULL;
			pcSymbol = NULL;
		}
		else if (uiKind == CPP_SYNTAX_BLOCK_SYMBOL)
		{
			pcScope = cReading.Scope(cReading.U16());
			lliName = cReading.Name(cReading.U16());
			uiWhich = cReading.U16();
			ppcFirst = pcScope ? pcScope->mcSymbols.Get(lliName) : NULL;
			for (pcSymbol = ppcFirst ? *ppcFirst : NULL; pcSymbol && (uiWhich > 0); pcSymbol = pcSymbol->mpcNext)
			{
				uiWhich--;
			}
			pcScope = NULL;
		}
		cReading.mbError |= !pcScope && !pcSymbol;
		cReading.mapcExternalScopes.Add(pcScope);
		cReading.mapcExternalSymbols.Add(pcSymbol);
	}

	for (i = 0; (i < uiScopes) && !cReading.mbError; i++)
	{
		pcScope = cReading.mapcScopes.GetPtr(i);
		pcScope->meKind = (ECPPScopeKind)cReading.U8();
		pcScope->mpcParent = cReading.Scope(cReading.U16());
		pcScope->mpcOwner = cReading.Symbol(cReading.U16());
		pcScope->mbDependent = cReading.U8() != 0;
		pcScope->mpcQualifier = cReading.Scope(cReading.U16());
		uiCount = cReading.U16();
		for (j = 0; (j < uiCount) && !cReading.mbError; j++)
		{
			pcScope->mapcBases.Add(cReading.Scope(cReading.U16()));
		}
		uiCount = cReading.U16();
		for (j = 0; (j < uiCount) && !cReading.mbError; j++)
		{
			pcScope->mapcUsings.Add(cReading.Scope(cReading.U16()));
		}
		uiCount = cReading.U16();
		for (j = 0; (j < uiCount) && !cReading.mbError; j++)
		{
			lliName = cReading.Name(cReading.U16());
			pcSymbol = cReading.Symbol(cReading.U16());
			pcScope->mcSymbols.Put(lliName, &pcSymbol);
		}
	}

	for (i = 0; (i < uiSymbols) && !cReading.mbError; i++)
	{
		pcSymbol = cReading.mapcSymbols.GetPtr(i);
		pcSymbol->mlliName = cReading.Name(cReading.U16());
		pcSymbol->meKind = (ECPPSymbolKind)cReading.U8();
		pcSymbol->mpcDeclaration = cReading.Node(cReading.U16());
		pcSymbol->mpcScope = cReading.Scope(cReading.U16());
		pcSymbol->mpcMembers = cReading.Scope(cReading.U16());
		pcSymbol->mpcNext = cReading.Symbol(cReading.U16());
		pcSymbol->mpcLazy = (CCPPSyntaxLazyDeclaration*)cReading.Node(cReading.U16());
		pcSymbol->mpcLoader = pcSymbol->mpcLazy ? pcLoader : NULL;
	}

	cFields.mpcReading = &cReading;
	for (i = 0; (i < uiNodes) && !cReading.mbError; i++)
	{
		pcNode = cReading.mapcNodes.GetPtr(i);
		uiParent = cReading.U16();
		pcNode->mpcParent = (i == 0) ? pcParent : cReading.Node(uiParent);
		pcNode->miFirstToken = cReading.Token(cReading.U16());
		pcNode->miLastToken = cReading.Token(cReading.U16());
		pcNode->VisitFields(&cFields);
	}

	for (i = 0; (i < (size)cReading.miTokens) && !cReading.mbError; i++)
	{
		pcSymbol = cReading.Symbol(cReading.U16());
		uiRole = cReading.U8();
		if (pcSymbol || (uiRole != 0))
		{
			pcTree->SetTokenSymbol(iFirstToken + (int)i, pcSymbol, (ECPPTokenRole)uiRole);
		}
	}

	pcDeclaration = (!cReading.mbError && (cReading.muiPosition == uiLength) && (uiNodes > 0)) ? cReading.mapcNodes.GetPtr(0) : NULL;
	cReading.Kill();
	return pcDeclaration;
}

