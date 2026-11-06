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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/FileUtil.h"
#include "CPPScope.h"
#include "CPPSyntaxKind.h"
#include "CPPSyntaxDeclarator.h"
#include "CPPSyntaxFunctionDefinition.h"
#include "CPPSyntaxFunctionSuffix.h"
#include "CPPSyntaxSimpleDeclaration.h"
#include "CPPSyntaxInitDeclarator.h"
#include "CPPSyntaxDeclSpecifiers.h"
#include "CPPSyntaxClassSpecifier.h"
#include "CPPSyntaxEnumSpecifier.h"
#include "CPPSyntaxCallExpression.h"
#include "CPPSyntaxIdExpression.h"
#include "CPPSyntaxMemberExpression.h"
#include "CPPSyntaxName.h"
#include "CPPSyntaxNameComponent.h"
#include "CPPSyntaxTranslationUnit.h"
#include "SourceFile.h"
#include "CPPIndex.h"


//A name more than this many columns before a place doesn't cover it (FindSymbolAt).
#define CPP_INDEX_MAX_NAME_LENGTH	256


//////////////////////////////////////////////////////////////////////////
//
// A place in a file as one number: 20 bits of file, 24 of line and 20
// of column.
//
//////////////////////////////////////////////////////////////////////////
int64 GetCPPIndexLocationKey(uint32 uiFile, int iLine, int iColumn)
{
	return (int64)(((uint64)uiFile << 44) | (((uint64)iLine & 0xFFFFFF) << 20) | ((uint64)iColumn & 0xFFFFF));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexSymbol::Init(const char* szKey, ECPPSymbolKind eKind)
{
	mszKey.Init(szKey);
	meKind = eKind;
	masLocations.Init();
	maiBases.Init();
	maiDerived.Init();
	mbVirtual = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexSymbol::Kill(void)
{
	maiDerived.Kill();
	maiBases.Kill();
	masLocations.Kill();
	mszKey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Where the symbol is defined (a function's body, a class's members),
// NULL if no unit added defines it.
//
//////////////////////////////////////////////////////////////////////////
SCPPIndexLocation* CCPPIndexSymbol::GetDefinition(void)
{
	size	i;

	for (i = 0; i < masLocations.NumElements(); i++)
	{
		if (masLocations.Get(i)->uiRole == CPPIR_Definition)
		{
			return masLocations.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the symbol is first declared without being defined, NULL if
// only its definition declares it.
//
//////////////////////////////////////////////////////////////////////////
SCPPIndexLocation* CCPPIndexSymbol::GetDeclaration(void)
{
	size	i;

	for (i = 0; i < masLocations.NumElements(); i++)
	{
		if (masLocations.Get(i)->uiRole == CPPIR_Declaration)
		{
			return masLocations.Get(i);
		}
	}
	return NULL;
}


size CCPPIndexSymbol::NumLocations(void) { return masLocations.NumElements(); }
SCPPIndexLocation* CCPPIndexSymbol::GetLocation(size uiIndex) { return masLocations.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::Init(void)
{
	maszFiles.Init();
	mcFiles.Init();
	mapcSymbols.Init();
	mcKeys.Init();
	mcLocations.Init(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::Kill(void)
{
	size				i;
	CCPPIndexSymbol*	pcSymbol;

	mcLocations.Kill();
	mcKeys.Kill();
	for (i = 0; i < mapcSymbols.NumElements(); i++)
	{
		pcSymbol = mapcSymbols.GetPtr(i);
		SafeKill(pcSymbol);
	}
	mapcSymbols.Kill();
	mcFiles.Kill();
	maszFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::Init(void)
{
	masEntries.Init();
	maszKeys.Init();
	mcKeys.Init();
	maiKinds.Init();
	maszFiles.Init();
	mcFiles.Init();
	miContainer = -1;
	maiBaseLinks.Init();
	maiVirtuals.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::Kill(void)
{
	maiVirtuals.Kill();
	maiBaseLinks.Kill();
	mcFiles.Kill();
	maszFiles.Kill();
	maiKinds.Kill();
	mcKeys.Kill();
	maszKeys.Kill();
	masEntries.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A name of the symbol szKey at a token's place.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::Add(const char* szKey, ECPPSymbolKind eKind, SPreprocessedToken* psSource, ECPPIndexRole eRole, uint8 uiFlags)
{
	SCPPIndexEntry*	psEntry;
	int				iKey;
	int				iFile;
	char*			szFileName;

	iKey = AddKey(szKey, eKind);
	szFileName = psSource->pcFile->FullName();
	iFile = mcFiles.Get(szFileName, -1);
	if (iFile < 0)
	{
		iFile = (int)maszFiles.NumElements();
		maszFiles.Add(szFileName);
		mcFiles.Put(szFileName, iFile);
	}

	psEntry = masEntries.Add();
	psEntry->uiKey = (uint32)iKey;
	psEntry->uiFile = (uint32)iFile;
	psEntry->iLine = psSource->iLine;
	psEntry->iColumn = psSource->iColumn;
	psEntry->iLength = (int32)psSource->uiLength;
	psEntry->uiRole = (uint8)eRole;
	psEntry->uiFlags = uiFlags;
	psEntry->iContainer = miContainer;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of szKey in the unit's keys, added if it isn't there.
//
//////////////////////////////////////////////////////////////////////////
int CCPPIndexUnit::AddKey(const char* szKey, ECPPSymbolKind eKind)
{
	int	iKey;

	iKey = mcKeys.Get(szKey, -1);
	if (iKey < 0)
	{
		iKey = (int)maszKeys.NumElements();
		maszKeys.Add(szKey);
		maiKinds.Add((int)eKind);
		mcKeys.Put(szKey, iKey);
	}
	return iKey;
}


//////////////////////////////////////////////////////////////////////////
//
// The names added next are in the class or function with szKey, NULL for
// neither: where Find Usages groups them.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::SetContainer(const char* szKey, ECPPSymbolKind eKind)
{
	miContainer = szKey ? AddKey(szKey, eKind) : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The class szClass has szBase as a base class.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::AddBase(const char* szClass, ECPPSymbolKind eClassKind, const char* szBase, ECPPSymbolKind eBaseKind)
{
	maiBaseLinks.Add(AddKey(szClass, eClassKind));
	maiBaseLinks.Add(AddKey(szBase, eBaseKind));
}


//////////////////////////////////////////////////////////////////////////
//
// The member function szFunction is declared virtual, override or final.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::AddVirtual(const char* szFunction, ECPPSymbolKind eKind)
{
	maiVirtuals.Add(AddKey(szFunction, eKind));
}


//////////////////////////////////////////////////////////////////////////
//
// Every name a parsed translation unit's files declare or use, gathered
// and merged.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::AddUnit(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree, CCPPSemanticPass* pcPass)
{
	CCPPIndexUnit	cUnit;

	cUnit.Init();
	Gather(pcTokeniser, pcTree, &cUnit, pcPass);
	Merge(&cUnit);
	cUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every name a parsed translation unit's files declare or use, put in
// pcDest.  It touches no index so units can be gathered on several
// threads at once.  Names from macro bodies are where the macro is
// defined, not used, so they are left out, but for what they declare: a
// class named by a macro, class ARRAY_BLOCK, is declared where the macro
// was used (CPPIF_MACRO).  With a semantic pass (NULL
// for none) the names it resolved are what it says: the members o.Init()
// and p->miCount name, and the overloads calls call.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::Gather(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree, CCPPIndexUnit* pcDest, CCPPSemanticPass* pcPass)
{
	CCPPSymbolKey			cKeys;
	CArrayIndexTokens		asTokens;
	size					i;
	size					uiNumTokens;
	CCPPToken*				pcToken;
	SPreprocessedToken*		psSource;
	SCPPTokenSymbol*		psSymbol;
	SCPPResolvedName*		psResolved;
	SPreprocessedToken		sMacroUse;
	uint8					uiFlags;
	CCPPSyntax*				pcContainer;
	CChars					szContainer;
	ECPPSymbolKind			eContainerKind;

	uiNumTokens = pcTokeniser->NumTokens();
	asTokens.Init();
	asTokens.GrowToAtLeastNumElements(uiNumTokens, true, 0);
	if (pcTree->GetRoot())
	{
		FindNames(pcTree->GetRoot(), &asTokens, false);
		FindContainers(pcTree->GetRoot(), &asTokens);
	}

	cKeys.Init(pcTokeniser, pcTree);
	if (pcTree->GetRoot())
	{
		FindHierarchy(&cKeys, pcTree, pcTree->GetRoot(), pcDest);
	}
	pcContainer = NULL;
	szContainer.Init();
	pcDest->SetContainer(NULL, CPPSK_Class);
	for (i = 0; i < uiNumTokens; i++)
	{
		pcToken = pcTokeniser->Get(i);
		if (!pcToken->IsIdentifier())
		{
			continue;
		}
		psSymbol = pcTree->GetTokenSymbol((int)i);
		psResolved = pcPass ? pcPass->GetResolved((int)i) : NULL;
		if (!psResolved && ((psSymbol == NULL) || (psSymbol->pcSymbol == NULL) || ((psSymbol->uiRole != CPPTR_Declaration) && (psSymbol->uiRole != CPPTR_Reference))))
		{
			continue;
		}
		psSource = pcTokeniser->GetSource(pcToken);
		if (psSource->pcFile == NULL)
		{
			continue;
		}
		uiFlags = 0;
		if (psSource->iExpansion != -1)
		{
			if (psResolved || (psSymbol == NULL) || (psSymbol->uiRole != CPPTR_Declaration) || !GetMacroUse(pcTokeniser, psSource, &sMacroUse))
			{
				continue;
			}
			psSource = &sMacroUse;
			uiFlags = CPPIF_MACRO;
		}

		//Each name is in its innermost class or function, the same as the name before it most often.
		if (asTokens.Get(i)->pcContainer != pcContainer)
		{
			pcContainer = asTokens.Get(i)->pcContainer;
			szContainer.Clear();
			if (pcContainer && GetContainerKey(&cKeys, pcTree, pcContainer, &szContainer, &eContainerKind))
			{
				pcDest->SetContainer(szContainer.Text(), eContainerKind);
			}
			else
			{
				pcDest->SetContainer(NULL, CPPSK_Class);
			}
		}

		if (psResolved)
		{
			GatherResolved(&cKeys, psResolved, psSource, pcDest);
		}
		else
		{
			GatherToken(&cKeys, psSymbol, asTokens.Get(i), psSource, uiFlags, pcDest);
		}
	}
	pcDest->SetContainer(NULL, CPPSK_Class);
	szContainer.Kill();
	cKeys.Kill();
	asTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A gathered unit's names added to the index.  A header's names are
// seen again by every unit that includes it; a place a symbol already
// has is skipped.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::Merge(CCPPIndexUnit* pcUnit)
{
	CArrayInt			aiSymbols;
	CArrayInt			aiFiles;
	size				i;
	SCPPIndexEntry*		psEntry;
	SCPPIndexLocation	sLocation;
	int					iClass;
	int					iBase;

	aiSymbols.Init();
	for (i = 0; i < pcUnit->maszKeys.NumElements(); i++)
	{
		aiSymbols.Add(GetSymbolIndex(pcUnit->maszKeys.GetText(i), (ECPPSymbolKind)pcUnit->maiKinds.GetValue(i)));
	}
	aiFiles.Init();
	for (i = 0; i < pcUnit->maszFiles.NumElements(); i++)
	{
		aiFiles.Add((int)GetFileIndex(pcUnit->maszFiles.GetText(i)));
	}

	for (i = 0; i < pcUnit->masEntries.NumElements(); i++)
	{
		psEntry = pcUnit->masEntries.Get(i);
		sLocation.uiFile = (uint32)aiFiles.GetValue(psEntry->uiFile);
		sLocation.iLine = psEntry->iLine;
		sLocation.iColumn = psEntry->iColumn;
		sLocation.iLength = psEntry->iLength;
		sLocation.uiRole = psEntry->uiRole;
		sLocation.uiFlags = psEntry->uiFlags;
		sLocation.iContainer = (psEntry->iContainer >= 0) ? aiSymbols.GetValue((size)psEntry->iContainer) : -1;
		AddLocation(aiSymbols.GetValue(psEntry->uiKey), &sLocation);
	}

	//The classes' bases and the virtual functions, for the overrides of a function (FindOverrides).
	for (i = 0; i + 1 < pcUnit->maiBaseLinks.NumElements(); i += 2)
	{
		iClass = aiSymbols.GetValue((size)pcUnit->maiBaseLinks.GetValue(i));
		iBase = aiSymbols.GetValue((size)pcUnit->maiBaseLinks.GetValue(i + 1));
		if ((iClass != iBase) && (mapcSymbols.GetPtr((size)iClass)->maiBases.Find(iBase) == ARRAY_ELEMENT_NOT_FOUND))
		{
			mapcSymbols.GetPtr((size)iClass)->maiBases.Add(iBase);
			mapcSymbols.GetPtr((size)iBase)->maiDerived.Add(iClass);
		}
	}
	for (i = 0; i < pcUnit->maiVirtuals.NumElements(); i++)
	{
		mapcSymbols.GetPtr((size)aiSymbols.GetValue((size)pcUnit->maiVirtuals.GetValue(i)))->mbVirtual = true;
	}
	aiFiles.Kill();
	aiSymbols.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each token's innermost class or function definition: its node is set
// on every token it spans, then its children's on theirs.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::FindContainers(CCPPSyntax* pcNode, CArrayIndexTokens* pasTokens)
{
	int32			iToken;
	SCPPIndexToken*	psToken;
	size			i;
	size			uiChildren;
	CCPPSyntax*		pcChild;

	if ((pcNode->GetKind() == CPPS_FunctionDefinition) || ((pcNode->GetKind() == CPPS_ClassSpecifier) && ((CCPPSyntaxClassSpecifier*)pcNode)->mpcName))
	{
		for (iToken = pcNode->miFirstToken; (iToken >= 0) && (iToken <= pcNode->miLastToken); iToken++)
		{
			psToken = pasTokens->SafeGet((size)iToken);
			if (psToken)
			{
				psToken->pcContainer = pcNode;
			}
		}
	}
	uiChildren = pcNode->NumChildren();
	for (i = 0; i < uiChildren; i++)
	{
		pcChild = pcNode->GetChild(i);
		if (pcChild)
		{
			FindContainers(pcChild, pasTokens);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The base classes of the classes pcNode defines, and the member
// functions it declares virtual, override or final.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::FindHierarchy(CCPPSymbolKey* pcKeys, CCPPSyntaxTree* pcTree, CCPPSyntax* pcNode, CCPPIndexUnit* pcDest)
{
	CCPPSyntaxClassSpecifier*	pcClass;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntax*					pcDeclarator;
	CCPPScope*					pcBase;
	CChars						szClass;
	CChars						szBase;
	ECPPSymbolKind				eKind;
	bool						bVirtual;
	size						i;
	size						uiChildren;
	CCPPSyntax*					pcChild;

	switch (pcNode->GetKind())
	{
	case CPPS_ClassSpecifier:
		pcClass = (CCPPSyntaxClassSpecifier*)pcNode;
		if (pcClass->mpcName && pcClass->mpcScope && (pcClass->mpcScope->mapcBases.NumElements() > 0))
		{
			szClass.Init();
			if (GetContainerKey(pcKeys, pcTree, pcNode, &szClass, &eKind))
			{
				szBase.Init();
				for (i = 0; i < pcClass->mpcScope->mapcBases.NumElements(); i++)
				{
					pcBase = pcClass->mpcScope->mapcBases.GetPtr(i);
					szBase.Clear();
					if (pcBase && pcBase->mpcOwner && pcKeys->GetKey(pcBase->mpcOwner, &szBase))
					{
						pcDest->AddBase(szClass.Text(), eKind, szBase.Text(), pcBase->mpcOwner->meKind);
					}
				}
				szBase.Kill();
			}
			szClass.Kill();
		}
		break;
	case CPPS_SimpleDeclaration:
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcNode;
		bVirtual = pcSimple->mpcSpecifiers && (pcSimple->mpcSpecifiers->muiFlags & CPPDS_VIRTUAL);
		for (i = 0; i < pcSimple->macpDeclarators.NumElements(); i++)
		{
			pcDeclarator = pcSimple->macpDeclarators.GetPtr(i);
			if (pcDeclarator && (pcDeclarator->GetKind() == CPPS_InitDeclarator))
			{
				pcDeclarator = ((CCPPSyntaxInitDeclarator*)pcDeclarator)->mpcDeclarator;
			}
			if (pcDeclarator && (pcDeclarator->GetKind() == CPPS_Declarator))
			{
				GatherVirtual(pcKeys, pcTree, (CCPPSyntaxDeclarator*)pcDeclarator, bVirtual, pcDest);
			}
		}
		break;
	case CPPS_FunctionDefinition:
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcNode;
		bVirtual = pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_VIRTUAL);
		GatherVirtual(pcKeys, pcTree, pcDefinition->mpcDeclarator, bVirtual, pcDest);
		break;
	default:
		break;
	}

	uiChildren = pcNode->NumChildren();
	for (i = 0; i < uiChildren; i++)
	{
		pcChild = pcNode->GetChild(i);
		if (pcChild)
		{
			FindHierarchy(pcKeys, pcTree, pcChild, pcDest);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function pcDeclarator declares is virtual if bVirtual (it has the
// virtual specifier) or it is declared override or final.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::GatherVirtual(CCPPSymbolKey* pcKeys, CCPPSyntaxTree* pcTree, CCPPSyntaxDeclarator* pcDeclarator, bool bVirtual, CCPPIndexUnit* pcDest)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxName*				pcName;
	SCPPTokenSymbol*			psSymbol;
	CChars						szKey;

	if ((pcDeclarator == NULL) || !pcDeclarator->IsFunction())
	{
		return;
	}
	if (!bVirtual)
	{
		pcSuffix = pcDeclarator->GetFunctionSuffix();
		bVirtual = pcSuffix && (pcSuffix->macpVirtSpecifiers.NumElements() > 0);
	}
	if (!bVirtual)
	{
		return;
	}
	pcName = pcDeclarator->GetName();
	if ((pcName == NULL) || (pcName->GetLast() == NULL) || (pcName->GetLast()->miName < 0))
	{
		return;
	}
	psSymbol = pcTree->GetTokenSymbol(pcName->GetLast()->miName);
	if ((psSymbol == NULL) || (psSymbol->pcSymbol == NULL) || !pcKeys->IsFunction(psSymbol->pcSymbol))
	{
		return;
	}
	szKey.Init();
	if (pcKeys->GetFunctionKey(psSymbol->pcSymbol, pcDeclarator, &szKey))
	{
		pcDest->AddVirtual(szKey.Text(), psSymbol->pcSymbol->meKind);
	}
	szKey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The key and kind of the class or function pcContainer defines.  False
// if its name isn't known.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPIndex::GetContainerKey(CCPPSymbolKey* pcKeys, CCPPSyntaxTree* pcTree, CCPPSyntax* pcContainer, CChars* pszDest, ECPPSymbolKind* peKind)
{
	CCPPSyntaxName*			pcName;
	CCPPSyntaxDeclarator*	pcDeclarator;
	SCPPTokenSymbol*		psSymbol;

	pcDeclarator = NULL;
	if (pcContainer->GetKind() == CPPS_FunctionDefinition)
	{
		pcDeclarator = ((CCPPSyntaxFunctionDefinition*)pcContainer)->mpcDeclarator;
		pcName = pcDeclarator ? pcDeclarator->GetName() : NULL;
	}
	else
	{
		pcName = ((CCPPSyntaxClassSpecifier*)pcContainer)->mpcName;
	}
	if ((pcName == NULL) || (pcName->GetLast() == NULL) || (pcName->GetLast()->miName < 0))
	{
		return false;
	}
	psSymbol = pcTree->GetTokenSymbol(pcName->GetLast()->miName);
	if ((psSymbol == NULL) || (psSymbol->pcSymbol == NULL))
	{
		return false;
	}
	*peKind = psSymbol->pcSymbol->meKind;
	if (pcDeclarator && pcKeys->IsFunction(psSymbol->pcSymbol))
	{
		return pcKeys->GetFunctionKey(psSymbol->pcSymbol, pcDeclarator, pszDest);
	}
	return pcKeys->GetKey(psSymbol->pcSymbol, pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// What the tree says about name tokens: the function declarators they
// name, the number of arguments a call passes the callee they name and
// whether they name what a definition defines.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::FindNames(CCPPSyntax* pcNode, CArrayIndexTokens* pasTokens, bool bInDefinition)
{
	CCPPSyntaxName*			pcName;
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPSyntaxExpression*	pcCallee;
	SCPPIndexToken*			psToken;
	size					i;
	size					uiChildren;
	CCPPSyntax*				pcChild;
	int						iArguments;

	pcName = NULL;
	iArguments = 0;
	switch (pcNode->GetKind())
	{
	case CPPS_FunctionDefinition:
		pcDeclarator = ((CCPPSyntaxFunctionDefinition*)pcNode)->mpcDeclarator;
		pcName = pcDeclarator ? pcDeclarator->GetName() : NULL;
		bInDefinition = true;
		break;
	case CPPS_Declarator:
		pcDeclarator = (CCPPSyntaxDeclarator*)pcNode;
		if (pcDeclarator->IsFunction())
		{
			pcName = pcDeclarator->GetName();
		}
		break;
	case CPPS_ClassSpecifier:
		pcName = ((CCPPSyntaxClassSpecifier*)pcNode)->mpcName;
		bInDefinition = true;
		break;
	case CPPS_EnumSpecifier:
		pcName = ((CCPPSyntaxEnumSpecifier*)pcNode)->mpcName;
		bInDefinition = ((CCPPSyntaxEnumSpecifier*)pcNode)->mbBody;
		break;
	case CPPS_CallExpression:
		pcCallee = ((CCPPSyntaxCallExpression*)pcNode)->mpcCallee;
		if (pcCallee && (pcCallee->GetKind() == CPPS_IdExpression))
		{
			pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		}
		else if (pcCallee && (pcCallee->GetKind() == CPPS_MemberExpression))
		{
			pcName = ((CCPPSyntaxMemberExpression*)pcCallee)->mpcName;
		}
		iArguments = (int)((CCPPSyntaxCallExpression*)pcNode)->macpArguments.NumElements() + 1;
		break;
	default:
		break;
	}

	if (pcName && pcName->GetLast() && (pcName->GetLast()->miName >= 0))
	{
		psToken = pasTokens->SafeGet((size)pcName->GetLast()->miName);
		if (psToken)
		{
			if (iArguments > 0)
			{
				psToken->iArguments = iArguments;
			}
			else
			{
				if ((pcNode->GetKind() == CPPS_FunctionDefinition) || (pcNode->GetKind() == CPPS_Declarator))
				{
					if (psToken->pcDeclarator == NULL)
					{
						psToken->pcDeclarator = (CCPPSyntaxDeclarator*)((pcNode->GetKind() == CPPS_Declarator) ? pcNode : ((CCPPSyntaxFunctionDefinition*)pcNode)->mpcDeclarator);
					}
				}
				psToken->bDefinition = psToken->bDefinition || bInDefinition;
			}
		}
	}

	//A definition's own name is defined; what is inside it is only declared or used.
	if (pcNode->GetKind() == CPPS_FunctionDefinition)
	{
		bInDefinition = false;
	}
	uiChildren = pcNode->NumChildren();
	for (i = 0; i < uiChildren; i++)
	{
		pcChild = pcNode->GetChild(i);
		if (pcChild)
		{
			FindNames(pcChild, pasTokens, (pcNode->GetKind() == CPPS_ClassSpecifier) || (pcNode->GetKind() == CPPS_EnumSpecifier) ? false : bInDefinition);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where the macro that made psSource was used in the source text, the
// outermost if one macro's body used another: ArrayBlockClass.inl's
// ARRAY_BLOCK for the CArrayBlock it made.  psDest is a token there, as
// long as the macro's name.  False if it isn't known.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPIndex::GetMacroUse(CCPPTokeniser* pcTokeniser, SPreprocessedToken* psSource, SPreprocessedToken* psDest)
{
	CPreprocessedTokens*	pcSource;
	SMacroExpansion*		psExpansion;

	pcSource = pcTokeniser->mpcSource;
	if ((pcSource == NULL) || (psSource->iExpansion < 0) || ((size)psSource->iExpansion >= pcSource->NumExpansions()))
	{
		return false;
	}
	psExpansion = pcSource->GetExpansion((size)psSource->iExpansion);
	while ((psExpansion->iParent >= 0) && ((size)psExpansion->iParent < pcSource->NumExpansions()))
	{
		psExpansion = pcSource->GetExpansion((size)psExpansion->iParent);
	}
	if (psExpansion->pcFile == NULL)
	{
		return false;
	}
	memcpy(psDest, psSource, sizeof(SPreprocessedToken));
	psDest->szText = psExpansion->szName;
	psDest->uiLength = psExpansion->uiNameLength;
	psDest->pcFile = psExpansion->pcFile;
	psDest->iLine = psExpansion->iLine;
	psDest->iColumn = psExpansion->iColumn;
	psDest->iExpansion = -1;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// One name token: its symbol's key and where it is.  A function's
// declaration is keyed by the overload its declarator declares; a call
// by the overloads its number of arguments fits.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::GatherToken(CCPPSymbolKey* pcKeys, SCPPTokenSymbol* psSymbol, SCPPIndexToken* psToken, SPreprocessedToken* psSource, uint8 uiFlags, CCPPIndexUnit* pcDest)
{
	CCPPSymbol*		pcSymbol;
	CChars			szKey;
	ECPPIndexRole	eRole;
	bool			bKey;

	pcSymbol = psSymbol->pcSymbol;
	if ((psSymbol->uiRole == CPPTR_Reference) && pcKeys->IsFunction(pcSymbol))
	{
		GatherReferenceToOverload(pcKeys, pcSymbol, psToken->iArguments - 1, psSource, pcDest);
		return;
	}

	szKey.Init();
	if ((psSymbol->uiRole == CPPTR_Declaration) && pcKeys->IsFunction(pcSymbol) && psToken->pcDeclarator)
	{
		bKey = pcKeys->GetFunctionKey(pcSymbol, psToken->pcDeclarator, &szKey);
	}
	else
	{
		bKey = pcKeys->GetKey(pcSymbol, &szKey);
	}
	if (bKey)
	{
		if (psSymbol->uiRole == CPPTR_Reference)
		{
			eRole = CPPIR_Reference;
		}
		else
		{
			eRole = psToken->bDefinition ? CPPIR_Definition : CPPIR_Declaration;
		}
		pcDest->Add(szKey.Text(), pcSymbol->meKind, psSource, eRole, uiFlags);
	}
	szKey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A name the semantic pass resolved: a reference to the member it names,
// or to the one overload a call calls.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::GatherResolved(CCPPSymbolKey* pcKeys, SCPPResolvedName* psResolved, SPreprocessedToken* psSource, CCPPIndexUnit* pcDest)
{
	CChars	szKey;
	bool	bKey;

	szKey.Init();
	if (psResolved->pcDeclarator && pcKeys->IsFunction(psResolved->pcSymbol))
	{
		bKey = pcKeys->GetFunctionKey(psResolved->pcSymbol, psResolved->pcDeclarator, &szKey);
	}
	else
	{
		bKey = pcKeys->GetKey(psResolved->pcSymbol, &szKey);
	}
	if (bKey)
	{
		pcDest->Add(szKey.Text(), psResolved->pcSymbol->meKind, psSource, CPPIR_Reference, 0);
	}
	szKey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A use of a function name.  The parser finds a name's first overload,
// so the overloads declared with it that iArguments arguments fit (any,
// if iArguments is -1) are what it could call.  It is a reference to
// each, flagged ambiguous when there is more than one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::GatherReferenceToOverload(CCPPSymbolKey* pcKeys, CCPPSymbol* pcSymbol, int iArguments, SPreprocessedToken* psSource, CCPPIndexUnit* pcDest)
{
	CArrayChars		aszCandidates;
	CArrayChars		aszAll;
	CCPPSymbol*		pcOverload;
	CChars			szKey;
	int				iMinimum;
	int				iMaximum;
	bool			bFits;
	CArrayChars*	paszChosen;
	size			i;
	ECPPSymbolKind	eKind;

	aszCandidates.Init();
	aszAll.Init();
	eKind = pcSymbol->meKind;
	for (pcOverload = pcSymbol; pcOverload; pcOverload = pcOverload->mpcNext)
	{
		if (!pcKeys->IsFunction(pcOverload))
		{
			continue;
		}
		szKey.Init();
		if (pcKeys->GetKey(pcOverload, &szKey))
		{
			if (!aszAll.Contains(&szKey))
			{
				aszAll.Add(&szKey);
			}
			bFits = true;
			if ((iArguments >= 0) && pcKeys->GetParameterCounts(pcOverload, &iMinimum, &iMaximum))
			{
				bFits = (iArguments >= iMinimum) && ((iMaximum < 0) || (iArguments <= iMaximum));
			}
			if (bFits && !aszCandidates.Contains(&szKey))
			{
				aszCandidates.Add(&szKey);
			}
		}
		szKey.Kill();
	}

	paszChosen = (aszCandidates.NumElements() > 0) ? &aszCandidates : &aszAll;
	for (i = 0; i < paszChosen->NumElements(); i++)
	{
		pcDest->Add(paszChosen->GetText(i), eKind, psSource, CPPIR_Reference, (paszChosen->NumElements() > 1) ? CPPIF_AMBIGUOUS : 0);
	}
	aszAll.Kill();
	aszCandidates.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the symbol with szKey, added if it isn't there yet.
//
//////////////////////////////////////////////////////////////////////////
int CCPPIndex::GetSymbolIndex(const char* szKey, ECPPSymbolKind eKind)
{
	int					iIndex;
	CCPPIndexSymbol*	pcSymbol;

	iIndex = mcKeys.Get(szKey, -1);
	if (iIndex >= 0)
	{
		return iIndex;
	}
	pcSymbol = NewMalloc<CCPPIndexSymbol>();
	pcSymbol->Init(szKey, eKind);
	iIndex = (int)mapcSymbols.NumElements();
	mapcSymbols.Add(pcSymbol);
	mcKeys.Put(szKey, iIndex);
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPPIndex::GetFileIndex(const char* szFileName)
{
	int	iIndex;

	iIndex = mcFiles.Get(szFileName, -1);
	if (iIndex >= 0)
	{
		return (uint32)iIndex;
	}
	iIndex = (int)maszFiles.NumElements();
	maszFiles.Add(szFileName);
	mcFiles.Put(szFileName, iIndex);
	return (uint32)iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// A name of the symbol in a file, unless the symbol already has one at
// that place.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPIndex::AddLocation(int iSymbol, SCPPIndexLocation* psLocation)
{
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psExisting;
	int64				lliKey;
	int32*				piExisting;
	size				i;

	pcSymbol = mapcSymbols.GetPtr((size)iSymbol);
	lliKey = GetCPPIndexLocationKey(psLocation->uiFile, psLocation->iLine, psLocation->iColumn);
	piExisting = mcLocations.Get(lliKey);
	if (piExisting)
	{
		if (*piExisting == iSymbol)
		{
			return false;
		}
		for (i = 0; i < pcSymbol->masLocations.NumElements(); i++)
		{
			psExisting = pcSymbol->masLocations.Get(i);
			if ((psExisting->uiFile == psLocation->uiFile) && (psExisting->iLine == psLocation->iLine) && (psExisting->iColumn == psLocation->iColumn))
			{
				return false;
			}
		}
	}

	pcSymbol->masLocations.Add(psLocation);
	if (!piExisting)
	{
		mcLocations.Put(lliKey, &iSymbol);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The member functions that override, or are overridden by, the member
// function pcFunction, with it: those with its name and parameters in its
// class's base classes, and in the classes derived from those and from
// its class.  Only if one of them is virtual.  The symbols' indices are
// added to paiDest.  False (nothing added) if it overrides nothing and
// nothing overrides it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPIndex::FindOverrides(CCPPIndexSymbol* pcFunction, CArrayInt* paiDest)
{
	const char*			szKey;
	size				uiLength;
	size				uiOpen;
	size				uiName;
	int					iDepth;
	int					iClass;
	int					iMember;
	CChars				szScope;
	CChars				szMember;
	CArrayInt			aiAncestors;
	CArrayInt			aiTops;
	CArrayInt			aiFamily;
	CArrayInt			aiFound;
	bool				bVirtual;
	size				i;
	CCPPIndexSymbol*	pcClass;

	//The key is the class's, then ::, then the name and parameters.
	szKey = pcFunction->mszKey.Text();
	uiLength = pcFunction->mszKey.Length();
	iDepth = 0;
	for (uiOpen = 0; uiOpen < uiLength; uiOpen++)
	{
		if (szKey[uiOpen] == '<')
		{
			iDepth++;
		}
		else if (szKey[uiOpen] == '>')
		{
			iDepth--;
		}
		else if ((szKey[uiOpen] == '(') && (iDepth == 0))
		{
			break;
		}
	}
	iDepth = 0;
	for (uiName = uiOpen; uiName >= 2; uiName--)
	{
		if (szKey[uiName - 1] == '>')
		{
			iDepth++;
		}
		else if (szKey[uiName - 1] == '<')
		{
			iDepth--;
		}
		else if ((iDepth == 0) && (szKey[uiName - 1] == ':') && (szKey[uiName - 2] == ':'))
		{
			break;
		}
	}
	if ((uiOpen == uiLength) || (uiName < 2))
	{
		return false;
	}
	szScope.Init();
	szScope.AppendSubString(szKey, 0, uiName - 2);
	iClass = mcKeys.Get(szScope.Text(), -1);
	szScope.Kill();
	if (iClass < 0)
	{
		return false;
	}

	//The topmost classes with the function: its class and its bases that have it.  Then their derived classes.
	aiAncestors.Init();
	AddClosure(&mapcSymbols, iClass, true, &aiAncestors);
	aiTops.Init();
	szMember.Init();
	for (i = 0; i < aiAncestors.NumElements(); i++)
	{
		pcClass = mapcSymbols.GetPtr((size)aiAncestors.GetValue(i));
		szMember.Init(pcClass->mszKey);
		szMember.Append(&szKey[uiName - 2]);
		if (mcKeys.Get(szMember.Text(), -1) >= 0)
		{
			aiTops.Add(aiAncestors.GetValue(i));
		}
		szMember.Kill();
	}
	aiFamily.Init();
	for (i = 0; i < aiTops.NumElements(); i++)
	{
		AddClosure(&mapcSymbols, aiTops.GetValue(i), false, &aiFamily);
	}

	aiFound.Init();
	bVirtual = false;
	for (i = 0; i < aiFamily.NumElements(); i++)
	{
		pcClass = mapcSymbols.GetPtr((size)aiFamily.GetValue(i));
		szMember.Init(pcClass->mszKey);
		szMember.Append(&szKey[uiName - 2]);
		iMember = mcKeys.Get(szMember.Text(), -1);
		if (iMember >= 0)
		{
			aiFound.Add(iMember);
			bVirtual = bVirtual || mapcSymbols.GetPtr((size)iMember)->mbVirtual;
		}
		szMember.Kill();
	}
	if (bVirtual && (aiFound.NumElements() > 1))
	{
		for (i = 0; i < aiFound.NumElements(); i++)
		{
			paiDest->Add(aiFound.GetValue(i));
		}
	}
	else
	{
		bVirtual = false;
	}
	aiFound.Kill();
	aiFamily.Kill();
	aiTops.Kill();
	aiAncestors.Kill();
	return bVirtual;
}


//////////////////////////////////////////////////////////////////////////
//
// iSymbol and, with bBases, every class it derives from, else every class
// derived from it, each added to paiDest once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::AddClosure(CArrayIndexSymbols* papcSymbols, int iSymbol, bool bBases, CArrayInt* paiDest)
{
	size		uiStart;
	size		i;
	CArrayInt*	paiNext;

	if (paiDest->Find(iSymbol) != ARRAY_ELEMENT_NOT_FOUND)
	{
		return;
	}
	uiStart = paiDest->NumElements();
	paiDest->Add(iSymbol);
	for (; uiStart < paiDest->NumElements(); uiStart++)
	{
		paiNext = bBases ? &papcSymbols->GetPtr((size)paiDest->GetValue(uiStart))->maiBases : &papcSymbols->GetPtr((size)paiDest->GetValue(uiStart))->maiDerived;
		for (i = 0; i < paiNext->NumElements(); i++)
		{
			if (paiDest->Find(paiNext->GetValue(i)) == ARRAY_ELEMENT_NOT_FOUND)
			{
				paiDest->Add(paiNext->GetValue(i));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol named at a place (0 based) in a file: by a name that
// starts there or before it on the line and covers it.  NULL if none.
//
//////////////////////////////////////////////////////////////////////////
CCPPIndexSymbol* CCPPIndex::FindSymbolAt(const char* szFileName, int iLine, int iColumn)
{
	int					iFile;
	int					iStart;
	int32*				piSymbol;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;
	size				i;

	iFile = mcFiles.Get(szFileName, -1);
	if (iFile < 0)
	{
		return NULL;
	}
	for (iStart = iColumn; (iStart >= 0) && (iColumn - iStart < CPP_INDEX_MAX_NAME_LENGTH); iStart--)
	{
		piSymbol = mcLocations.Get(GetCPPIndexLocationKey((uint32)iFile, iLine, iStart));
		if (piSymbol)
		{
			pcSymbol = mapcSymbols.GetPtr((size)*piSymbol);
			for (i = 0; i < pcSymbol->masLocations.NumElements(); i++)
			{
				psLocation = pcSymbol->masLocations.Get(i);
				if ((psLocation->uiFile == (uint32)iFile) && (psLocation->iLine == iLine) && (psLocation->iColumn == iStart))
				{
					return (iColumn < iStart + psLocation->iLength) ? pcSymbol : NULL;
				}
			}
			return NULL;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Each symbol and its places, as File.cpp:line:column role (1 based),
// for tests.
//
//////////////////////////////////////////////////////////////////////////
char* CCPPIndex::Print(CChars* pszDest)
{
	size				i;
	size				j;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;
	CFileUtil			cFileUtil;
	CChars				szFile;

	for (i = 0; i < mapcSymbols.NumElements(); i++)
	{
		pcSymbol = mapcSymbols.GetPtr(i);
		pszDest->Append(pcSymbol->mszKey);
		pszDest->AppendNewLine();
		for (j = 0; j < pcSymbol->masLocations.NumElements(); j++)
		{
			psLocation = pcSymbol->masLocations.Get(j);
			szFile.Init();
			cFileUtil.GetFileName(&szFile, maszFiles.GetText(psLocation->uiFile));
			pszDest->Append("  ");
			pszDest->Append(szFile);
			pszDest->Append(':');
			pszDest->Append(psLocation->iLine + 1);
			pszDest->Append(':');
			pszDest->Append(psLocation->iColumn + 1);
			pszDest->Append((psLocation->uiRole == CPPIR_Definition) ? " definition" : (psLocation->uiRole == CPPIR_Declaration) ? " declaration" : " reference");
			if (psLocation->uiFlags & CPPIF_AMBIGUOUS)
			{
				pszDest->Append(" ambiguous");
			}
			if (psLocation->uiFlags & CPPIF_MACRO)
			{
				pszDest->Append(" macro");
			}
			pszDest->AppendNewLine();
			szFile.Kill();
		}
	}
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPIndexSymbol* CCPPIndex::FindSymbol(const char* szKey)
{
	int	iIndex;

	iIndex = mcKeys.Get(szKey, -1);
	return (iIndex >= 0) ? mapcSymbols.GetPtr((size)iIndex) : NULL;
}


size CCPPIndex::NumSymbols(void) { return mapcSymbols.NumElements(); }
CCPPIndexSymbol* CCPPIndex::GetSymbol(size uiIndex) { return mapcSymbols.GetPtr(uiIndex); }
size CCPPIndex::NumFiles(void) { return maszFiles.NumElements(); }
char* CCPPIndex::GetFileName(size uiFile) { return maszFiles.GetText(uiFile); }

