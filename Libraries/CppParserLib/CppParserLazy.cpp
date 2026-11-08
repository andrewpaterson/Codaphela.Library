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
#include "CppParser.h"
#include "CPPSyntaxes.h"
#include "CPPSyntaxBlock.h"
#include "SourceFile.h"


//The most variants of a header declaration (a header included with other macros) tried at one place; past them the
//declaration is parsed, as it is the first time it is seen.
#define CPP_PARSER_LAZY_VARIANTS	16


//////////////////////////////////////////////////////////////////////////
//
// Header declarations the index has seen are left unparsed, their names
// declared as lazy symbols, until one is looked up (CCPPScope::LoadLazy).
// NULL parses everything.
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::SetHeaderIndex(CCPPHeaderIndex* pcIndex)
{
	mpcHeaderIndex = pcIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPParser::NumLazySkipped(void)
{
	return muiLazySkipped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPParser::NumLazyLoaded(void)
{
	return muiLazyLoaded;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the lazy declarations used were read as parsed before (their
// blocks), not parsed.
//
//////////////////////////////////////////////////////////////////////////
size CCPPParser::NumLazyRead(void)
{
	return muiLazyRead;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the tokens iFirst to iLast (inclusive) are a header's own (tokens
// made by macros are allowed after the first), and a hash of their text.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::HeaderTokens(int iFirst, int iLast, CSourceFile** ppcFile, uint64* puiHash)
{
	SPreprocessedToken*	psSource;
	CSourceFile*		pcFile;
	uint64				uiHash;
	int					i;
	size				j;

	pcFile = NULL;
	uiHash = 1469598103934665603ULL;
	for (i = iFirst; i <= iLast; i++)
	{
		psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(i));
		if (!psSource)
		{
			return false;
		}
		if (psSource->iExpansion < 0)
		{
			if (!psSource->pcFile)
			{
				return false;
			}
			if (!pcFile)
			{
				pcFile = psSource->pcFile;
				if (pcFile->IsTranslationUnit())
				{
					return false;
				}
			}
			else if (psSource->pcFile != pcFile)
			{
				return false;
			}
		}
		else if (i == iFirst)
		{
			return false;
		}
		for (j = 0; j < psSource->uiLength; j++)
		{
			uiHash = (uiHash ^ (uint8)psSource->szText[j]) * 1099511628211ULL;
		}
		uiHash = (uiHash ^ ' ') * 1099511628211ULL;
	}
	*ppcFile = pcFile;
	*puiHash = uiHash;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A namespace declaration of a header that names a class, enum, typedef,
// template or function (and declares no variable), or defines a member
// function outside its class, can be left unparsed.
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::RecordHeaderDeclaration(int iFirst, int iLast, CCPPSyntaxDeclaration* pcDeclaration, CArrayTemplate<CCPPSymbol*>* papcDeclared)
{
	SPreprocessedToken*		psSource;
	CSourceFile*			pcFile;
	uint64					uiHash;
	uint64					uiKey;
	SCPPHeaderDeclaration*	psRecord;
	CCPPSymbol*				pcSymbol;
	CCPPSymbol*				pcClass;
	ECPPSyntax				eKind;
	bool					bLazy;
	char*					szName;
	size					i;

	if ((iLast < iFirst) || !HeaderTokens(iFirst, iLast, &pcFile, &uiHash))
	{
		return;
	}
	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(iFirst));
	uiKey = CCPPHeaderIndex::Key(pcFile->FullName(), psSource->iLine, psSource->iColumn);
	mpcHeaderIndex->Lock();
	for (psRecord = mpcHeaderIndex->Get(uiKey); psRecord; psRecord = psRecord->psNext)
	{
		if ((psRecord->uiTokens == (uint32)(iLast - iFirst + 1)) && (psRecord->uiHash == uiHash))
		{
			break;
		}
	}
	mpcHeaderIndex->Unlock();
	if (psRecord)
	{
		return;
	}

	eKind = pcDeclaration->GetKind();
	bLazy = ((eKind == CPPS_SimpleDeclaration) || (eKind == CPPS_FunctionDefinition) || (eKind == CPPS_TemplateDeclaration) || (eKind == CPPS_AliasDeclaration)) && (papcDeclared->NumElements() > 0);
	for (i = 0; bLazy && (i < papcDeclared->NumElements()); i++)
	{
		pcSymbol = *papcDeclared->Get(i);
		switch (pcSymbol->meKind)
		{
		case CPPSK_Class:
		case CPPSK_Enum:
		case CPPSK_Typedef:
		case CPPSK_ClassTemplate:
		case CPPSK_AliasTemplate:
		case CPPSK_Function:
		case CPPSK_FunctionTemplate:
		case CPPSK_Enumerator:
			break;
		default:
			bLazy = false;
			break;
		}
		if (!mpcTokeniser->GetIdentifier(pcSymbol->mlliName))
		{
			bLazy = false;
		}
		//A name introduced by a friend or a forward declaration (class X;) is the same symbol as its definition
		//elsewhere: the declaration stays parsed so the name is declared where it was.
		if (pcSymbol->mpcDeclaration && (pcSymbol->mpcDeclaration->GetKind() == CPPS_ElaboratedTypeSpecifier))
		{
			bLazy = false;
		}
	}

	//A member function defined outside its class declares no name: it is parsed after the class.
	pcClass = NULL;
	if ((papcDeclared->NumElements() == 0) && ((eKind == CPPS_FunctionDefinition) || (eKind == CPPS_TemplateDeclaration)))
	{
		pcClass = MemberDefinitionClass(pcDeclaration);
		if (pcClass)
		{
			bLazy = true;
		}
	}

	psRecord = mpcHeaderIndex->New();
	psRecord->uiTokens = (uint32)(iLast - iFirst + 1);
	psRecord->uiHash = uiHash;
	psRecord->bLazy = bLazy;
	if (pcClass)
	{
		psRecord->szClass.Append(mpcTokeniser->GetIdentifier(pcClass->mlliName));
	}
	for (i = 0; bLazy && (i < papcDeclared->NumElements()); i++)
	{
		pcSymbol = *papcDeclared->Get(i);
		szName = mpcTokeniser->GetIdentifier(pcSymbol->mlliName);
		psRecord->aszNames.Add(szName);
		psRecord->aiKinds.Add((int)pcSymbol->meKind);
	}

	//A declaration later units leave unparsed is kept as it was parsed here, to be read where one of them uses it.
	if (bLazy)
	{
		CCPPSyntaxBlockWriter::Write(mpcTree, pcDeclaration, mpcSymbols->GetGlobalScope(), &psRecord->auiBlock);
	}
	mpcHeaderIndex->Publish(uiKey, psRecord);
}


//////////////////////////////////////////////////////////////////////////
//
// The declaration at the current token, if the index has seen it (the same
// tokens) and it can be left unparsed: a placeholder takes its place and
// its names are declared as lazy symbols.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::SkipLazy(CArraySyntax* papcDeclarations, CCPPSyntax* pcParent)
{
	SPreprocessedToken*			psSource;
	SCPPHeaderDeclaration*		psRecord;
	CCPPSyntaxLazyDeclaration*	pcLazy;
	CCPPSyntaxLazyDeclaration*	pcClass;
	CCPPSyntaxLazyDeclaration**	ppcMember;
	CCPPSymbol*					pcSymbol;
	CSourceFile*				pcFile;
	SCPPParserPosition			sEnd;
	uint64						uiHash;
	int							iFirst;
	int							iLast;
	size						i;
	SCPPHeaderDeclaration*		apsVariants[CPP_PARSER_LAZY_VARIANTS];
	size						uiVariants;

	iFirst = CurrentToken();
	if (mbHalf || (iFirst >= miNumTokens))
	{
		return false;
	}
	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(iFirst));
	if (!psSource || !psSource->pcFile || psSource->pcFile->IsTranslationUnit() || (psSource->iExpansion >= 0))
	{
		return false;
	}
	//The first variant whose tokens are the ones here.  The variants are taken from the index while it is locked;
	//they don't change once in it.
	mpcHeaderIndex->Lock();
	uiVariants = 0;
	for (psRecord = mpcHeaderIndex->Get(CCPPHeaderIndex::Key(psSource->pcFile->FullName(), psSource->iLine, psSource->iColumn)); psRecord && (uiVariants < CPP_PARSER_LAZY_VARIANTS); psRecord = psRecord->psNext)
	{
		apsVariants[uiVariants] = psRecord;
		uiVariants++;
	}
	mpcHeaderIndex->Unlock();
	psRecord = NULL;
	for (i = 0; i < uiVariants; i++)
	{
		iLast = iFirst + (int)apsVariants[i]->uiTokens - 1;
		if ((iLast < miNumTokens) && HeaderTokens(iFirst, iLast, &pcFile, &uiHash) && (uiHash == apsVariants[i]->uiHash))
		{
			psRecord = apsVariants[i];
			break;
		}
	}
	if (!psRecord || !psRecord->bLazy)
	{
		return false;
	}
	//A class declared here before (class X;) may have been kept by whatever found it, which would not
	//parse the definition when it needs the members.
	for (i = 0; i < psRecord->aszNames.NumElements(); i++)
	{
		if (((psRecord->aiKinds.GetValue(i) == CPPSK_Class) || (psRecord->aiKinds.GetValue(i) == CPPSK_ClassTemplate)) && HasParsedSymbol(psRecord->aszNames.GetText(i)))
		{
			return false;
		}
	}
	pcClass = NULL;
	if (!psRecord->szClass.Empty())
	{
		//Only while its class is unparsed; otherwise it is parsed now, as it would be.
		pcClass = UnloadedClass(psRecord->szClass.Text());
		if (!pcClass)
		{
			return false;
		}
	}

	pcLazy = mpcMemory->Create<CCPPSyntaxLazyDeclaration>(pcParent);
	pcLazy->SetTokens(iFirst, iLast);
	pcLazy->mpapcOwner = papcDeclarations;
	pcLazy->muiIndex = papcDeclarations->NumElements();
	pcLazy->mpsRecord = psRecord;
	pcLazy->mpcScope = mpcScope;
	pcLazy->miPack = miPack;
	papcDeclarations->Add(pcLazy);
	if (pcClass)
	{
		ppcMember = &pcClass->mpcMembers;
		while (*ppcMember)
		{
			ppcMember = &(*ppcMember)->mpcNextMember;
		}
		*ppcMember = pcLazy;
	}
	for (i = 0; i < psRecord->aszNames.NumElements(); i++)
	{
		pcSymbol = mpcSymbols->CreateSymbol(mpcTokeniser->GetIdentifierID(psRecord->aszNames.GetText(i)), (ECPPSymbolKind)psRecord->aiKinds.GetValue(i), pcLazy, mpcScope);
		pcSymbol->mpcLazy = pcLazy;
		pcSymbol->mpcLoader = this;
		mpcScope->Add(pcSymbol);
	}

	muiLazySkipped++;
	sEnd.iToken = iLast + 1;
	sEnd.bHalf = false;
	Rewind(sEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The name's lazy symbols in pcScope are taken out and their declarations
// parsed, in the order they were in, which declares the real symbols.
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::LoadLazy(CCPPScope* pcScope, int64 lliName)
{
	CCPPSymbol**						ppcFirst;
	CCPPSymbol*							pcSymbol;
	CCPPSymbol*							pcReal;
	CCPPSymbol*							pcLast;
	CArrayTemplate<CCPPSyntaxLazyDeclaration*>	apcLazy;
	CCPPSyntaxLazyDeclaration*			pcLazy;
	size								i;
	size								j;

	ppcFirst = pcScope->mcSymbols.Get(lliName);
	if (!ppcFirst)
	{
		return;
	}

	//The chain without its lazy symbols.
	apcLazy.Init();
	pcReal = NULL;
	pcLast = NULL;
	for (pcSymbol = *ppcFirst; pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		if (pcSymbol->mpcLazy)
		{
			pcLazy = pcSymbol->mpcLazy;
			for (j = 0; (j < apcLazy.NumElements()) && (*apcLazy.Get(j) != pcLazy); j++)
			{
			}
			if (j == apcLazy.NumElements())
			{
				apcLazy.Add(&pcLazy);
			}
		}
		else
		{
			if (pcLast)
			{
				pcLast->mpcNext = pcSymbol;
			}
			else
			{
				pcReal = pcSymbol;
			}
			pcLast = pcSymbol;
		}
	}
	if (pcLast)
	{
		pcLast->mpcNext = NULL;
	}
	pcScope->mcSymbols.Put(lliName, &pcReal);

	//In the order they were declared.
	for (i = 0; i < apcLazy.NumElements(); i++)
	{
		for (j = i + 1; j < apcLazy.NumElements(); j++)
		{
			if ((*apcLazy.Get(j))->miFirstToken < (*apcLazy.Get(i))->miFirstToken)
			{
				pcLazy = *apcLazy.Get(i);
				*apcLazy.Get(i) = *apcLazy.Get(j);
				*apcLazy.Get(j) = pcLazy;
			}
		}
	}
	for (i = 0; i < apcLazy.NumElements(); i++)
	{
		pcLazy = *apcLazy.Get(i);
		if (!pcLazy->mbLoaded)
		{
			LoadDeclaration(pcLazy);
		}
	}
	apcLazy.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Reads a lazy declaration as it was parsed before (its block, kept by the
// header index), in place of the placeholder.  False if it has no block, or
// the block can't be read here (it is parsed instead).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ReadDeclaration(CCPPSyntaxLazyDeclaration* pcLazy)
{
	CArrayUint8*	pauiBlock;
	CCPPSyntax*		pcDeclaration;

	pauiBlock = pcLazy->mpsRecord ? &pcLazy->mpsRecord->auiBlock : NULL;
	if (!pauiBlock || (pauiBlock->NumElements() == 0))
	{
		return false;
	}
	pcDeclaration = CCPPSyntaxBlockReader::Read(pauiBlock->GetData(), pauiBlock->NumElements(), mpcTree, pcLazy->miFirstToken, mpcSymbols->GetGlobalScope(), pcLazy->mpcParent, this);
	if (!pcDeclaration || (pcDeclaration->miLastToken != pcLazy->miLastToken))
	{
		return false;
	}
	pcLazy->mpapcOwner->SetPtr(pcLazy->muiIndex, pcDeclaration);
	muiLazyRead++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads a lazy declaration as it was parsed before if it can be
// (ReadDeclaration), otherwise parses it where it was in its namespace,
// with the parser as it was there, and puts it in place of the
// placeholder.  The parser is put back as it was.  Then its class's member
// functions defined outside it are loaded.
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::LoadDeclaration(CCPPSyntaxLazyDeclaration* pcLazy)
{
	SCPPParserPosition				sPosition;
	SCPPParserPosition				sStart;
	SCPPParserState					sState;
	bool							bMemberName;
	int								iTentative;
	bool							bFailed;
	int								iPack;
	CArrayDeferredBodies			acDeferred;
	CArrayTemplate<CCPPSymbol*>*	papcRecorded;
	CCPPScope*						pcRecordScope;
	CCPPSyntaxDeclaration*			pcDeclaration;

	pcLazy->mbLoaded = true;
	muiLazyLoaded++;
	if (ReadDeclaration(pcLazy))
	{
		LoadMembers(pcLazy);
		return;
	}

	sPosition = Mark();
	sState = SaveState();
	bMemberName = mbMemberName;
	iTentative = miTentative;
	bFailed = mbFailed;
	iPack = miPack;
	acDeferred = macDeferred;
	macDeferred.Init();
	papcRecorded = mpapcRecorded;
	pcRecordScope = mpcRecordScope;

	mpcScope = pcLazy->mpcScope;
	miClassDepth = 0;
	mbNoGreater = false;
	mbMemberName = false;
	miTentative = 0;
	mbFailed = false;
	mbDeclaringTemplate = false;
	miPack = pcLazy->miPack;
	mpapcRecorded = NULL;
	mpcRecordScope = NULL;
	sStart.iToken = pcLazy->miFirstToken;
	sStart.bHalf = false;
	Rewind(sStart);

	pcDeclaration = ParseDeclaration(pcLazy->mpcParent, CPPDC_Namespace);
	if (pcDeclaration && (PreviousToken() == pcLazy->miLastToken))
	{
		pcLazy->mpapcOwner->SetPtr(pcLazy->muiIndex, pcDeclaration);
	}

	macDeferred.Kill();
	macDeferred = acDeferred;
	mpapcRecorded = papcRecorded;
	mpcRecordScope = pcRecordScope;
	miPack = iPack;
	mbFailed = bFailed;
	miTentative = iTentative;
	mbMemberName = bMemberName;
	RestoreState(&sState);
	Rewind(sPosition);

	LoadMembers(pcLazy);
}


//////////////////////////////////////////////////////////////////////////
//
// The member functions of a lazy class defined outside it, loaded after it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::LoadMembers(CCPPSyntaxLazyDeclaration* pcLazy)
{
	CCPPSyntaxLazyDeclaration*	pcMember;

	for (pcMember = pcLazy->mpcMembers; pcMember; pcMember = pcMember->mpcNextMember)
	{
		if (!pcMember->mbLoaded)
		{
			LoadDeclaration(pcMember);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The class of a member function defined outside it, as in
// template<class M> void CArrayTemplate<M>::Add(M* pv) { ... }, if the class is in the
// current namespace.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPParser::MemberDefinitionClass(CCPPSyntaxDeclaration* pcDeclaration)
{
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxName*					pcName;
	CCPPSyntaxNameComponent*		pcClass;
	CCPPSymbol*						pcSymbol;

	while (pcDeclaration && (pcDeclaration->GetKind() == CPPS_TemplateDeclaration))
	{
		pcDeclaration = ((CCPPSyntaxTemplateDeclaration*)pcDeclaration)->mpcDeclaration;
	}
	if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_FunctionDefinition))
	{
		return NULL;
	}
	pcFunction = (CCPPSyntaxFunctionDefinition*)pcDeclaration;
	pcName = pcFunction->mpcDeclarator ? pcFunction->mpcDeclarator->GetName() : NULL;
	if (!pcName || pcName->mbGlobal || (pcName->macpComponents.NumElements() != 2))
	{
		return NULL;
	}
	pcClass = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0);
	pcSymbol = pcClass->mpcSymbol;
	if (!pcSymbol || (pcSymbol->mpcScope != mpcScope) || ((pcSymbol->meKind != CPPSK_Class) && (pcSymbol->meKind != CPPSK_ClassTemplate)) || !mpcTokeniser->GetIdentifier(pcSymbol->mlliName))
	{
		return NULL;
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// The current scope has a parsed declaration of szName.  Nothing is loaded.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::HasParsedSymbol(const char* szName)
{
	CCPPSymbol**	ppcSymbol;
	CCPPSymbol*		pcSymbol;

	ppcSymbol = mpcScope->mcSymbols.Get(mpcTokeniser->GetIdentifierID((char*)szName));
	if (!ppcSymbol)
	{
		return false;
	}
	for (pcSymbol = *ppcSymbol; pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		if (!pcSymbol->mpcLazy)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The unparsed declaration of the class szName in the current scope, or NULL if
// the class is parsed (or there is no such class).  Nothing is loaded.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxLazyDeclaration* CCPPParser::UnloadedClass(const char* szName)
{
	CCPPSymbol**	ppcSymbol;
	CCPPSymbol*		pcSymbol;

	ppcSymbol = mpcScope->mcSymbols.Get(mpcTokeniser->GetIdentifierID((char*)szName));
	if (!ppcSymbol)
	{
		return NULL;
	}
	for (pcSymbol = *ppcSymbol; pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		if (pcSymbol->mpcLazy && !pcSymbol->mpcLazy->mbLoaded && ((pcSymbol->meKind == CPPSK_Class) || (pcSymbol->meKind == CPPSK_ClassTemplate)))
		{
			return pcSymbol->mpcLazy;
		}
	}
	return NULL;
}
