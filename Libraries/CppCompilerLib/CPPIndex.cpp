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
#include "CPPSyntaxLinkageSpecification.h"
#include "CPPSyntaxCallExpression.h"
#include "CPPSyntaxIdExpression.h"
#include "CPPSyntaxMemberExpression.h"
#include "CPPSyntaxBinaryExpression.h"
#include "CPPSyntaxUnaryExpression.h"
#include "CPPSyntaxParenthesisedExpression.h"
#include "CPPSyntaxMemberInitializer.h"
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
	masBaseLinks.Init();
	masVirtuals.Init();
	mcHierarchyKeys.Init(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::Kill(void)
{
	size				i;
	CCPPIndexSymbol*	pcSymbol;

	mcHierarchyKeys.Kill();
	masVirtuals.Kill();
	masBaseLinks.Kill();
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
	mcCachedKeys.Init(true);
	mpcLastFile = NULL;
	miLastFile = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::Kill(void)
{
	mcCachedKeys.Kill();
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
void CCPPIndexUnit::Add(const char* szKey, ECPPSymbolKind eKind, SPreprocessedToken* psSource, ECPPIndexRole eRole, uint8 uiFlags, ECPPUsageType eUsage)
{
	AddEntry(AddKey(szKey, eKind), psSource, eRole, uiFlags, eUsage);
}


//////////////////////////////////////////////////////////////////////////
//
// A name of the unit's key iKey (from AddKey) at a token's place, used as
// eUsage says.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::AddEntry(int iKey, SPreprocessedToken* psSource, ECPPIndexRole eRole, uint8 uiFlags, ECPPUsageType eUsage)
{
	SCPPIndexEntry*	psEntry;
	int				iFile;

	iFile = AddFile(psSource->pcFile);
	psEntry = masEntries.Add();
	psEntry->uiKey = (uint32)iKey;
	psEntry->uiFile = (uint32)iFile;
	psEntry->iLine = psSource->iLine;
	psEntry->iColumn = psSource->iColumn;
	psEntry->iLength = (int32)psSource->uiLength;
	psEntry->uiRole = (uint8)eRole;
	psEntry->uiFlags = uiFlags;
	psEntry->uiUsage = (uint8)eUsage;
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
// The class szClass has szBase as a base class, as pcFile (the class's
// definition's) says.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::AddBase(const char* szClass, ECPPSymbolKind eClassKind, const char* szBase, ECPPSymbolKind eBaseKind, CSourceFile* pcFile)
{
	maiBaseLinks.Add(AddKey(szClass, eClassKind));
	maiBaseLinks.Add(AddKey(szBase, eBaseKind));
	maiBaseLinks.Add(AddFile(pcFile));
}


//////////////////////////////////////////////////////////////////////////
//
// The member function szFunction is declared virtual, override or final
// in pcFile.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndexUnit::AddVirtual(const char* szFunction, ECPPSymbolKind eKind, CSourceFile* pcFile)
{
	maiVirtuals.Add(AddKey(szFunction, eKind));
	maiVirtuals.Add(AddFile(pcFile));
}


//////////////////////////////////////////////////////////////////////////
//
// The index of pcFile in the unit's files, added if it isn't there.
// Names come a file at a time: the last file's index is kept.
//
//////////////////////////////////////////////////////////////////////////
int CCPPIndexUnit::AddFile(CSourceFile* pcFile)
{
	int		iFile;
	char*	szFileName;

	if (pcFile == mpcLastFile)
	{
		return miLastFile;
	}
	szFileName = pcFile->FullName();
	iFile = mcFiles.Get(szFileName, -1);
	if (iFile < 0)
	{
		iFile = (int)maszFiles.NumElements();
		maszFiles.Add(szFileName);
		mcFiles.Put(szFileName, iFile);
	}
	mpcLastFile = pcFile;
	miLastFile = iFile;
	return iFile;
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
void CCPPIndex::Gather(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree, CCPPIndexUnit* pcDest, CCPPSemanticPass* pcPass, CCPPIndexedFiles* pcIndexed)
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
	CSourceFile*			pcLastFile;
	bool					bLastIndexed;
	SCPPUsageContext		sContext;

	uiNumTokens = pcTokeniser->NumTokens();
	pcLastFile = NULL;
	bLastIndexed = false;
	asTokens.Init();
	asTokens.GrowToAtLeastNumElements(uiNumTokens, true, 0);
	if (pcTree->GetRoot())
	{
		FindNames(pcTokeniser, pcTree->GetRoot(), &asTokens, false, false);
		FindContainers(pcTree->GetRoot(), &asTokens);
		sContext.eScope = CPPUS_Namespace;
		sContext.bExpression = false;
		sContext.bTemplateArgument = false;
		FindUsageTypes(pcTree->GetRoot(), &asTokens, &sContext);
	}

	cKeys.Init(pcTokeniser, pcTree);
	if (pcTree->GetRoot())
	{
		FindHierarchy(pcTokeniser, &cKeys, pcTree, pcTree->GetRoot(), pcDest);
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

		//A file another unit indexed (a header) has the same names here; tokens come a file at a time.  But the names the
		//semantic pass resolved are kept: a template's code is only typed where it is instantiated, which may not be in
		//the unit that indexed its header.
		if (pcIndexed && !psResolved)
		{
			if (psSource->pcFile != pcLastFile)
			{
				pcLastFile = psSource->pcFile;
				bLastIndexed = pcIndexed->IsIndexed(pcLastFile);
			}
			if (bLastIndexed)
			{
				continue;
			}
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
			GatherResolved(&cKeys, psResolved, psSource, (ECPPUsageType)asTokens.Get(i)->uiUsage, pcDest);
		}
		else
		{
			if (asTokens.Get(i)->bCLinkage && (psSymbol->uiRole == CPPTR_Declaration))
			{
				uiFlags |= CPPIF_C_LINKAGE;
			}
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
		sLocation.uiUsage = psEntry->uiUsage;
		sLocation.iContainer = (psEntry->iContainer >= 0) ? aiSymbols.GetValue((size)psEntry->iContainer) : -1;
		AddLocation(aiSymbols.GetValue(psEntry->uiKey), &sLocation);
	}

	//The classes' bases and the virtual functions, for the overrides of a function (FindOverrides).
	for (i = 0; i + 2 < pcUnit->maiBaseLinks.NumElements(); i += 3)
	{
		iClass = aiSymbols.GetValue((size)pcUnit->maiBaseLinks.GetValue(i));
		iBase = aiSymbols.GetValue((size)pcUnit->maiBaseLinks.GetValue(i + 1));
		AddBaseLink(iClass, iBase, (uint32)aiFiles.GetValue((size)pcUnit->maiBaseLinks.GetValue(i + 2)));
	}
	for (i = 0; i + 1 < pcUnit->maiVirtuals.NumElements(); i += 2)
	{
		AddVirtual(aiSymbols.GetValue((size)pcUnit->maiVirtuals.GetValue(i)), (uint32)aiFiles.GetValue((size)pcUnit->maiVirtuals.GetValue(i + 1)));
	}
	aiFiles.Kill();
	aiSymbols.Kill();
}


//The index's walks go on to each child: names (FindNames), containers (FindContainers) and classes' bases and
//virtual functions (FindHierarchy).
class CCPPIndexNameChildren : public CCPPSyntaxChildVisitor
{
public:
	CCPPTokeniser*		mpcTokeniser;
	CArrayIndexTokens*	mpasTokens;
	bool				mbInDefinition;
	bool				mbCLinkage;

	void	Visit(CCPPSyntax* pcChild) override	{ CCPPIndex::FindNames(mpcTokeniser, pcChild, mpasTokens, mbInDefinition, mbCLinkage); }
};


class CCPPIndexContainerChildren : public CCPPSyntaxChildVisitor
{
public:
	CArrayIndexTokens*	mpasTokens;

	void	Visit(CCPPSyntax* pcChild) override	{ CCPPIndex::FindContainers(pcChild, mpasTokens); }
};


class CCPPIndexHierarchyChildren : public CCPPSyntaxChildVisitor
{
public:
	CCPPTokeniser*	mpcTokeniser;
	CCPPSymbolKey*	mpcKeys;
	CCPPSyntaxTree*	mpcTree;
	CCPPIndexUnit*	mpcDest;

	void	Visit(CCPPSyntax* pcChild) override	{ CCPPIndex::FindHierarchy(mpcTokeniser, mpcKeys, mpcTree, pcChild, mpcDest); }
};


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
	CCPPIndexContainerChildren	cChildren;

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
	cChildren.mpasTokens = pasTokens;
	pcNode->VisitFields(&cChildren);
}


class CCPPIndexUsageChildren : public CCPPSyntaxChildVisitor
{
public:
	CArrayIndexTokens*	mpasTokens;
	SCPPUsageContext*	mpsContext;

	void	Visit(CCPPSyntax* pcChild) override	{ CCPPIndex::FindUsageTypes(pcChild, mpasTokens, mpsContext); }
};


//////////////////////////////////////////////////////////////////////////
//
// Each token's usage type, from the nodes it is in: a node says what the
// names it spans are used as (a parameter's, as its type), then its
// children say it again for theirs, the innermost last.  After the
// children, the names a node uses itself are set: what a call calls,
// what an assignment assigns to.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::FindUsageTypes(CCPPSyntax* pcNode, CArrayIndexTokens* pasTokens, SCPPUsageContext* psContext)
{
	CCPPIndexUsageChildren	cChildren;
	SCPPUsageContext		sContext;
	ECPPSyntax				eKind;
	ECppTokens				eOperator;
	CCPPSyntaxName*			pcName;

	sContext = *psContext;
	cChildren.mpasTokens = pasTokens;
	cChildren.mpsContext = &sContext;
	if (psContext->bTemplateArgument)
	{
		//All of a template argument is one, whatever it is made of.
		FillUsageType(pcNode, pasTokens, CPPUT_TemplateArgument);
		return;
	}

	eKind = pcNode->GetKind();
	switch (eKind)
	{
	case CPPS_TranslationUnit:
	case CPPS_Namespace:
	case CPPS_LinkageSpecification:
		sContext.eScope = CPPUS_Namespace;
		sContext.bExpression = false;
		break;
	case CPPS_ClassSpecifier:
		sContext.eScope = CPPUS_Class;
		sContext.bExpression = false;
		break;
	case CPPS_FunctionDefinition:
		FillUsageType(pcNode, pasTokens, CPPUT_Function);
		sContext.eScope = CPPUS_Function;
		sContext.bExpression = false;
		break;
	case CPPS_SimpleDeclaration:
		FillUsageType(pcNode, pasTokens, GetDeclarationUsageType(pcNode, psContext->eScope));
		sContext.bExpression = false;
		break;
	case CPPS_RangeForStatement:
		FillUsageType(pcNode, pasTokens, CPPUT_Local);
		break;
	case CPPS_Parameter:
		FillUsageType(pcNode, pasTokens, CPPUT_Parameter);
		sContext.bExpression = false;
		break;
	case CPPS_BaseSpecifier:
		FillUsageType(pcNode, pasTokens, CPPUT_BaseClass);
		break;
	case CPPS_AliasDeclaration:
	case CPPS_UsingDeclaration:
	case CPPS_UsingDirective:
		FillUsageType(pcNode, pasTokens, CPPUT_Alias);
		break;
	case CPPS_TypeId:
		if (psContext->bExpression)
		{
			FillUsageType(pcNode, pasTokens, CPPUT_TypeInExpression);
		}
		break;
	case CPPS_NameComponent:
		//What a name component has in it is its template's arguments.
		sContext.bTemplateArgument = true;
		break;
	default:
		if ((eKind >= CPPS_LiteralExpression) && (eKind <= CPPS_NoexceptExpression) && !psContext->bExpression)
		{
			//An expression's names are read, but for what says otherwise inside it.
			FillUsageType(pcNode, pasTokens, CPPUT_Read);
			sContext.bExpression = true;
		}
		break;
	}

	pcNode->VisitFields(&cChildren);

	switch (eKind)
	{
	case CPPS_CallExpression:
		SetNameUsageType(((CCPPSyntaxCallExpression*)pcNode)->mpcCallee, pasTokens, CPPUT_Call);
		break;
	case CPPS_BinaryExpression:
		eOperator = ((CCPPSyntaxBinaryExpression*)pcNode)->meOperator;
		if ((eOperator == CT_Assignment) || (eOperator == CT_AssignmentAdd) || (eOperator == CT_AssignmentSubtract) || (eOperator == CT_AssignmentMultiply) ||
			(eOperator == CT_AssignmentDivide) || (eOperator == CT_AssignmentModulus) || (eOperator == CT_AssignmentBitwiseAnd) || (eOperator == CT_AssignmentBitwiseOr) ||
			(eOperator == CT_AssignmentBitwiseXor) || (eOperator == CT_AssignmentLeftShift) || (eOperator == CT_AssignmentRightShift))
		{
			SetNameUsageType(((CCPPSyntaxBinaryExpression*)pcNode)->mpcLeft, pasTokens, CPPUT_Write);
		}
		break;
	case CPPS_UnaryExpression:
		eOperator = ((CCPPSyntaxUnaryExpression*)pcNode)->meOperator;
		if ((eOperator == CT_Increment) || (eOperator == CT_Decrement))
		{
			SetNameUsageType(((CCPPSyntaxUnaryExpression*)pcNode)->mpcOperand, pasTokens, CPPUT_Write);
		}
		break;
	case CPPS_MemberInitializer:
		pcName = ((CCPPSyntaxMemberInitializer*)pcNode)->mpcName;
		if (pcName && pcName->GetLast() && (pcName->GetLast()->miName >= 0) && pasTokens->SafeGet((size)pcName->GetLast()->miName))
		{
			pasTokens->Get((size)pcName->GetLast()->miName)->uiUsage = CPPUT_Write;
		}
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every token pcNode spans is used as eUsage, until something inside it
// says otherwise.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::FillUsageType(CCPPSyntax* pcNode, CArrayIndexTokens* pasTokens, ECPPUsageType eUsage)
{
	int32			iToken;
	SCPPIndexToken*	psToken;

	for (iToken = pcNode->miFirstToken; (iToken >= 0) && (iToken <= pcNode->miLastToken); iToken++)
	{
		psToken = pasTokens->SafeGet((size)iToken);
		if (psToken)
		{
			psToken->uiUsage = (uint8)eUsage;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The name an expression is (x, (x), o.x or p->x) is used as eUsage; any
// other expression names nothing.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::SetNameUsageType(CCPPSyntax* pcExpression, CArrayIndexTokens* pasTokens, ECPPUsageType eUsage)
{
	CCPPSyntaxName*	pcName;
	SCPPIndexToken*	psToken;

	while (pcExpression && (pcExpression->GetKind() == CPPS_ParenthesisedExpression))
	{
		pcExpression = ((CCPPSyntaxParenthesisedExpression*)pcExpression)->mpcExpression;
	}
	if (pcExpression == NULL)
	{
		return;
	}
	if (pcExpression->GetKind() == CPPS_IdExpression)
	{
		pcName = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName;
	}
	else if (pcExpression->GetKind() == CPPS_MemberExpression)
	{
		pcName = ((CCPPSyntaxMemberExpression*)pcExpression)->mpcName;
	}
	else
	{
		return;
	}
	if (pcName && pcName->GetLast() && (pcName->GetLast()->miName >= 0))
	{
		psToken = pasTokens->SafeGet((size)pcName->GetLast()->miName);
		if (psToken)
		{
			psToken->uiUsage = (uint8)eUsage;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a simple declaration's names are used as, where it is: a typedef's
// are an alias's, a function declaration's are its, and a variable's are
// a field's, a local's or a global's.  A declaration that declares
// nothing (class C { ... };) leaves its names to what is in it.
//
//////////////////////////////////////////////////////////////////////////
ECPPUsageType CCPPIndex::GetDeclarationUsageType(CCPPSyntax* pcNode, ECPPUsageScope eScope)
{
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntax*						pcDeclarator;
	CCPPSyntaxInitDeclarator*		pcInit;
	size							i;

	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcNode;
	if (pcSimple->mpcSpecifiers && (pcSimple->mpcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return CPPUT_Alias;
	}
	if (pcSimple->macpDeclarators.NumElements() == 0)
	{
		return CPPUT_Other;
	}
	for (i = 0; i < pcSimple->macpDeclarators.NumElements(); i++)
	{
		pcDeclarator = pcSimple->macpDeclarators.GetPtr(i);
		if (pcDeclarator && (pcDeclarator->GetKind() == CPPS_InitDeclarator))
		{
			pcInit = (CCPPSyntaxInitDeclarator*)pcDeclarator;
			if (pcInit->mpcDeclarator && pcInit->mpcDeclarator->IsFunction())
			{
				return CPPUT_Function;
			}
		}
	}
	switch (eScope)
	{
	case CPPUS_Class:
		return CPPUT_Field;
	case CPPUS_Function:
		return CPPUT_Local;
	default:
		return CPPUT_Global;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A type named in an expression (CFoo(1), (CFoo*)p) constructs or casts
// to it: it isn't read, written or called.
//
//////////////////////////////////////////////////////////////////////////
ECPPUsageType CCPPIndex::GetSymbolUsageType(CCPPSymbol* pcSymbol, ECPPUsageType eUsage)
{
	ECPPSymbolKind	eKind;

	if ((pcSymbol == NULL) || ((eUsage != CPPUT_Read) && (eUsage != CPPUT_Write) && (eUsage != CPPUT_Call)))
	{
		return eUsage;
	}
	eKind = pcSymbol->meKind;
	if ((eKind == CPPSK_Class) || (eKind == CPPSK_Enum) || (eKind == CPPSK_Typedef) || (eKind == CPPSK_ClassTemplate) || (eKind == CPPSK_AliasTemplate) ||
		(eKind == CPPSK_TypeParameter) || (eKind == CPPSK_BuiltinType))
	{
		return CPPUT_TypeInExpression;
	}
	return eUsage;
}


//////////////////////////////////////////////////////////////////////////
//
// Every place a name is in szFileName is forgotten, before the file is
// indexed again after it has changed, and the base classes and virtual
// functions it declares.  Its symbols are kept, with their places in other
// files; one named only there is left with none.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::RemoveFile(const char* szFileName)
{
	int					iFile;
	size				i;
	bool				bHierarchy;
	size				j;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;

	iFile = mcFiles.Get(szFileName, -1);
	if (iFile < 0)
	{
		return;
	}
	for (i = 0; i < mapcSymbols.NumElements(); i++)
	{
		pcSymbol = mapcSymbols.GetPtr(i);
		for (j = pcSymbol->masLocations.NumElements(); j > 0; j--)
		{
			psLocation = pcSymbol->masLocations.Get(j - 1);
			if (psLocation->uiFile == (uint32)iFile)
			{
				mcLocations.Remove(GetCPPIndexLocationKey(psLocation->uiFile, psLocation->iLine, psLocation->iColumn));
				pcSymbol->masLocations.RemoveAt(j - 1, true);
			}
		}
	}

	//The file's base classes and virtual functions go; the classes' and functions' are made again from the rest.
	bHierarchy = false;
	for (i = masBaseLinks.NumElements(); i > 0; i--)
	{
		if (masBaseLinks.Get(i - 1)->uiFile == (uint32)iFile)
		{
			mcHierarchyKeys.Remove(GetHierarchyKey(masBaseLinks.Get(i - 1)->iClass, masBaseLinks.Get(i - 1)->iBase, (uint32)iFile));
			masBaseLinks.RemoveAt(i - 1, true);
			bHierarchy = true;
		}
	}
	for (i = masVirtuals.NumElements(); i > 0; i--)
	{
		if (masVirtuals.Get(i - 1)->uiFile == (uint32)iFile)
		{
			mcHierarchyKeys.Remove(GetHierarchyKey(masVirtuals.Get(i - 1)->iFunction, -1, (uint32)iFile));
			masVirtuals.RemoveAt(i - 1, true);
			bHierarchy = true;
		}
	}
	if (bHierarchy)
	{
		RemakeHierarchy();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The key a base class link (iFirst the class, iSecond the base) or a
// virtual (iFirst the function, iSecond -1) is kept by, in a file.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPIndex::GetHierarchyKey(int32 iFirst, int32 iSecond, uint32 uiFile)
{
	return ((int64)iFirst << 42) ^ ((int64)(iSecond + 1) << 21) ^ (int64)uiFile;
}


//////////////////////////////////////////////////////////////////////////
//
// The class iClass has iBase as a base class, as file uiFile says.  Each
// is kept once, whichever units say so.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::AddBaseLink(int32 iClass, int32 iBase, uint32 uiFile)
{
	int64				lliKey;
	int32				iOne;
	SCPPIndexBaseLink*	psLink;
	CCPPIndexSymbol*	pcClass;

	lliKey = GetHierarchyKey(iClass, iBase, uiFile);
	if ((iClass == iBase) || mcHierarchyKeys.Get(lliKey))
	{
		return;
	}
	iOne = 1;
	mcHierarchyKeys.Put(lliKey, &iOne);
	psLink = masBaseLinks.Add();
	psLink->iClass = iClass;
	psLink->iBase = iBase;
	psLink->uiFile = uiFile;
	pcClass = mapcSymbols.GetPtr((size)iClass);
	if (pcClass->maiBases.Find(iBase) == ARRAY_ELEMENT_NOT_FOUND)
	{
		pcClass->maiBases.Add(iBase);
		mapcSymbols.GetPtr((size)iBase)->maiDerived.Add(iClass);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The member function iFunction is declared virtual, override or final in
// file uiFile.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::AddVirtual(int32 iFunction, uint32 uiFile)
{
	int64				lliKey;
	int32				iOne;
	SCPPIndexVirtual*	psVirtual;

	lliKey = GetHierarchyKey(iFunction, -1, uiFile);
	if (mcHierarchyKeys.Get(lliKey))
	{
		return;
	}
	iOne = 1;
	mcHierarchyKeys.Put(lliKey, &iOne);
	psVirtual = masVirtuals.Add();
	psVirtual->iFunction = iFunction;
	psVirtual->uiFile = uiFile;
	mapcSymbols.GetPtr((size)iFunction)->mbVirtual = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbols' base classes, derived classes and virtual flags made again
// from the links and virtuals the files say, after a file has gone.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::RemakeHierarchy(void)
{
	size				i;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexBaseLink*	psLink;

	for (i = 0; i < mapcSymbols.NumElements(); i++)
	{
		pcSymbol = mapcSymbols.GetPtr(i);
		pcSymbol->maiBases.ReInit();
		pcSymbol->maiDerived.ReInit();
		pcSymbol->mbVirtual = false;
	}
	for (i = 0; i < masBaseLinks.NumElements(); i++)
	{
		psLink = masBaseLinks.Get(i);
		pcSymbol = mapcSymbols.GetPtr((size)psLink->iClass);
		if (pcSymbol->maiBases.Find(psLink->iBase) == ARRAY_ELEMENT_NOT_FOUND)
		{
			pcSymbol->maiBases.Add(psLink->iBase);
			mapcSymbols.GetPtr((size)psLink->iBase)->maiDerived.Add(psLink->iClass);
		}
	}
	for (i = 0; i < masVirtuals.NumElements(); i++)
	{
		mapcSymbols.GetPtr((size)masVirtuals.Get(i)->iFunction)->mbVirtual = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The base classes of the classes pcNode defines, and the member
// functions it declares virtual, override or final.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::FindHierarchy(CCPPTokeniser* pcTokeniser, CCPPSymbolKey* pcKeys, CCPPSyntaxTree* pcTree, CCPPSyntax* pcNode, CCPPIndexUnit* pcDest)
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
	CCPPIndexHierarchyChildren	cChildren;
	CSourceFile*				pcFile;

	switch (pcNode->GetKind())
	{
	case CPPS_ClassSpecifier:
		pcClass = (CCPPSyntaxClassSpecifier*)pcNode;
		pcFile = (pcClass->mpcName && pcClass->mpcName->GetLast()) ? GetTokenFile(pcTokeniser, pcClass->mpcName->GetLast()->miName) : NULL;
		if (pcFile && pcClass->mpcScope && (pcClass->mpcScope->mapcBases.NumElements() > 0))
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
						pcDest->AddBase(szClass.Text(), eKind, szBase.Text(), pcBase->mpcOwner->meKind, pcFile);
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
				GatherVirtual(pcTokeniser, pcKeys, pcTree, (CCPPSyntaxDeclarator*)pcDeclarator, bVirtual, pcDest);
			}
		}
		break;
	case CPPS_FunctionDefinition:
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcNode;
		bVirtual = pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_VIRTUAL);
		GatherVirtual(pcTokeniser, pcKeys, pcTree, pcDefinition->mpcDeclarator, bVirtual, pcDest);
		break;
	default:
		break;
	}

	cChildren.mpcTokeniser = pcTokeniser;
	cChildren.mpcKeys = pcKeys;
	cChildren.mpcTree = pcTree;
	cChildren.mpcDest = pcDest;
	pcNode->VisitFields(&cChildren);
}


//////////////////////////////////////////////////////////////////////////
//
// The function pcDeclarator declares is virtual if bVirtual (it has the
// virtual specifier) or it is declared override or final.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::GatherVirtual(CCPPTokeniser* pcTokeniser, CCPPSymbolKey* pcKeys, CCPPSyntaxTree* pcTree, CCPPSyntaxDeclarator* pcDeclarator, bool bVirtual, CCPPIndexUnit* pcDest)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxName*				pcName;
	SCPPTokenSymbol*			psSymbol;
	CChars						szKey;
	CSourceFile*				pcFile;

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
	pcFile = GetTokenFile(pcTokeniser, pcName->GetLast()->miName);
	if ((psSymbol == NULL) || (psSymbol->pcSymbol == NULL) || !pcKeys->IsFunction(psSymbol->pcSymbol) || (pcFile == NULL))
	{
		return;
	}
	szKey.Init();
	if (pcKeys->GetFunctionKey(psSymbol->pcSymbol, pcDeclarator, &szKey))
	{
		pcDest->AddVirtual(szKey.Text(), psSymbol->pcSymbol->meKind, pcFile);
	}
	szKey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The file iToken was written in, where its macro was used if a macro
// made it; NULL if it isn't known.
//
//////////////////////////////////////////////////////////////////////////
CSourceFile* CCPPIndex::GetTokenFile(CCPPTokeniser* pcTokeniser, int32 iToken)
{
	SPreprocessedToken*	psSource;
	SPreprocessedToken	sMacroUse;

	if ((iToken < 0) || ((size)iToken >= pcTokeniser->NumTokens()))
	{
		return NULL;
	}
	psSource = pcTokeniser->GetSource(pcTokeniser->Get((size)iToken));
	if (psSource->iExpansion != -1)
	{
		return GetMacroUse(pcTokeniser, psSource, &sMacroUse) ? sMacroUse.pcFile : NULL;
	}
	return psSource->pcFile;
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
// If pcLinkage is extern "C" (any language but C++): what it declares is
// labelled by its name alone.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPIndex::IsCLinkage(CCPPTokeniser* pcTokeniser, CCPPSyntaxLinkageSpecification* pcLinkage)
{
	CChars		szLanguage;
	CCPPToken*	pcToken;
	bool		bC;

	szLanguage.Init();
	pcToken = pcTokeniser->Get(pcLinkage->miLanguage);
	if (pcToken)
	{
		pcToken->Append(&szLanguage, pcTokeniser);
	}
	bC = !szLanguage.Contains("C++");
	szLanguage.Kill();
	return bC;
}


//////////////////////////////////////////////////////////////////////////
//
// The token of pcName's last part names a declaration inside extern "C".
//
//////////////////////////////////////////////////////////////////////////
static void MarkCLinkage(CCPPSyntaxName* pcName, CArrayIndexTokens* pasTokens)
{
	SCPPIndexToken*	psToken;

	if (pcName && pcName->GetLast() && (pcName->GetLast()->miName >= 0))
	{
		psToken = pasTokens->SafeGet((size)pcName->GetLast()->miName);
		if (psToken)
		{
			psToken->bCLinkage = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the tree says about name tokens: the function declarators they
// name, the number of arguments a call passes the callee they name and
// whether they name what a definition defines.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::FindNames(CCPPTokeniser* pcTokeniser, CCPPSyntax* pcNode, CArrayIndexTokens* pasTokens, bool bInDefinition, bool bCLinkage)
{
	CCPPSyntaxName*			pcName;
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPSyntaxExpression*	pcCallee;
	SCPPIndexToken*			psToken;
	CCPPIndexNameChildren	cChildren;
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
		else if (bCLinkage)
		{
			//A variable declared inside extern "C".
			MarkCLinkage(pcDeclarator->GetName(), pasTokens);
		}
		break;
	case CPPS_LinkageSpecification:
		bCLinkage = IsCLinkage(pcTokeniser, (CCPPSyntaxLinkageSpecification*)pcNode);
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
				psToken->bCLinkage = psToken->bCLinkage || (bCLinkage && (iArguments == 0) && (pcNode->GetKind() != CPPS_ClassSpecifier) && (pcNode->GetKind() != CPPS_EnumSpecifier));
			}
		}
	}

	//A definition's own name is defined; what is inside it is only declared or used.
	if (pcNode->GetKind() == CPPS_FunctionDefinition)
	{
		bInDefinition = false;
	}
	cChildren.mpcTokeniser = pcTokeniser;
	cChildren.mpasTokens = pasTokens;
	cChildren.mbInDefinition = (pcNode->GetKind() == CPPS_ClassSpecifier) || (pcNode->GetKind() == CPPS_EnumSpecifier) ? false : bInDefinition;
	//A class's members, a function's parameters and what its body declares aren't the linkage's.
	cChildren.mbCLinkage = bCLinkage && (pcNode->GetKind() != CPPS_ClassSpecifier) && (pcNode->GetKind() != CPPS_FunctionDefinition) &&
						   (pcNode->GetKind() != CPPS_Parameter) && !((pcNode->GetKind() == CPPS_Declarator) && ((CCPPSyntaxDeclarator*)pcNode)->IsFunction());
	pcNode->VisitFields(&cChildren);
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
	ECPPIndexRole	eRole;
	int				iKey;

	pcSymbol = psSymbol->pcSymbol;
	if ((psSymbol->uiRole == CPPTR_Reference) && pcKeys->IsFunction(pcSymbol))
	{
		GatherReferenceToOverload(pcKeys, pcSymbol, psToken->iArguments - 1, psSource, (ECPPUsageType)psToken->uiUsage, pcDest);
		return;
	}

	iKey = GetUnitKey(pcKeys, pcSymbol, (psSymbol->uiRole == CPPTR_Declaration) ? psToken->pcDeclarator : NULL, pcDest);
	if (iKey >= 0)
	{
		if (psSymbol->uiRole == CPPTR_Reference)
		{
			eRole = CPPIR_Reference;
		}
		else
		{
			eRole = psToken->bDefinition ? CPPIR_Definition : CPPIR_Declaration;
		}
		pcDest->AddEntry(iKey, psSource, eRole, uiFlags, GetSymbolUsageType(pcSymbol, (ECPPUsageType)psToken->uiUsage));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The unit's key (AddKey) of pcSymbol, or of the function pcDeclarator
// declares if it isn't NULL, worked out once per unit: a name is mostly
// named many times.  -1 if it has no key.
//
//////////////////////////////////////////////////////////////////////////
int CCPPIndex::GetUnitKey(CCPPSymbolKey* pcKeys, CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator, CCPPIndexUnit* pcDest)
{
	int64	lliAddress;
	int32*	piCached;
	int32	iKey;
	CChars	szKey;
	bool	bKey;

	if (pcDeclarator && !pcKeys->IsFunction(pcSymbol))
	{
		pcDeclarator = NULL;
	}
	lliAddress = pcDeclarator ? (int64)(size)pcDeclarator : (int64)(size)pcSymbol;
	piCached = pcDest->mcCachedKeys.Get(lliAddress);
	if (piCached)
	{
		return *piCached;
	}

	szKey.Init();
	if (pcDeclarator)
	{
		bKey = pcKeys->GetFunctionKey(pcSymbol, pcDeclarator, &szKey);
	}
	else
	{
		bKey = pcKeys->GetKey(pcSymbol, &szKey);
	}
	iKey = bKey ? pcDest->AddKey(szKey.Text(), pcSymbol->meKind) : -1;
	szKey.Kill();
	pcDest->mcCachedKeys.Put(lliAddress, &iKey);
	return iKey;
}


//////////////////////////////////////////////////////////////////////////
//
// A name the semantic pass resolved: a reference to the member it names,
// or to the one overload a call calls.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::GatherResolved(CCPPSymbolKey* pcKeys, SCPPResolvedName* psResolved, SPreprocessedToken* psSource, ECPPUsageType eUsage, CCPPIndexUnit* pcDest)
{
	int	iKey;

	iKey = GetUnitKey(pcKeys, psResolved->pcSymbol, psResolved->pcDeclarator, pcDest);
	if (iKey >= 0)
	{
		pcDest->AddEntry(iKey, psSource, CPPIR_Reference, 0, GetSymbolUsageType(psResolved->pcSymbol, eUsage));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A use of a function name.  The parser finds a name's first overload,
// so the overloads declared with it that iArguments arguments fit (any,
// if iArguments is -1) are what it could call.  It is a reference to
// each, flagged ambiguous when there is more than one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPIndex::GatherReferenceToOverload(CCPPSymbolKey* pcKeys, CCPPSymbol* pcSymbol, int iArguments, SPreprocessedToken* psSource, ECPPUsageType eUsage, CCPPIndexUnit* pcDest)
{
	CArrayInt		aiCandidates;
	CArrayInt		aiAll;
	CCPPSymbol*		pcOverload;
	int				iKey;
	int				iMinimum;
	int				iMaximum;
	bool			bFits;
	CArrayInt*		paiChosen;
	size			i;

	aiCandidates.Init();
	aiAll.Init();
	for (pcOverload = pcSymbol; pcOverload; pcOverload = pcOverload->mpcNext)
	{
		if (!pcKeys->IsFunction(pcOverload))
		{
			continue;
		}
		iKey = GetUnitKey(pcKeys, pcOverload, NULL, pcDest);
		if (iKey >= 0)
		{
			if (aiAll.Find(iKey) == ARRAY_ELEMENT_NOT_FOUND)
			{
				aiAll.Add(iKey);
			}
			bFits = true;
			if ((iArguments >= 0) && pcKeys->GetParameterCounts(pcOverload, &iMinimum, &iMaximum))
			{
				bFits = (iArguments >= iMinimum) && ((iMaximum < 0) || (iArguments <= iMaximum));
			}
			if (bFits && (aiCandidates.Find(iKey) == ARRAY_ELEMENT_NOT_FOUND))
			{
				aiCandidates.Add(iKey);
			}
		}
	}

	paiChosen = (aiCandidates.NumElements() > 0) ? &aiCandidates : &aiAll;
	for (i = 0; i < paiChosen->NumElements(); i++)
	{
		pcDest->AddEntry(paiChosen->GetValue(i), psSource, CPPIR_Reference, (paiChosen->NumElements() > 1) ? CPPIF_AMBIGUOUS : 0, eUsage);
	}
	aiAll.Kill();
	aiCandidates.Kill();
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
			if (psLocation->uiFlags & CPPIF_C_LINKAGE)
			{
				pszDest->Append(" extern-C");
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

