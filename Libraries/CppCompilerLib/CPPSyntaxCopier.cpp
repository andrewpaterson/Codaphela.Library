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
#include "CPPSyntaxCopier.h"


//One of a node's fields, as VisitFields gives it.
enum ECPPCopiedField
{
	CPPCF_Node,
	CPPCF_Nodes,
	CPPCF_Link,
	CPPCF_Symbol,
	CPPCF_Scope,
	CPPCF_Token,
	CPPCF_Value,
};


struct SCPPCopiedField
{
	ECPPCopiedField	eKind;
	void*			pvField;
	size			uiSize;
};


typedef CArrayTemplate<SCPPCopiedField>	CArrayCPPCopiedFields;


//The fields of the node copied, in order.
class CCPPSyntaxFieldReader : public CCPPSyntaxFields
{
public:
	CArrayCPPCopiedFields*	mpasFields;

	void	Add(ECPPCopiedField eKind, void* pvField, size uiSize)
	{
		SCPPCopiedField*	psField;

		psField = mpasFields->Add();
		psField->eKind = eKind;
		psField->pvField = pvField;
		psField->uiSize = uiSize;
	}

	void	NodeField(CCPPSyntax** ppcNode) override					{ Add(CPPCF_Node, ppcNode, 0); }
	void	NodesField(CArraySyntax* papcNodes) override				{ Add(CPPCF_Nodes, papcNodes, 0); }
	void	LinkField(CCPPSyntax** ppcNode) override					{ Add(CPPCF_Link, ppcNode, 0); }
	void	SymbolField(CCPPSymbol** ppcSymbol) override				{ Add(CPPCF_Symbol, ppcSymbol, 0); }
	void	ScopeField(CCPPScope** ppcScope, bool bMade) override		{ Add(CPPCF_Scope, ppcScope, 0); }
	void	TokenField(int32* piToken) override							{ Add(CPPCF_Token, piToken, 0); }
	void	ValueField(void* pvValue, size uiSize) override				{ Add(CPPCF_Value, pvValue, uiSize); }
};


//The copy's fields, each given the field read from the node copied (in the same order, as the copy is of the same
//kind), its children copied.
class CCPPSyntaxFieldWriter : public CCPPSyntaxFields
{
public:
	CCPPSyntaxCopier*		mpcCopier;
	CArrayCPPCopiedFields*	mpasFields;
	CCPPSyntax*				mpcCopy;
	size					muiField;

	void*	Next(void)
	{
		return mpasFields->Get(muiField++)->pvField;
	}

	void	NodeField(CCPPSyntax** ppcNode) override
	{
		CCPPSyntax*	pcChild;

		pcChild = *(CCPPSyntax**)Next();
		*ppcNode = pcChild ? mpcCopier->Copy(pcChild, mpcCopy) : NULL;
	}

	void	NodesField(CArraySyntax* papcNodes) override
	{
		CArraySyntax*	papcFrom;
		CCPPSyntax*		pcChild;
		size			i;

		papcFrom = (CArraySyntax*)Next();
		for (i = 0; i < papcFrom->NumElements(); i++)
		{
			pcChild = papcFrom->GetPtr(i);
			papcNodes->Add(pcChild ? mpcCopier->Copy(pcChild, mpcCopy) : NULL);
		}
	}

	void	LinkField(CCPPSyntax** ppcNode) override					{ *ppcNode = *(CCPPSyntax**)Next(); }
	void	SymbolField(CCPPSymbol** ppcSymbol) override				{ *ppcSymbol = mpcCopier->Substituted(*(CCPPSymbol**)Next()); }
	void	ScopeField(CCPPScope** ppcScope, bool bMade) override		{ *ppcScope = *(CCPPScope**)Next(); }
	void	TokenField(int32* piToken) override							{ *piToken = *(int32*)Next(); }
	void	ValueField(void* pvValue, size uiSize) override				{ memcpy(pvValue, Next(), uiSize); }
};


//////////////////////////////////////////////////////////////////////////
//
// Copies into pcMemory.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCopier::Init(CCPPSyntaxMemory* pcMemory)
{
	mpcMemory = pcMemory;
	mapcFrom.Init();
	mapcTo.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCopier::Kill(void)
{
	mapcTo.Kill();
	mapcFrom.Kill();
	mpcMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The copy names pcTo where the tree copied names pcFrom.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCopier::Substitute(CCPPSymbol* pcFrom, CCPPSymbol* pcTo)
{
	mapcFrom.Add(pcFrom);
	mapcTo.Add(pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol the copy names for pcSymbol.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPSyntaxCopier::Substituted(CCPPSymbol* pcSymbol)
{
	size	i;

	for (i = 0; pcSymbol && (i < mapcFrom.NumElements()); i++)
	{
		if (mapcFrom.GetPtr(i) == pcSymbol)
		{
			return mapcTo.GetPtr(i);
		}
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of pcNode and its children, pcParent's child, with the same
// tokens.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxCopier::Copy(CCPPSyntax* pcNode, CCPPSyntax* pcParent)
{
	CArrayCPPCopiedFields	asFields;
	CCPPSyntaxFieldReader	cReader;
	CCPPSyntaxFieldWriter	cWriter;
	CCPPSyntax*				pcCopy;

	pcCopy = mpcMemory->CreateKind(pcNode->GetKind(), pcParent);
	if (!pcCopy)
	{
		return NULL;
	}
	pcCopy->SetTokens(pcNode->miFirstToken, pcNode->miLastToken);

	asFields.Init();
	cReader.mpasFields = &asFields;
	pcNode->VisitFields(&cReader);
	cWriter.mpcCopier = this;
	cWriter.mpasFields = &asFields;
	cWriter.mpcCopy = pcCopy;
	cWriter.muiField = 0;
	pcCopy->VisitFields(&cWriter);
	asFields.Kill();
	return pcCopy;
}
