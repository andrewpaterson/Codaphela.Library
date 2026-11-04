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
#include "SourceFile.h"
#include "CPPSyntaxes.h"
#include "CPPSymbolTable.h"
#include "CPPSyntaxBlock.h"


//The kinds of a block's externals.
#define CPP_SYNTAX_BLOCK_GLOBAL		0		//The global scope.
#define CPP_SYNTAX_BLOCK_MEMBERS	1		//The members of a symbol.
#define CPP_SYNTAX_BLOCK_SYMBOL		2		//A name in a scope, the symbol of it declared at a place in a file.
#define CPP_SYNTAX_BLOCK_PARENT		3		//The parent of a scope (a class template's parameters: its members' parent).
#define CPP_SYNTAX_BLOCK_NO_LINE	0xFFFFFFFF	//A symbol with no declaration (a compiler type).


//A symbol or scope outside the block that it refers to.
struct SCPPSyntaxBlockExternal
{
	uint8	uiKind;
	uint16	uiScope;	//The scope of a symbol, or the scope whose parent this is.
	uint16	uiSymbol;	//The symbol whose members a scope is.
	uint16	uiName;
	uint16	uiFile;		//Where a symbol's declaration starts: the file's string, its line and column.
	uint32	uiLine;
	uint16	uiColumn;
};


//A string of a block (a name or a file's): its characters and how many.
struct SCPPSyntaxBlockString
{
	const char*	sz;
	int			iLength;
};


typedef CArrayTemplate<SCPPSyntaxBlockExternal>	CArraySyntaxBlockExternals;
typedef CArrayTemplate<SCPPSyntaxBlockString>	CArraySyntaxBlockStrings;
typedef CArrayTemplatePtr<CCPPSymbol>			CArraySymbols;


//////////////////////////////////////////////////////////////////////////
//
// Where pcSymbol's declaration starts in pcTree's tokens (its file, line
// and column), or false if it has none.  Symbols outside a block are found
// by it, the same in any tree with the declaration.
//
//////////////////////////////////////////////////////////////////////////
static bool CPPSyntaxBlockPosition(CCPPSyntaxTree* pcTree, CCPPSymbol* pcSymbol, CSourceFile** ppcFile, int* piLine, int* piColumn)
{
	CCPPTokeniser*			pcTokeniser;
	CCPPToken*				pcToken;
	SPreprocessedToken*		psSource;

	if (!pcSymbol->mpcDeclaration || (pcSymbol->mpcDeclaration->miFirstToken < 0))
	{
		return false;
	}
	pcTokeniser = pcTree->GetTokeniser();
	pcToken = pcTokeniser->Get(pcSymbol->mpcDeclaration->miFirstToken);
	psSource = pcToken ? pcTokeniser->GetSource(pcToken) : NULL;
	if (!psSource || !psSource->pcFile)
	{
		return false;
	}
	*ppcFile = psSource->pcFile;
	*piLine = psSource->iLine;
	*piColumn = psSource->iColumn;
	return true;
}


//Keys (addresses, or names + 1: never 0) and their 16 bit values, by open addressing: what the writer looks up for
//every field, which a sorted map made slow for big declarations.
class CCPPSyntaxBlockIndex
{
protected:
	uint64*		mauiKeys;
	uint16*		mauiValues;
	size		muiCapacity;	//A power of 2, or 0.
	size		muiUsed;

public:
	void	Init(void);
	void	Kill(void);
	uint16*	Get(uint64 uiKey);
	void	Put(uint64 uiKey, uint16 uiValue);

protected:
	size	Slot(uint64 uiKey);
	void	Grow(void);
};


//A declaration being written as a block: its nodes, scopes, symbols and externals, each given its index as found.
class CCPPSyntaxBlockWriting
{
public:
	CCPPSyntaxTree*				mpcTree;
	CCPPScope*					mpcGlobal;
	int							miFirstToken;
	int							miTokens;
	CArraySyntax				mapcNodes;
	CCPPSyntaxBlockIndex		mcNodes;		//By address: its index.
	CArrayScopes				mapcScopes;		//The block's own.
	CArraySymbols				mapcSymbols;	//The block's own.
	CCPPSyntaxBlockIndex		mcScopes;		//By address: a reference (the block's own index, or an external's).
	CCPPSyntaxBlockIndex		mcSymbols;
	CCPPSyntaxBlockIndex		mcOwnScopes;	//The scopes the block's nodes have: the block's own.
	CArraySyntaxBlockExternals	masExternals;
	CArraySyntaxBlockStrings	masStrings;
	CCPPSyntaxBlockIndex		mcNames;		//By name + 1: its string's index.
	CCPPSyntaxBlockIndex		mcFiles;		//By the file's address: its name's string's index.
	CArraySyntaxBlockDeclared	masDeclared;	//What it put in scopes outside it (put there again when read).
	ECPPSyntaxBlock				meResult;
	CArrayUint8*				mpauiDest;

	void	Init(CCPPSyntaxTree* pcTree, CCPPScope* pcGlobal, CArrayUint8* pauiDest);
	void	Kill(void);

	void	CollectNodes(CCPPSyntax* pcDeclaration);
	uint16	NodeIndex(CCPPSyntax* pcNode);
	bool	IsOwn(CCPPScope* pcScope);
	bool	IsOwn(CCPPSymbol* pcSymbol);
	bool	IsDeclared(CCPPSymbol* pcSymbol);
	CCPPSymbol*	TemplateClass(CCPPScope* pcScope);
	uint16	String(const char* sz, int iLength);
	uint16	Name(int64 lliName);
	uint16	File(CSourceFile* pcFile);
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
//the scopes that are the block's own: those its nodes made (not those they are in, as a lazy declaration's).
class CCPPSyntaxBlockReferences : public CCPPSyntaxFields
{
public:
	CCPPSyntaxBlockWriting*	mpcWriting;
	bool					mbOwnScopes;

	void	SymbolField(CCPPSymbol** ppcSymbol) override	{ if (!mbOwnScopes) { mpcWriting->Symbol(*ppcSymbol); } }
	void	ScopeField(CCPPScope** ppcScope, bool bMade) override
	{
		if (mbOwnScopes)
		{
			if (*ppcScope && bMade)
			{
				mpcWriting->mcOwnScopes.Put((uint64)(ptr)*ppcScope, 1);
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
	void	ScopeField(CCPPScope** ppcScope, bool bMade) override	{ mpcWriting->U16(mpcWriting->Scope(*ppcScope)); }
	void	TokenField(int32* piToken) override				{ mpcWriting->U16(mpcWriting->Token(*piToken)); }
	void	ValueField(void* pvValue, size uiSize) override	{ mpcWriting->Bytes(pvValue, uiSize); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockIndex::Init(void)
{
	mauiKeys = NULL;
	mauiValues = NULL;
	muiCapacity = 0;
	muiUsed = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockIndex::Kill(void)
{
	free(mauiKeys);
	free(mauiValues);
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
// Where uiKey is, or the empty slot it would go in (the capacity isn't 0).
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxBlockIndex::Slot(uint64 uiKey)
{
	size	uiSlot;

	uiSlot = (size)((uiKey * 0x9E3779B97F4A7C15ULL) >> 32) & (muiCapacity - 1);
	while ((mauiKeys[uiSlot] != 0) && (mauiKeys[uiSlot] != uiKey))
	{
		uiSlot = (uiSlot + 1) & (muiCapacity - 1);
	}
	return uiSlot;
}


//////////////////////////////////////////////////////////////////////////
//
// uiKey's value, or NULL if it hasn't one.
//
//////////////////////////////////////////////////////////////////////////
uint16* CCPPSyntaxBlockIndex::Get(uint64 uiKey)
{
	size	uiSlot;

	if (muiCapacity == 0)
	{
		return NULL;
	}
	uiSlot = Slot(uiKey);
	return (mauiKeys[uiSlot] == uiKey) ? &mauiValues[uiSlot] : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// uiKey's value is uiValue.  The table is kept at most half full.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockIndex::Put(uint64 uiKey, uint16 uiValue)
{
	size	uiSlot;

	if ((muiUsed + 1) * 2 > muiCapacity)
	{
		Grow();
	}
	uiSlot = Slot(uiKey);
	if (mauiKeys[uiSlot] == 0)
	{
		mauiKeys[uiSlot] = uiKey;
		muiUsed++;
	}
	mauiValues[uiSlot] = uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Twice the capacity (64 at first), the keys put again.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockIndex::Grow(void)
{
	uint64*		auiKeys;
	uint16*		auiValues;
	size		uiCapacity;
	size		uiSlot;
	size		i;

	auiKeys = mauiKeys;
	auiValues = mauiValues;
	uiCapacity = muiCapacity;
	muiCapacity = (uiCapacity == 0) ? 64 : uiCapacity * 2;
	mauiKeys = (uint64*)calloc(muiCapacity, sizeof(uint64));
	mauiValues = (uint16*)malloc(muiCapacity * sizeof(uint16));
	for (i = 0; i < uiCapacity; i++)
	{
		if (auiKeys[i] != 0)
		{
			uiSlot = Slot(auiKeys[i]);
			mauiKeys[uiSlot] = auiKeys[i];
			mauiValues[uiSlot] = auiValues[i];
		}
	}
	free(auiKeys);
	free(auiValues);
}


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
	mcNodes.Init();
	mapcScopes.Init();
	mapcSymbols.Init();
	mcScopes.Init();
	mcSymbols.Init();
	mcOwnScopes.Init();
	masExternals.Init();
	masStrings.Init();
	mcNames.Init();
	mcFiles.Init();
	masDeclared.Init();
	meResult = CPPSB_Written;
	mpauiDest = pauiDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxBlockWriting::Kill(void)
{
	masDeclared.Kill();
	mcFiles.Kill();
	mcNames.Kill();
	masStrings.Kill();
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
		if (mcNodes.Get((uint64)(ptr)pcNode))
		{
			continue;
		}
		if (mapcNodes.NumElements() >= CPP_SYNTAX_BLOCK_MOST)
		{
			meResult = CPPSB_TooBig;
			break;
		}
		uiIndex = (uint16)mapcNodes.NumElements();
		mcNodes.Put((uint64)(ptr)pcNode, uiIndex);
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
	puiIndex = mcNodes.Get((uint64)(ptr)pcNode);
	return puiIndex ? *puiIndex : CPP_SYNTAX_BLOCK_OUTSIDE;
}


//////////////////////////////////////////////////////////////////////////
//
// A new string's index.
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::String(const char* sz, int iLength)
{
	SCPPSyntaxBlockString*	psString;
	uint16					uiIndex;

	uiIndex = Add((uint16)masStrings.NumElements());
	psString = masStrings.Add();
	psString->sz = sz;
	psString->iLength = iLength;
	return uiIndex;
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
	char*	szName;
	int		iLength;

	szName = (lliName >= 0) ? mpcTree->GetTokeniser()->GetIdentifier(lliName, &iLength) : NULL;
	if (!szName)
	{
		return CPP_SYNTAX_BLOCK_NULL;
	}
	puiIndex = mcNames.Get((uint64)lliName + 1);
	if (puiIndex)
	{
		return *puiIndex;
	}
	uiIndex = String(szName, iLength);
	mcNames.Put((uint64)lliName + 1, uiIndex);
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of a file's full name's string.
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::File(CSourceFile* pcFile)
{
	uint16*	puiIndex;
	uint16	uiIndex;
	char*	szName;

	puiIndex = mcFiles.Get((uint64)(ptr)pcFile);
	if (puiIndex)
	{
		return *puiIndex;
	}
	szName = pcFile->FullName();
	uiIndex = String(szName, (int)strlen(szName));
	mcFiles.Put((uint64)(ptr)pcFile, uiIndex);
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
	if (mcOwnScopes.Get((uint64)(ptr)pcScope))
	{
		return true;
	}
	return pcScope->mpcOwner && (pcScope->mpcOwner->mpcMembers == pcScope) && pcScope->mpcOwner->mpcDeclaration && mcNodes.Get((uint64)(ptr)pcScope->mpcOwner->mpcDeclaration);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the symbol is the block's own: one its nodes declare, or one in
// its own scope.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxBlockWriting::IsOwn(CCPPSymbol* pcSymbol)
{
	if (pcSymbol->mpcDeclaration && mcNodes.Get((uint64)(ptr)pcSymbol->mpcDeclaration))
	{
		return true;
	}
	return pcSymbol->mpcScope && IsOwn(pcSymbol->mpcScope);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the block's own symbol is declared in a scope outside it: its
// place among the scope's symbols of its name isn't the block's (it is
// added to the scope when the block is read, as parsing would).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxBlockWriting::IsDeclared(CCPPSymbol* pcSymbol)
{
	return pcSymbol->mpcScope && !IsOwn(pcSymbol->mpcScope);
}


//////////////////////////////////////////////////////////////////////////
//
// The class template whose parameters pcScope is (its members' parent), or
// NULL: the template declaration that made the scope, found from one of the
// scope's parameters, declares the class.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPSyntaxBlockWriting::TemplateClass(CCPPScope* pcScope)
{
	SMapIterator					sIterator;
	CCPPSymbol**					ppcSymbol;
	CCPPSyntax*						pcNode;
	CCPPSyntaxTemplateDeclaration*	pcTemplate;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntax*						pcType;
	CCPPSymbol*						pcClass;

	if ((pcScope->meKind != CPPSC_Template) || !pcScope->mcSymbols.StartIteration(&sIterator, NULL, NULL, (void**)&ppcSymbol, NULL) || !*ppcSymbol)
	{
		return NULL;
	}
	for (pcNode = (*ppcSymbol)->mpcDeclaration; pcNode; pcNode = pcNode->mpcParent)
	{
		if ((pcNode->GetKind() == CPPS_TemplateDeclaration) && (((CCPPSyntaxTemplateDeclaration*)pcNode)->mpcScope == pcScope))
		{
			break;
		}
	}
	pcTemplate = (CCPPSyntaxTemplateDeclaration*)pcNode;
	if (!pcTemplate || !pcTemplate->mpcDeclaration || (pcTemplate->mpcDeclaration->GetKind() != CPPS_SimpleDeclaration))
	{
		return NULL;
	}
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcTemplate->mpcDeclaration;
	pcType = pcSimple->mpcSpecifiers ? pcSimple->mpcSpecifiers->mpcType : NULL;
	if (!pcType || (pcType->GetKind() != CPPS_ClassSpecifier))
	{
		return NULL;
	}
	pcClass = ((CCPPSyntaxClassSpecifier*)pcType)->mpcSymbol;
	return (pcClass && pcClass->mpcMembers && (pcClass->mpcMembers->mpcParent == pcScope)) ? pcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The reference of pcScope: the block's own scope (IsOwn), an external's
// (the global scope, the members of a symbol, or a class template's
// parameters), or CPP_SYNTAX_BLOCK_NULL.  The block's own scope's parent,
// owner, bases, usings, qualifier and names get references too.
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::Scope(CCPPScope* pcScope)
{
	uint16*						puiReference;
	uint16						uiReference;
	SCPPSyntaxBlockExternal		sExternal;
	SMapIterator				sIterator;
	CCPPSymbol**				ppcSymbol;
	CCPPSymbol*					pcClass;
	size						i;
	bool						bMore;

	if (!pcScope)
	{
		return CPP_SYNTAX_BLOCK_NULL;
	}
	puiReference = mcScopes.Get((uint64)(ptr)pcScope);
	if (puiReference)
	{
		return *puiReference;
	}

	if (IsOwn(pcScope))
	{
		uiReference = Add((uint16)mapcScopes.NumElements());
		mcScopes.Put((uint64)(ptr)pcScope, uiReference);
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
	else if ((pcClass = TemplateClass(pcScope)) != NULL)
	{
		sExternal.uiKind = CPP_SYNTAX_BLOCK_PARENT;
		sExternal.uiScope = Scope(pcClass->mpcMembers);
	}
	else
	{
		meResult = CPPSB_UnnamedScope;
		return CPP_SYNTAX_BLOCK_NULL;
	}
	uiReference = Add((uint16)masExternals.NumElements()) | CPP_SYNTAX_BLOCK_EXTERNAL;
	mcScopes.Put((uint64)(ptr)pcScope, uiReference);
	masExternals.Add(&sExternal);
	return uiReference;
}


//////////////////////////////////////////////////////////////////////////
//
// The reference of pcSymbol: the block's own symbol (IsOwn), an external's
// (found by its name in its scope and where its declaration is), or CPP_
// SYNTAX_BLOCK_NULL.  The block's own symbol's scopes, and next if it is
// in the block's own scope, get references too.
//
//////////////////////////////////////////////////////////////////////////
uint16 CCPPSyntaxBlockWriting::Symbol(CCPPSymbol* pcSymbol)
{
	uint16*						puiReference;
	uint16						uiReference;
	SCPPSyntaxBlockExternal		sExternal;
	CCPPSymbol**				ppcFirst;
	CCPPSymbol*					pcOther;
	CSourceFile*				pcFile;
	CSourceFile*				pcOtherFile;
	int							iLine;
	int							iColumn;
	int							iOtherLine;
	int							iOtherColumn;
	bool						bPosition;
	bool						bOtherPosition;

	if (!pcSymbol)
	{
		return CPP_SYNTAX_BLOCK_NULL;
	}
	puiReference = mcSymbols.Get((uint64)(ptr)pcSymbol);
	if (puiReference)
	{
		return *puiReference;
	}

	if (IsOwn(pcSymbol))
	{
		uiReference = Add((uint16)mapcSymbols.NumElements());
		mcSymbols.Put((uint64)(ptr)pcSymbol, uiReference);
		mapcSymbols.Add(pcSymbol);
		Name(pcSymbol->mlliName);
		Scope(pcSymbol->mpcScope);
		Scope(pcSymbol->mpcMembers);
		if (!IsDeclared(pcSymbol))
		{
			Symbol(pcSymbol->mpcNext);
		}
		return uiReference;
	}

	//The first parsed symbol of its name in its scope declared where it is: in a unit with more or fewer declarations
	//of the name, or lazy ones, it is still found.
	memset(&sExternal, 0, sizeof(SCPPSyntaxBlockExternal));
	sExternal.uiKind = CPP_SYNTAX_BLOCK_SYMBOL;
	sExternal.uiScope = Scope(pcSymbol->mpcScope);
	sExternal.uiName = Name(pcSymbol->mlliName);
	bPosition = CPPSyntaxBlockPosition(mpcTree, pcSymbol, &pcFile, &iLine, &iColumn);
	sExternal.uiFile = bPosition ? File(pcFile) : CPP_SYNTAX_BLOCK_NULL;
	sExternal.uiLine = bPosition ? (uint32)iLine : CPP_SYNTAX_BLOCK_NO_LINE;
	sExternal.uiColumn = bPosition ? (uint16)iColumn : CPP_SYNTAX_BLOCK_NULL;
	ppcFirst = pcSymbol->mpcScope ? pcSymbol->mpcScope->mcSymbols.Get(pcSymbol->mlliName) : NULL;
	for (pcOther = ppcFirst ? *ppcFirst : NULL; pcOther; pcOther = pcOther->mpcNext)
	{
		if (pcOther->mpcLazy)
		{
			continue;
		}
		bOtherPosition = CPPSyntaxBlockPosition(mpcTree, pcOther, &pcOtherFile, &iOtherLine, &iOtherColumn);
		if ((bOtherPosition == bPosition) && (!bPosition || ((pcOtherFile == pcFile) && (iOtherLine == iLine) && (iOtherColumn == iColumn))))
		{
			break;
		}
	}
	if ((pcOther != pcSymbol) || (bPosition && (iColumn > CPP_SYNTAX_BLOCK_MOST)) ||(sExternal.uiScope == CPP_SYNTAX_BLOCK_NULL) || (sExternal.uiName == CPP_SYNTAX_BLOCK_NULL))
	{
		if (meResult == CPPSB_Written)
		{
			meResult = CPPSB_UnnamedSymbol;
		}
		return CPP_SYNTAX_BLOCK_NULL;
	}
	uiReference = Add((uint16)masExternals.NumElements()) | CPP_SYNTAX_BLOCK_EXTERNAL;
	mcSymbols.Put((uint64)(ptr)pcSymbol, uiReference);
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
	SCPPSyntaxBlockString*		psString;
	SCPPSyntaxBlockDeclared*	psDeclared;
	size						uiStart;
	size						uiEntries;
	size						uiCount;
	size						i;
	size						j;
	bool						bMore;

	U32(CPP_SYNTAX_BLOCK_MAGIC);
	U16((uint16)mapcNodes.NumElements());
	U16((uint16)masStrings.NumElements());
	U16((uint16)masExternals.NumElements());
	U16((uint16)mapcScopes.NumElements());
	U16((uint16)mapcSymbols.NumElements());
	U16((uint16)miTokens);

	for (i = 0; i < mapcNodes.NumElements(); i++)
	{
		U8((uint8)mapcNodes.GetPtr(i)->GetKind());
	}

	for (i = 0; i < masStrings.NumElements(); i++)
	{
		psString = masStrings.Get(i);
		U16((uint16)psString->iLength);
		Bytes((void*)psString->sz, psString->iLength);
	}

	for (i = 0; i < masExternals.NumElements(); i++)
	{
		psExternal = masExternals.Get(i);
		U8(psExternal->uiKind);
		if (psExternal->uiKind == CPP_SYNTAX_BLOCK_MEMBERS)
		{
			U16(psExternal->uiSymbol);
		}
		else if (psExternal->uiKind == CPP_SYNTAX_BLOCK_PARENT)
		{
			U16(psExternal->uiScope);
		}
		else if (psExternal->uiKind == CPP_SYNTAX_BLOCK_SYMBOL)
		{
			U16(psExternal->uiScope);
			U16(psExternal->uiName);
			U16(psExternal->uiFile);
			U32(psExternal->uiLine);
			U16(psExternal->uiColumn);
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
		U16(IsDeclared(pcSymbol) ? CPP_SYNTAX_BLOCK_NULL : Symbol(pcSymbol->mpcNext));
		U16(NodeIndex(pcSymbol->mpcLazy));
	}

	U16(Add((uint16)masDeclared.NumElements()));
	for (i = 0; i < masDeclared.NumElements(); i++)
	{
		psDeclared = masDeclared.Get(i);
		U8(psDeclared->bAlias ? 1 : 0);
		U16(Scope(psDeclared->pcScope));
		U16(Name(psDeclared->lliName));
		U16(Symbol(psDeclared->pcSymbol));
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
// be (otherwise pauiDest is as it was).  What it put in scopes outside it
// is found from their names: its own symbols declared in them, and the
// names of its own scopes their parents outside it have too (aliases);
// or is pasDeclared, if it is given (as a block read with it gives them).
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntaxBlock CCPPSyntaxBlockWriter::Write(CCPPSyntaxTree* pcTree, CCPPSyntax* pcDeclaration, CCPPScope* pcGlobal, CArrayUint8* pauiDest, CArraySyntaxBlockDeclared* pasDeclared)
{
	CCPPSyntaxBlockWriting		cWriting;
	CCPPSyntaxBlockReferences	cReferences;
	SCPPTokenSymbol*			psTokenSymbol;
	ECPPSyntaxBlock				eResult;
	CCPPSymbol*					pcSymbol;
	CCPPSymbol**				ppcFirst;
	CCPPSymbol*					pcOther;
	CCPPScope*					pcScope;
	SCPPSyntaxBlockDeclared*	psDeclared;
	SMapIterator				sIterator;
	CCPPSymbol**				ppcSymbol;
	int64*						plliName;
	size						uiStart;
	size						i;
	bool						bMore;

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

	//What the declaration put in scopes outside it: its own symbols declared there, then the names of its own scopes
	//found from their parents outside it (an unscoped enum's enumerators), in the order they were found.
	if (pasDeclared)
	{
		cWriting.masDeclared.Copy(pasDeclared);
	}
	for (i = 0; !pasDeclared && (cWriting.meResult == CPPSB_Written) && (i < cWriting.mapcSymbols.NumElements()); i++)
	{
		pcSymbol = cWriting.mapcSymbols.GetPtr(i);
		if (!cWriting.IsDeclared(pcSymbol))
		{
			continue;
		}
		ppcFirst = pcSymbol->mpcScope->mcSymbols.Get(pcSymbol->mlliName);
		for (pcOther = ppcFirst ? *ppcFirst : NULL; pcOther && (pcOther != pcSymbol); pcOther = pcOther->mpcNext)
		{
		}
		if (pcOther)
		{
			psDeclared = cWriting.masDeclared.Add();
			psDeclared->pcScope = pcSymbol->mpcScope;
			psDeclared->lliName = pcSymbol->mlliName;
			psDeclared->pcSymbol = pcSymbol;
			psDeclared->bAlias = false;
		}
	}
	for (i = 0; !pasDeclared && (cWriting.meResult == CPPSB_Written) && (i < cWriting.mapcScopes.NumElements()); i++)
	{
		pcScope = cWriting.mapcScopes.GetPtr(i);
		if (!pcScope->mpcParent || cWriting.IsOwn(pcScope->mpcParent))
		{
			continue;
		}
		for (bMore = pcScope->mcSymbols.StartIteration(&sIterator, (void**)&plliName, NULL, (void**)&ppcSymbol, NULL); bMore; bMore = pcScope->mcSymbols.Iterate(&sIterator, (void**)&plliName, NULL, (void**)&ppcSymbol, NULL))
		{
			ppcFirst = pcScope->mpcParent->mcSymbols.Get(*plliName);
			if (ppcFirst && (*ppcFirst == *ppcSymbol))
			{
				psDeclared = cWriting.masDeclared.Add();
				psDeclared->pcScope = pcScope->mpcParent;
				psDeclared->lliName = *plliName;
				psDeclared->pcSymbol = *ppcSymbol;
				psDeclared->bAlias = true;
			}
		}
	}
	//Their scopes, names and symbols have references before the bytes are written.
	for (i = 0; (cWriting.meResult == CPPSB_Written) && (i < cWriting.masDeclared.NumElements()); i++)
	{
		psDeclared = cWriting.masDeclared.Get(i);
		cWriting.Scope(psDeclared->pcScope);
		cWriting.Name(psDeclared->lliName);
		cWriting.Symbol(psDeclared->pcSymbol);
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
	CArraySyntax				mapcNodes;
	CArraySyntaxBlockStrings	masStrings;
	CArrayTemplate<int64>		malliNames;		//By string: its identifier, -2 until it is a name.
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
	bool			IsFile(uint16 uiIndex, CSourceFile* pcFile);
	CCPPSymbol*		FindExternal(CCPPScope* pcScope, int64 lliName, uint16 uiFile, uint32 uiLine, uint16 uiColumn);
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
	void	ScopeField(CCPPScope** ppcScope, bool bMade) override	{ *ppcScope = mpcReading->Scope(mpcReading->U16()); }
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
	masStrings.Init();
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
	masStrings.Kill();
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
	SCPPSyntaxBlockString*	psString;
	int64*					plliName;
	CChars					szName;

	if (uiIndex == CPP_SYNTAX_BLOCK_NULL)
	{
		return -1;
	}
	if (uiIndex >= masStrings.NumElements())
	{
		mbError = true;
		return -1;
	}
	plliName = malliNames.Get(uiIndex);
	if (*plliName == -2)
	{
		psString = masStrings.Get(uiIndex);
		szName.Init(psString->sz, 0, psString->iLength);
		*plliName = mpcTree->GetTokeniser()->GetIdentifierID(szName.Text());
		szName.Kill();
	}
	return *plliName;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the string is pcFile's full name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxBlockReading::IsFile(uint16 uiIndex, CSourceFile* pcFile)
{
	SCPPSyntaxBlockString*	psString;
	char*					szName;

	if (uiIndex >= masStrings.NumElements())
	{
		return false;
	}
	psString = masStrings.Get(uiIndex);
	szName = pcFile->FullName();
	return ((int)strlen(szName) == psString->iLength) && (memcmp(szName, psString->sz, psString->iLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The first parsed symbol of lliName in pcScope declared at the place (or
// with no declaration if uiLine is CPP_SYNTAX_BLOCK_NO_LINE).  The name's
// lazy declarations there are parsed first (CCPPScope::FindHere), as
// looking it up while parsing would.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPSyntaxBlockReading::FindExternal(CCPPScope* pcScope, int64 lliName, uint16 uiFile, uint32 uiLine, uint16 uiColumn)
{
	CCPPSymbol*		pcSymbol;
	CSourceFile*	pcFile;
	int				iLine;
	int				iColumn;
	bool			bPosition;

	if (!pcScope || (lliName < 0))
	{
		return NULL;
	}
	for (pcSymbol = pcScope->FindHere(lliName); pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		if (pcSymbol->mpcLazy)
		{
			continue;
		}
		bPosition = CPPSyntaxBlockPosition(mpcTree, pcSymbol, &pcFile, &iLine, &iColumn);
		if (uiLine == CPP_SYNTAX_BLOCK_NO_LINE)
		{
			if (!bPosition)
			{
				return pcSymbol;
			}
		}
		else if (bPosition && ((uint32)iLine == uiLine) && (iColumn == uiColumn) && IsFile(uiFile, pcFile))
		{
			return pcSymbol;
		}
	}
	return NULL;
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
// symbols declared in scopes outside it are added to them (as parsing would
// declare them), and the aliases it made are made again; or if pasDeclared
// is given, they are only given to it.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxBlockReader::Read(uint8* puiBlock, size uiLength, CCPPSyntaxTree* pcTree, int iFirstToken, CCPPScope* pcGlobal, CCPPSyntax* pcParent, CCPPLazyLoader* pcLoader, CArraySyntaxBlockDeclared* pasDeclared)
{
	CCPPSyntaxBlockReading		cReading;
	CCPPSyntaxBlockFieldReader	cFields;
	CCPPSymbolTable*			pcSymbols;
	CCPPScope*					pcScope;
	CCPPSymbol*					pcSymbol;
	CCPPSyntax*					pcNode;
	CCPPSyntax*					pcDeclaration;
	SCPPSyntaxBlockString*		psString;
	CArraySyntaxBlockDeclared	asDeclared;
	SCPPSyntaxBlockDeclared*	psDeclared;
	uint16						uiNodes;
	uint16						uiNames;
	uint16						uiExternals;
	uint16						uiScopes;
	uint16						uiSymbols;
	uint16						uiCount;
	uint16						uiName;
	uint16						uiFile;
	uint32						uiLine;
	uint16						uiColumn;
	uint16						uiParent;
	uint8						uiKind;
	uint8						uiRole;
	int64						lliName;
	size						i;
	size						j;

	cReading.Init(puiBlock, uiLength, pcTree, iFirstToken);
	asDeclared.Init();
	pcSymbols = pcTree->GetSymbols();
	if (cReading.U32() != CPP_SYNTAX_BLOCK_MAGIC)
	{
		asDeclared.Kill();
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

	//The strings, each made an identifier of the tree's tokeniser when it is used as a name.
	lliName = -2;
	for (i = 0; (i < uiNames) && !cReading.mbError; i++)
	{
		uiCount = cReading.U16();
		if (cReading.muiPosition + uiCount > uiLength)
		{
			cReading.mbError = true;
			break;
		}
		psString = cReading.masStrings.Add();
		psString->sz = (char*)&puiBlock[cReading.muiPosition];
		psString->iLength = uiCount;
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
		else if (uiKind == CPP_SYNTAX_BLOCK_PARENT)
		{
			pcScope = cReading.Scope(cReading.U16());
			pcScope = pcScope ? pcScope->mpcParent : NULL;
		}
		else if (uiKind == CPP_SYNTAX_BLOCK_SYMBOL)
		{
			pcScope = cReading.Scope(cReading.U16());
			uiName = cReading.U16();
			uiFile = cReading.U16();
			uiLine = cReading.U32();
			uiColumn = cReading.U16();
			pcSymbol = cReading.FindExternal(pcScope, cReading.Name(uiName), uiFile, uiLine, uiColumn);
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

	//What it put in scopes outside it, put there again (or given) once the block is read.
	uiCount = cReading.U16();
	for (i = 0; (i < uiCount) && !cReading.mbError; i++)
	{
		psDeclared = asDeclared.Add();
		psDeclared->bAlias = cReading.U8() != 0;
		psDeclared->pcScope = cReading.Scope(cReading.U16());
		psDeclared->lliName = cReading.Name(cReading.U16());
		psDeclared->pcSymbol = cReading.Symbol(cReading.U16());
		cReading.mbError |= !psDeclared->pcScope || !psDeclared->pcSymbol;
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
	for (i = 0; pcDeclaration && (i < asDeclared.NumElements()); i++)
	{
		psDeclared = asDeclared.Get(i);
		if (pasDeclared)
		{
			pasDeclared->Add(psDeclared);
		}
		else if (psDeclared->bAlias)
		{
			psDeclared->pcScope->AddAlias(psDeclared->lliName, psDeclared->pcSymbol);
		}
		else
		{
			psDeclared->pcSymbol->mpcNext = NULL;
			psDeclared->pcScope->Add(psDeclared->pcSymbol);
		}
	}
	asDeclared.Kill();
	cReading.Kill();
	return pcDeclaration;
}

