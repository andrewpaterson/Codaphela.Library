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
#include "CTIdentifier.h"
#include "CppParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxTypeSpecifier* CCPPParser::ParseClassSpecifier(CCPPSyntax* pcParent, ECPPDeclarationContext eContext, bool bFriend)
{
	CCPPSyntaxClassSpecifier*			pcClass;
	CCPPSyntaxElaboratedTypeSpecifier*	pcElaborated;
	CCPPSyntaxNameComponent*			pcComponent;
	CCPPSyntaxAlignasSpecifier*			pcAlignas;
	CCPPSymbol*							pcSymbol;
	CCPPScope*							pcDeclarationScope;
	CCPPScope*							pcQualifier;
	CArraySyntax						apcAttributes;
	ECppTokens							eKey;
	ECppTokens							eDeclared;
	int									iFirstToken;
	int64								lliName;
	bool								bSimple;
	bool								bTemplate;

	//class-key attributes name final base-clause { members } or an elaborated type specifier.  [class] [dcl.type.elab]
	iFirstToken = CurrentToken();
	eKey = ((CCTReservedWord*)Peek(0))->GetToken();
	bTemplate = IsDeclaringTemplate();
	Next();

	//alignas is an attribute-specifier, so it can come before the name too.  [dcl.attr.grammar] 1
	apcAttributes.Init();
	for (;;)
	{
		ParseAttributes(&apcAttributes, pcParent);
		if (!Is(CT_alignas))
		{
			break;
		}
		pcAlignas = ParseAlignasSpecifier(pcParent);
		if (!pcAlignas)
		{
			apcAttributes.Kill();
			return NULL;
		}
		apcAttributes.Add(pcAlignas);
	}

	pcClass = mpcMemory->Create<CCPPSyntaxClassSpecifier>(pcParent);
	pcClass->miFirstToken = iFirstToken;
	pcClass->meKey = eKey;
	pcClass->miPack = miPack;
	pcClass->macpAttributes.InsertArrayAfterEnd(&apcAttributes);
	apcAttributes.Kill();

	pcQualifier = NULL;
	if (IsIdentifier() || Is(CT_ScopeResolution))
	{
		meNameLookup = CPPL_Type;
		pcClass->mpcName = ParseName(pcClass, true, &pcQualifier);
		if (!pcClass->mpcName)
		{
			return NULL;
		}
	}
	ParseAttributes(&pcClass->macpAttributes, pcClass);

	lliName = GetIdentifier();
	if ((lliName == mpcTokeniser->mlliFinal) && (IsAhead(1, CT_Colon) || IsAhead(1, CT_OpenCurlyBracket)))
	{
		pcClass->mbFinal = true;
		Next();
	}

	pcComponent = pcClass->mpcName ? pcClass->mpcName->GetLast() : NULL;
	bSimple = pcClass->mpcName && pcClass->mpcName->IsSimple();
	lliName = (pcComponent && (pcComponent->miName != -1)) ? GetTokenIdentifier(pcComponent->miName) : -1;
	pcDeclarationScope = mpcScope->GetDeclarationScope();

	if (!Is(CT_OpenCurlyBracket) && !Is(CT_Colon))
	{
		//An elaborated type specifier: class X; or class X* p;  An unknown name is declared.  [basic.scope.pdecl] 7
		if (!pcClass->mpcName)
		{
			Error("Expected a class name or a class body.");
			return NULL;
		}
		pcElaborated = mpcMemory->Create<CCPPSyntaxElaboratedTypeSpecifier>(pcParent);
		pcElaborated->miFirstToken = iFirstToken;
		pcElaborated->meKey = eKey;
		pcElaborated->macpAttributes.InsertArrayAfterEnd(&pcClass->macpAttributes);
		pcElaborated->mpcName = pcClass->mpcName;
		pcClass->mpcName->mpcParent = pcElaborated;

		pcSymbol = pcClass->mpcName->mpcSymbol;
		if (pcSymbol && pcSymbol->IsClass() && pcSymbol->mpcDeclaration && (miTentative == 0) && !IsSkimming(iFirstToken))
		{
			//A union is named with union, a class with class or struct.  [dcl.type.elab] 3
			eDeclared = (pcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier) ? ((CCPPSyntaxClassSpecifier*)pcSymbol->mpcDeclaration)->meKey :
						((pcSymbol->mpcDeclaration->GetKind() == CPPS_ElaboratedTypeSpecifier) ? ((CCPPSyntaxElaboratedTypeSpecifier*)pcSymbol->mpcDeclaration)->meKey : CT_class);
			if ((eDeclared == CT_union) != (eKey == CT_union))
			{
				ErrorAt(iFirstToken, (char*)"A union is named with union, and a class with class or struct");
			}
		}
		if (bSimple)
		{
			if (bFriend)
			{
				pcDeclarationScope = pcDeclarationScope->GetEnclosingNamespace();
			}
			if (Is(CT_SemiColon) || !pcSymbol)
			{
				pcSymbol = pcDeclarationScope->FindHere(lliName);
				pcSymbol = ReuseOrDeclare((pcSymbol && pcSymbol->IsClass()) ? pcSymbol : NULL, lliName, pcComponent->miName, bTemplate ? CPPSK_ClassTemplate : CPPSK_Class, pcElaborated, pcDeclarationScope);
			}
			pcElaborated->mpcName->mpcSymbol = pcSymbol;
			if (pcComponent)
			{
				pcComponent->mpcSymbol = pcSymbol;
			}
		}
		pcElaborated->miLastToken = PreviousToken();
		return pcElaborated;
	}

	//A class definition.
	pcSymbol = NULL;
	if (bSimple)
	{
		pcSymbol = pcDeclarationScope->FindHere(lliName);
		if (!pcSymbol || !pcSymbol->IsClass() || (pcSymbol->mpcMembers && (pcSymbol->mpcMembers->mpcOwner != pcSymbol)))
		{
			pcSymbol = Declare(lliName, pcComponent->miName, bTemplate ? CPPSK_ClassTemplate : CPPSK_Class, pcClass, pcDeclarationScope);
		}
		else
		{
			Resolve(pcComponent->miName, pcSymbol, CPPTR_Declaration);
			if ((miTentative == 0) && pcSymbol->mpcDeclaration && (pcSymbol->mpcDeclaration->GetKind() == CPPS_ClassSpecifier) && (pcSymbol->mpcDeclaration != pcClass))
			{
				//[basic.def.odr] 1
				ErrorAt(pcComponent->miName, "The class is defined twice");
			}
			if (miTentative == 0)
			{
				pcSymbol->mpcDeclaration = pcClass;
			}
		}
	}
	else if (pcClass->mpcName)
	{
		//A nested class defined outside its class, or a specialisation, X<int>.
		pcSymbol = pcClass->mpcName->mpcSymbol;
	}
	pcClass->mpcSymbol = pcSymbol;
	if (pcComponent)
	{
		pcComponent->mpcSymbol = pcSymbol;
	}

	//A symbol declared before is only changed by the real parse: a tentative one is rewound, its class dropped.
	//A nested class defined outside its class is in the scope it is defined in (whose template parameters it names),
	//the class it is a member of searched first, as a member function's defined outside is.
	pcClass->mpcScope = mpcSymbols->CreateScope(CPPSC_Class, mpcScope, pcSymbol);
	pcClass->mpcScope->mpcQualifier = pcQualifier;
	if (pcSymbol && (miTentative == 0))
	{
		if (!pcSymbol->mpcMembers || bSimple)
		{
			pcSymbol->mpcMembers = pcClass->mpcScope;
		}
		//The injected class name.  [class] 2
		pcClass->mpcScope->AddAlias(pcSymbol->mlliName, pcSymbol, -1);
	}
	if (bTemplate || mpcScope->IsDependent())
	{
		pcClass->mpcScope->mbDependent = true;
	}

	if (Is(CT_Colon))
	{
		if (!ParseBaseSpecifiers(pcClass))
		{
			return NULL;
		}
	}

	if (!ParseClassBody(pcClass))
	{
		return NULL;
	}
	pcClass->miLastToken = PreviousToken();
	return pcClass;
}


//Walks a node's descendants for a name that is a template parameter.
class CCPPTemplateParameterFinder : public CCPPSyntaxFields
{
public:
	bool	mbFound;

	CCPPTemplateParameterFinder(void) { mbFound = false; }
	void	NodeField(CCPPSyntax** ppcNode) override
	{
		if (*ppcNode && !mbFound)
		{
			(*ppcNode)->VisitFields(this);
		}
	}
	void	NodesField(CArraySyntax* papcNodes) override
	{
		size	i;

		for (i = 0; !mbFound && (i < papcNodes->NumElements()); i++)
		{
			NodeField(papcNodes->Get(i));
		}
	}
	void	SymbolField(CCPPSymbol** ppcSymbol) override
	{
		if (*ppcSymbol && (((*ppcSymbol)->meKind == CPPSK_TypeParameter) || ((*ppcSymbol)->meKind == CPPSK_ValueParameter) || ((*ppcSymbol)->meKind == CPPSK_TemplateTemplateParameter)))
		{
			mbFound = true;
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Whether a node names a template parameter anywhere in it: a base class
// named so is a dependent base.  [temp.dep.type]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::NamesTemplateParameter(CCPPSyntax* pcNode)
{
	CCPPTemplateParameterFinder	cFinder;

	cFinder.NodeField(&pcNode);
	return cFinder.mbFound;
}


//////////////////////////////////////////////////////////////////////////
//
// A class's bases.  One that depends on a template parameter is marked in
// the class's scope (CCPPScope::IsDependentBase): a name that isn't
// qualified isn't looked for in it.  [temp.dep] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ParseBaseSpecifiers(CCPPSyntaxClassSpecifier* pcClass)
{
	CCPPSyntaxBaseSpecifier*	pcBase;
	CCPPSymbol*					pcSymbol;

	//: base-specifier, ...  [class.derived]
	Expect(CT_Colon);
	for (;;)
	{
		pcBase = mpcMemory->Create<CCPPSyntaxBaseSpecifier>(pcClass);
		pcBase->miFirstToken = CurrentToken();
		ParseAttributes(&pcBase->macpAttributes, pcBase);
		for (;;)
		{
			if (Accept(CT_virtual))
			{
				pcBase->mbVirtual = true;
			}
			else if (Accept(CT_public))
			{
				pcBase->meAccess = CPPA_Public;
			}
			else if (Accept(CT_protected))
			{
				pcBase->meAccess = CPPA_Protected;
			}
			else if (Accept(CT_private))
			{
				pcBase->meAccess = CPPA_Private;
			}
			else
			{
				break;
			}
		}

		pcBase->mpcName = ParseName(pcBase, false, NULL);
		if (!pcBase->mpcName)
		{
			return false;
		}
		pcBase->mbPack = Accept(CT_Elipsis);
		pcBase->miLastToken = PreviousToken();
		pcClass->macpBases.Add(pcBase);

		pcSymbol = pcBase->mpcName->mpcSymbol;
		if (pcSymbol && pcSymbol->mpcMembers && (pcSymbol->mpcMembers != pcClass->mpcScope))
		{
			if (NamesTemplateParameter(pcBase->mpcName))
			{
				pcClass->mpcScope->SetDependentBase(pcClass->mpcScope->mapcBases.NumElements());
			}
			pcClass->mpcScope->mapcBases.Add(pcSymbol->mpcMembers);
		}
		else
		{
			//A base that depends on a template parameter, or that is unknown: names can be missing from the class.
			pcClass->mpcScope->mbDependent = true;
		}

		if (!Accept(CT_Comma))
		{
			break;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ParseClassBody(CCPPSyntaxClassSpecifier* pcClass)
{
	CCPPSyntaxAccessSpecifier*		pcAccess;
	CCPPSyntaxDeclaration*			pcMember;
	CCPPSyntaxSkippedDeclaration*	pcSkipped;
	SCPPParserPosition				sStart;
	SCPPParserState					sOutside;
	SCPPParserState					sMembers;

	//{ member-specification }  [class.mem]
	if (!Expect(CT_OpenCurlyBracket))
	{
		return false;
	}

	sOutside = SaveState();
	mpcScope = pcClass->mpcScope;
	mbDeclaringTemplate = false;
	mbNoGreater = false;
	miClassDepth++;
	sMembers = SaveState();

	while (!IsEnd() && !Is(CT_CloseCurlyBracket))
	{
		if ((Is(CT_public) || Is(CT_protected) || Is(CT_private)) && IsAhead(1, CT_Colon))
		{
			pcAccess = mpcMemory->Create<CCPPSyntaxAccessSpecifier>(pcClass);
			pcAccess->meAccess = Is(CT_public) ? CPPA_Public : (Is(CT_protected) ? CPPA_Protected : CPPA_Private);
			pcAccess->SetTokens(CurrentToken(), CurrentToken() + 1);
			Next();
			Next();
			pcClass->macpMembers.Add(pcAccess);
			continue;
		}
		if (SkipPragma())
		{
			continue;
		}

		sStart = Mark();
		pcMember = ParseDeclaration(pcClass, CPPDC_Class);
		if (pcMember)
		{
			pcClass->macpMembers.Add(pcMember);
		}
		else
		{
			RecoverAt(sStart, &sMembers, false);
			pcSkipped = mpcMemory->Create<CCPPSyntaxSkippedDeclaration>(pcClass);
			pcSkipped->SetTokens(sStart.iToken, PreviousToken());
			pcClass->macpMembers.Add(pcSkipped);
		}
	}

	RestoreState(&sOutside);
	if (!Expect(CT_CloseCurlyBracket))
	{
		return false;
	}
	CheckClassMembers(pcClass);

	if (!pcClass->mpcName && (miTentative == 0))
	{
		//The members of an anonymous union or struct are members of the enclosing class.  [class.union.anon]
		mpcScope->mapcUsings.Add(pcClass->mpcScope);
	}

	if (miClassDepth == 0)
	{
		ParseDeferredBodies();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::ParseDeferredBodies(void)
{
	SCPPDeferredBody	sDeferred;
	SCPPParserPosition	sPosition;
	SCPPParserPosition	sBody;
	SCPPParserState		sState;
	CCPPSyntax*			pcDefault;

	//The bodies of member functions defined in a class, now that the class is complete.  [class.mem] 6  A class in one
	//of them comes back here when it ends, and takes the rest: this loop then has none left.
	while (muiDeferredNext < macDeferred.NumElements())
	{
		sDeferred = *macDeferred.Get(muiDeferredNext);
		muiDeferredNext++;

		sPosition = Mark();
		sState = SaveState();
		mbNoGreater = false;

		sBody.iToken = sDeferred.iStart;
		sBody.bHalf = false;
		Rewind(sBody);
		if (sDeferred.pcParameter)
		{
			//A default argument, parsed again over the same tokens in its function's scope, now finding the members
			//declared after it.  What doesn't end where it did the first time keeps the first parse.
			mpcScope = sDeferred.pcScope;
			pcDefault = ParseInitializerClause(sDeferred.pcParameter);
			if (pcDefault && (PreviousToken() == sDeferred.iEnd))
			{
				sDeferred.pcParameter->mpcDefault = pcDefault;
			}
		}
		else if (ParseFunctionBody(sDeferred.pcFunction))
		{
			if (PreviousToken() != sDeferred.iEnd)
			{
				ErrorAt(sDeferred.iStart, "The function body did not end where it was expected to.");
			}
		}
		else
		{
			sDeferred.pcFunction->meBody = CPPFB_Skipped;
		}

		Rewind(sPosition);
		RestoreState(&sState);
	}
	macDeferred.FakeSetUsedElements(0);
	muiDeferredNext = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxTypeSpecifier* CCPPParser::ParseEnumSpecifier(CCPPSyntax* pcParent, ECPPDeclarationContext eContext)
{
	CCPPSyntaxEnumSpecifier*			pcEnum;
	CCPPSyntaxElaboratedTypeSpecifier*	pcElaborated;
	CCPPSyntaxEnumerator*				pcEnumerator;
	CCPPSyntaxNameComponent*			pcComponent;
	CCPPSymbol*							pcSymbol;
	CCPPSymbol*							pcEnumeratorSymbol;
	CCPPScope*							pcDeclarationScope;
	SCPPParserState						sOutside;
	int64								lliName;

	//enum [class] name [: type] { enumerators }  [dcl.enum]
	pcEnum = mpcMemory->Create<CCPPSyntaxEnumSpecifier>(pcParent);
	pcEnum->miFirstToken = CurrentToken();
	Expect(CT_enum);
	if (Is(CT_class) || Is(CT_struct))
	{
		pcEnum->mbScoped = true;
		pcEnum->meScopeKey = ((CCTReservedWord*)Peek(0))->GetToken();
		Next();
	}
	ParseAttributes(&pcEnum->macpAttributes, pcEnum);

	if (IsIdentifier() || Is(CT_ScopeResolution))
	{
		meNameLookup = CPPL_Type;
		pcEnum->mpcName = ParseName(pcEnum, true, NULL);
		if (!pcEnum->mpcName)
		{
			return NULL;
		}
	}

	if (Is(CT_Colon))
	{
		Next();
		pcEnum->mpcBase = ParseTypeId(pcEnum);
		if (!pcEnum->mpcBase)
		{
			return NULL;
		}
	}

	pcComponent = pcEnum->mpcName ? pcEnum->mpcName->GetLast() : NULL;
	lliName = (pcComponent && (pcComponent->miName != -1) && pcEnum->mpcName->IsSimple()) ? GetTokenIdentifier(pcComponent->miName) : -1;
	pcDeclarationScope = mpcScope->GetDeclarationScope();

	if (!Is(CT_OpenCurlyBracket))
	{
		//enum X; or enum X x;
		if (!pcEnum->mpcName)
		{
			Error("Expected an enum name or body.");
			return NULL;
		}
		if (!pcEnum->mbScoped && !pcEnum->mpcBase && Is(CT_SemiColon) && (miTentative == 0) && !IsSkimming(pcEnum->miFirstToken))
		{
			//An unscoped enum declared without its body gives its type: enum E : int;  [dcl.enum] 2
			ErrorAt(pcEnum->miFirstToken, (char*)"An enum declared without its enumerators is a scoped one, or gives its underlying type");
		}
		pcElaborated = mpcMemory->Create<CCPPSyntaxElaboratedTypeSpecifier>(pcParent);
		pcElaborated->miFirstToken = pcEnum->miFirstToken;
		pcElaborated->meKey = CT_enum;
		pcElaborated->mpcName = pcEnum->mpcName;
		pcEnum->mpcName->mpcParent = pcElaborated;
		if ((lliName != -1) && !pcEnum->mpcName->mpcSymbol)
		{
			pcEnum->mpcName->mpcSymbol = Declare(lliName, pcComponent->miName, CPPSK_Enum, pcElaborated, pcDeclarationScope);
		}
		if (pcEnum->mbScoped || pcEnum->mpcBase)
		{
			CheckEnumRedeclaration(pcEnum->mpcName->mpcSymbol, pcEnum);
		}
		pcElaborated->miLastToken = PreviousToken();
		return pcElaborated;
	}

	pcSymbol = NULL;
	if (lliName != -1)
	{
		pcSymbol = pcDeclarationScope->FindHere(lliName);
		pcSymbol = ReuseOrDeclare((pcSymbol && (pcSymbol->meKind == CPPSK_Enum)) ? pcSymbol : NULL, lliName, pcComponent->miName, CPPSK_Enum, pcEnum, pcDeclarationScope);
		CheckEnumRedeclaration(pcSymbol, pcEnum);
	}
	else if (pcEnum->mpcName)
	{
		pcSymbol = pcEnum->mpcName->mpcSymbol;
	}
	pcEnum->mpcSymbol = pcSymbol;
	if (pcEnum->mpcName)
	{
		pcEnum->mpcName->mpcSymbol = pcSymbol;
	}

	pcEnum->mpcScope = mpcSymbols->CreateScope(CPPSC_Enum, mpcScope, pcSymbol);
	if (pcSymbol && (miTentative == 0))
	{
		pcSymbol->mpcMembers = pcEnum->mpcScope;
	}

	Next();
	pcEnum->mbBody = true;
	sOutside = SaveState();
	mpcScope = pcEnum->mpcScope;
	mbNoGreater = false;
	while (!Is(CT_CloseCurlyBracket))
	{
		if (!IsIdentifier())
		{
			RestoreState(&sOutside);
			Error("Expected an enumerator.");
			return NULL;
		}
		pcEnumerator = mpcMemory->Create<CCPPSyntaxEnumerator>(pcEnum);
		pcEnumerator->miFirstToken = CurrentToken();
		pcEnumerator->miName = CurrentToken();
		lliName = GetIdentifier();
		Next();

		ParseAttributes(&pcEnumerator->macpAttributes, pcEnumerator);
		if (Accept(CT_Assignment))
		{
			pcEnumerator->mpcValue = ParseConstantExpression(pcEnumerator);
			if (!pcEnumerator->mpcValue)
			{
				RestoreState(&sOutside);
				return NULL;
			}
		}

		//An enumerator is declared after its definition, so its own value names the x outside in enum { x = x };, and
		//it can be used in the values of the ones after it.  [basic.scope.pdecl] 3
		CheckRedeclaration(lliName, CPPSK_Enumerator, false, pcEnumerator->miName, pcEnum->mbScoped ? pcEnum->mpcScope : pcDeclarationScope);
		pcEnumeratorSymbol = Declare(lliName, pcEnumerator->miName, CPPSK_Enumerator, pcEnumerator, pcEnum->mpcScope);
		pcEnumerator->mpcSymbol = pcEnumeratorSymbol;
		if (pcEnumeratorSymbol && !pcEnum->mbScoped)
		{
			pcDeclarationScope->AddAlias(lliName, pcEnumeratorSymbol, -1);
			if (mpapcRecorded && (pcDeclarationScope == mpcRecordScope))
			{
				mpapcRecorded->Add(&pcEnumeratorSymbol);
			}
		}
		pcEnumerator->miLastToken = PreviousToken();
		pcEnum->macpEnumerators.Add(pcEnumerator);

		if (!Accept(CT_Comma))
		{
			break;
		}
		if (Is(CT_CloseCurlyBracket))
		{
			pcEnum->mbTrailingComma = true;
		}
	}
	RestoreState(&sOutside);

	if (!Expect(CT_CloseCurlyBracket))
	{
		return NULL;
	}
	pcEnum->miLastToken = PreviousToken();
	return pcEnum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseTemplateDeclaration(CCPPSyntax* pcParent, ECPPDeclarationContext eContext)
{
	CCPPSyntaxTemplateDeclaration*	pcTemplate;
	CCPPScope*						pcScope;
	bool							bDeclaringTemplate;

	//template < parameters > declaration, template<> for a specialisation and template declaration for an explicit
	//instantiation.  [temp] [temp.expl.spec] [temp.explicit]
	pcTemplate = mpcMemory->Create<CCPPSyntaxTemplateDeclaration>(pcParent);
	pcTemplate->miFirstToken = CurrentToken();
	pcTemplate->mbExtern = Accept(CT_extern);
	Expect(CT_template);

	bDeclaringTemplate = mbDeclaringTemplate;
	if (!Is(CT_LessThan))
	{
		pcTemplate->mbInstantiation = true;
		mbDeclaringTemplate = false;
		pcTemplate->mpcDeclaration = ParseDeclaration(pcTemplate, eContext);
		mbDeclaringTemplate = bDeclaringTemplate;
		if (!pcTemplate->mpcDeclaration)
		{
			return NULL;
		}
		pcTemplate->miLastToken = PreviousToken();
		return pcTemplate;
	}

	pcScope = PushScope(CPPSC_Template, NULL);
	pcTemplate->mpcScope = pcScope;
	if (!ParseTemplateParameters(pcTemplate))
	{
		PopScope(pcScope);
		return NULL;
	}

	//template<> declares specialisations, not templates.
	mbDeclaringTemplate = pcTemplate->macpParameters.NumElements() > 0;
	if (mbDeclaringTemplate)
	{
		miTemplates++;
	}
	pcTemplate->mpcDeclaration = ParseDeclaration(pcTemplate, eContext);
	if (pcTemplate->macpParameters.NumElements() > 0)
	{
		miTemplates--;
	}
	mbDeclaringTemplate = bDeclaringTemplate;
	PopScope(pcScope);
	if (!pcTemplate->mpcDeclaration)
	{
		return NULL;
	}
	pcTemplate->miLastToken = PreviousToken();
	if (pcTemplate->macpParameters.NumElements() > 0)
	{
		CheckTemplateDeclaration(pcTemplate);
	}
	return pcTemplate;
}


//Walks a node's descendants for a name, written alone, of the identifier.
class CCPPNameFinder : public CCPPSyntaxFields
{
public:
	int64			mlliName;
	CCPPParser*		mpcParser;
	bool			mbFound;

	void	NodeField(CCPPSyntax** ppcNode) override
	{
		CCPPSyntaxName*	pcName;

		if (!*ppcNode || mbFound)
		{
			return;
		}
		if ((*ppcNode)->GetKind() == CPPS_Name)
		{
			pcName = (CCPPSyntaxName*)*ppcNode;
			if (!pcName->mbGlobal && (pcName->macpComponents.NumElements() >= 1) && (((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0))->meKind == CPPNC_Identifier) && (mpcParser->TokenIdentifier(((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0))->miName) == mlliName))
			{
				mbFound = true;
				return;
			}
		}
		(*ppcNode)->VisitFields(this);
	}
	void	NodesField(CArraySyntax* papcNodes) override
	{
		size	i;

		for (i = 0; !mbFound && (i < papcNodes->NumElements()); i++)
		{
			NodeField(papcNodes->Get(i));
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A class template declared again (template<class T = int> struct A;
// then its definition) gives none of its parameters a default the first
// declaration gave.  [temp.param] 12
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckDefaultsOnce(CCPPSyntaxTemplateDeclaration* pcTemplate, CCPPSymbol* pcSymbol)
{
	CCPPSyntaxTemplateDeclaration**	ppcFirst;
	CCPPSyntaxTemplateDeclaration*	pcFirstTemplate;
	CCPPSyntax*						pcParameter;
	CCPPSyntax*						pcFirstParameter;
	size							i;
	bool							bDefault;
	bool							bFirstDefault;

	if (!pcSymbol || (pcSymbol->meKind != CPPSK_ClassTemplate) || (miTentative > 0))
	{
		return;
	}
	ppcFirst = mcFirstClassTemplates.Get((int64)(size)pcSymbol);
	if (!ppcFirst)
	{
		mcFirstClassTemplates.Put((int64)(size)pcSymbol, &pcTemplate);
		return;
	}
	pcFirstTemplate = *ppcFirst;
	if ((pcFirstTemplate == pcTemplate) || (pcFirstTemplate->miFirstToken == pcTemplate->miFirstToken) || (pcFirstTemplate->mpcTree != pcTemplate->mpcTree) || (pcFirstTemplate->macpParameters.NumElements() != pcTemplate->macpParameters.NumElements()))
	{
		return;
	}
	for (i = 0; i < pcTemplate->macpParameters.NumElements(); i++)
	{
		pcParameter = pcTemplate->macpParameters.GetPtr(i);
		pcFirstParameter = pcFirstTemplate->macpParameters.GetPtr(i);
		bDefault = ((pcParameter->GetKind() == CPPS_TypeParameter) && ((CCPPSyntaxTypeParameter*)pcParameter)->mpcDefault) || ((pcParameter->GetKind() == CPPS_Parameter) && ((CCPPSyntaxParameter*)pcParameter)->mpcDefault);
		bFirstDefault = ((pcFirstParameter->GetKind() == CPPS_TypeParameter) && ((CCPPSyntaxTypeParameter*)pcFirstParameter)->mpcDefault) || ((pcFirstParameter->GetKind() == CPPS_Parameter) && ((CCPPSyntaxParameter*)pcFirstParameter)->mpcDefault);
		if (bDefault && bFirstDefault)
		{
			ErrorAt(pcParameter->miFirstToken, (char*)"A template parameter's default is given once, where the template is first declared to have it");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A partial specialisation's arguments aren't the primary template's
// parameters as they are (A<T, U> of template<class T, class U> struct A),
// and a non-type argument it specialises isn't for a parameter whose type
// is the primary template's type parameter (template<class T, T V> struct
// C; then C<T, 1>).  [temp.class.spec] 8
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckPartialSpecialisation(CCPPSyntaxTemplateDeclaration* pcTemplate, CCPPSyntaxName* pcName)
{
	CCPPSyntaxTemplateDeclaration**	ppcPrimary;
	CCPPSyntaxNameComponent*		pcComponent;
	CCPPSyntax*						pcArgument;
	CCPPSyntax*						pcParameter;
	CCPPSyntax*						pcPrimaryParameter;
	CCPPSymbol*						pcArgumentSymbol;
	CCPPSymbol*						pcParameterSymbol;
	size							i;
	bool							bSame;

	pcComponent = pcName->GetLast();
	if ((miTentative > 0) || !pcName->mpcSymbol)
	{
		return;
	}
	bSame = pcComponent->macpTemplateArguments.NumElements() == pcTemplate->macpParameters.NumElements();
	for (i = 0; bSame && (i < pcComponent->macpTemplateArguments.NumElements()); i++)
	{
		pcArgument = pcComponent->macpTemplateArguments.GetPtr(i);
		pcParameter = pcTemplate->macpParameters.GetPtr(i);
		pcArgumentSymbol = ArgumentSymbol(pcArgument);
		pcParameterSymbol = (pcParameter->GetKind() == CPPS_TypeParameter) ? ((CCPPSyntaxTypeParameter*)pcParameter)->mpcSymbol : ((pcParameter->GetKind() == CPPS_Parameter) ? ((CCPPSyntaxParameter*)pcParameter)->mpcSymbol : NULL);
		bSame = pcArgumentSymbol && (pcArgumentSymbol == pcParameterSymbol);
	}
	if (bSame)
	{
		ErrorAt(pcComponent->miFirstToken, (char*)"A partial specialisation's arguments are not the primary template's parameters as they are");
		return;
	}

	ppcPrimary = mcFirstClassTemplates.Get((int64)(size)pcName->mpcSymbol);
	for (i = 0; ppcPrimary && (i < pcComponent->macpTemplateArguments.NumElements()) && (i < (*ppcPrimary)->macpParameters.NumElements()); i++)
	{
		pcArgument = pcComponent->macpTemplateArguments.GetPtr(i);
		pcPrimaryParameter = (*ppcPrimary)->macpParameters.GetPtr(i);
		if ((pcPrimaryParameter->GetKind() == CPPS_Parameter) && !ArgumentSymbol(pcArgument) && ((CCPPSyntaxParameter*)pcPrimaryParameter)->mpcSpecifiers &&
			NamesTemplateParameter(((CCPPSyntaxParameter*)pcPrimaryParameter)->mpcSpecifiers))
		{
			ErrorAt(pcArgument->miFirstToken, (char*)"A specialised non-type argument's parameter has no type that depends on the specialisation's parameters");
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol a template argument is just the name of (T, N), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPParser::ArgumentSymbol(CCPPSyntax* pcArgument)
{
	CCPPSyntaxTypeId*			pcTypeId;
	CCPPSyntaxTypeSpecifier*	pcType;

	if (pcArgument->GetKind() == CPPS_IdExpression)
	{
		return ((CCPPSyntaxIdExpression*)pcArgument)->mpcName->mpcSymbol;
	}
	if (pcArgument->GetKind() != CPPS_TypeId)
	{
		return NULL;
	}
	pcTypeId = (CCPPSyntaxTypeId*)pcArgument;
	if (pcTypeId->mbPack || (pcTypeId->mpcDeclarator && !pcTypeId->mpcDeclarator->IsEmpty()) || !pcTypeId->mpcSpecifiers || (pcTypeId->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
	{
		return NULL;
	}
	pcType = pcTypeId->mpcSpecifiers->mpcType;
	return (pcType && (pcType->GetKind() == CPPS_NamedTypeSpecifier)) ? ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName->mpcSymbol : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What a template's parameters and declaration may not be.  A parameter
// pack has no default; in a class, alias or variable template a default is
// followed only by defaults (or a pack), and a pack is the last parameter;
// a partial specialisation's have no defaults.  A non-type parameter isn't
// of a class type.  A member function template isn't virtual.  An alias
// template's type doesn't name the alias template.  [temp.param] 7, 9, 11,
// [temp.class.spec] 8, [temp.mem] 3, [temp.alias] 4
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckTemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplate)
{
	CCPPSyntax*						pcDeclaration;
	CCPPSyntax*						pcParameter;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxParameter*			pcValue;
	CCPPSyntaxTypeParameter*		pcType;
	CCPPSyntaxClassSpecifier*		pcClass;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSymbol*						pcTypeSymbol;
	CCPPNameFinder					cFinder;
	size							i;
	bool							bDefault;
	bool							bPack;
	bool							bDefaulted;
	bool							bClassLike;
	bool							bPartial;

	pcDeclaration = pcTemplate->mpcDeclaration;
	pcSpecifiers = NULL;
	bClassLike = false;
	bPartial = false;
	if (pcDeclaration->GetKind() == CPPS_AliasDeclaration)
	{
		bClassLike = true;
		cFinder.mlliName = TokenIdentifier(((CCPPSyntaxAliasDeclaration*)pcDeclaration)->miName);
		cFinder.mpcParser = this;
		cFinder.mbFound = false;
		cFinder.NodeField((CCPPSyntax**)&((CCPPSyntaxAliasDeclaration*)pcDeclaration)->mpcType);
		if (cFinder.mbFound)
		{
			ErrorAt(pcDeclaration->miFirstToken, (char*)"An alias template's type can't name the alias template");
		}
	}
	else if (pcDeclaration->GetKind() == CPPS_SimpleDeclaration)
	{
		pcSimple = (CCPPSyntaxSimpleDeclaration*)pcDeclaration;
		pcSpecifiers = pcSimple->mpcSpecifiers;
		pcInit = (pcSimple->macpDeclarators.NumElements() > 0) ? (CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(0) : NULL;
		if (!pcInit && pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			bClassLike = true;
			pcClass = (CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType;
			bPartial = pcClass->mpcName && pcClass->mpcName->GetLast() && pcClass->mpcName->GetLast()->mbTemplateArguments;
			if (pcClass->mpcName && !bPartial)
			{
				CheckDefaultsOnce(pcTemplate, pcClass->mpcSymbol);
			}
			else if (bPartial)
			{
				CheckPartialSpecialisation(pcTemplate, pcClass->mpcName);
			}
		}
		else if (!pcInit && pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ElaboratedTypeSpecifier) && ((CCPPSyntaxElaboratedTypeSpecifier*)pcSpecifiers->mpcType)->mpcName)
		{
			CheckDefaultsOnce(pcTemplate, ((CCPPSyntaxElaboratedTypeSpecifier*)pcSpecifiers->mpcType)->mpcName->mpcSymbol);
		}
		else if (pcInit && pcInit->mpcDeclarator && !pcInit->mpcDeclarator->IsFunction())
		{
			bClassLike = true;
		}
	}
	else if (pcDeclaration->GetKind() == CPPS_FunctionDefinition)
	{
		pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcDeclaration)->mpcSpecifiers;
	}
	if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_VIRTUAL))
	{
		ErrorAt(pcDeclaration->miFirstToken, (char*)"A member function template can't be virtual");
	}

	bDefaulted = false;
	for (i = 0; i < pcTemplate->macpParameters.NumElements(); i++)
	{
		pcParameter = pcTemplate->macpParameters.GetPtr(i);
		bDefault = false;
		bPack = false;
		if (pcParameter->GetKind() == CPPS_TypeParameter)
		{
			pcType = (CCPPSyntaxTypeParameter*)pcParameter;
			bDefault = pcType->mpcDefault != NULL;
			bPack = pcType->mbPack;
		}
		else if (pcParameter->GetKind() == CPPS_Parameter)
		{
			pcValue = (CCPPSyntaxParameter*)pcParameter;
			bDefault = pcValue->mpcDefault != NULL;
			bPack = pcValue->mpcDeclarator && pcValue->mpcDeclarator->mbPack;
			pcTypeSymbol = (pcValue->mpcSpecifiers && pcValue->mpcSpecifiers->mpcType && (pcValue->mpcSpecifiers->mpcType->GetKind() == CPPS_NamedTypeSpecifier)) ? ((CCPPSyntaxNamedTypeSpecifier*)pcValue->mpcSpecifiers->mpcType)->mpcName->mpcSymbol : NULL;
			if (pcTypeSymbol && ((pcTypeSymbol->meKind == CPPSK_Class) || (pcTypeSymbol->meKind == CPPSK_ClassTemplate)) && !(pcValue->mpcDeclarator && ((pcValue->mpcDeclarator->NumPointers() > 0) || pcValue->mpcDeclarator->mpcNested)))
			{
				ErrorAt(pcParameter->miFirstToken, (char*)"A non-type template parameter isn't of a class type");
			}
		}
		if (bPack && bDefault)
		{
			ErrorAt(pcParameter->miFirstToken, (char*)"A template parameter pack has no default");
		}
		else if (bPartial && bDefault)
		{
			ErrorAt(pcParameter->miFirstToken, (char*)"A partial specialisation's template parameters have no defaults");
		}
		else if (bClassLike && !bPartial && bDefaulted && !bDefault && !bPack)
		{
			ErrorAt(pcParameter->miFirstToken, (char*)"A template parameter after one with a default has a default too");
		}
		else if (bClassLike && !bPartial && bPack && (i + 1 < pcTemplate->macpParameters.NumElements()))
		{
			ErrorAt(pcParameter->miFirstToken, (char*)"A class template's parameter pack is its last parameter");
		}
		bDefaulted |= bDefault;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// template<class> class C (= Default): a template parameter that is a class
// template, with template parameters of its own, in a scope of their own.
// [temp.param] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPParser::ParseTemplateTemplateParameter(CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeParameter*	pcType;
	CCPPScope*					pcScope;
	int64						lliName;
	bool						bParameters;

	pcType = mpcMemory->Create<CCPPSyntaxTypeParameter>(pcParent);
	pcType->miFirstToken = CurrentToken();
	pcType->mpcTemplate = mpcMemory->Create<CCPPSyntaxTemplateDeclaration>(pcType);
	pcType->mpcTemplate->miFirstToken = CurrentToken();
	Expect(CT_template);
	pcScope = PushScope(CPPSC_Template, NULL);
	pcType->mpcTemplate->mpcScope = pcScope;
	bParameters = ParseTemplateParameters(pcType->mpcTemplate);
	PopScope(pcScope);
	pcType->mpcTemplate->miLastToken = PreviousToken();
	if (!bParameters || (!Is(CT_class) && !Is(CT_typename)))
	{
		Error("Expected class after a template template parameter's parameters.");
		return NULL;
	}
	pcType->mbTypename = Is(CT_typename);
	Next();
	pcType->mbPack = Accept(CT_Elipsis);
	if (IsIdentifier())
	{
		pcType->miName = CurrentToken();
		lliName = GetIdentifier();
		Next();
		pcType->mpcSymbol = Declare(lliName, pcType->miName, CPPSK_TemplateTemplateParameter, pcType, mpcScope);
	}
	if (Accept(CT_Assignment))
	{
		pcType->mpcDefault = ParseTypeId(pcType);
		if (!pcType->mpcDefault)
		{
			return NULL;
		}
	}
	pcType->miLastToken = PreviousToken();
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ParseTemplateParameters(CCPPSyntaxTemplateDeclaration* pcTemplate)
{
	CCPPSyntax*		pcParameter;
	bool			bNoGreater;

	//< template-parameter-list >  [temp.param]
	if (!Expect(CT_LessThan))
	{
		return false;
	}
	bNoGreater = mbNoGreater;
	mbNoGreater = true;
	while (!IsCloseAngle())
	{
		pcParameter = ParseTemplateParameter(pcTemplate);
		if (!pcParameter)
		{
			mbNoGreater = bNoGreater;
			return false;
		}
		pcTemplate->macpParameters.Add(pcParameter);
		if (!Accept(CT_Comma))
		{
			break;
		}
	}
	mbNoGreater = bNoGreater;
	if (!AcceptCloseAngle())
	{
		return Error("Expected '>' to end the template parameters.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPParser::ParseTemplateParameter(CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeParameter*	pcType;
	CCPPSyntaxParameter*		pcValue;
	int64						lliName;
	int							i;

	//class T, typename... Ts, class T = int or a parameter-declaration like int N = 3.
	if (Is(CT_class) || Is(CT_typename))
	{
		i = 1;
		if (IsAhead(i, CT_Elipsis))
		{
			i++;
		}
		if (IsIdentifierAhead(i))
		{
			i++;
		}
		if (IsAhead(i, CT_Comma) || IsAhead(i, CT_GreaterThan) || IsAhead(i, CT_RightShift) || IsAhead(i, CT_Assignment))
		{
			pcType = mpcMemory->Create<CCPPSyntaxTypeParameter>(pcParent);
			pcType->miFirstToken = CurrentToken();
			pcType->mbTypename = Is(CT_typename);
			Next();
			pcType->mbPack = Accept(CT_Elipsis);
			if (IsIdentifier())
			{
				pcType->miName = CurrentToken();
				lliName = GetIdentifier();
				Next();
				pcType->mpcSymbol = Declare(lliName, pcType->miName, CPPSK_TypeParameter, pcType, mpcScope);
			}
			if (Accept(CT_Assignment))
			{
				pcType->mpcDefault = ParseTypeId(pcType);
				if (!pcType->mpcDefault)
				{
					return NULL;
				}
			}
			pcType->miLastToken = PreviousToken();
			return pcType;
		}
	}

	if (Is(CT_template))
	{
		return ParseTemplateTemplateParameter(pcParent);
	}

	pcValue = ParseParameter(pcParent);
	if (pcValue && pcValue->mpcSymbol)
	{
		pcValue->mpcSymbol->meKind = CPPSK_ValueParameter;
	}
	return pcValue;
}


//////////////////////////////////////////////////////////////////////////
//
// An enum declared again is scoped or not as it was first, with the same
// underlying type (an unscoped one's definition may leave it out where the
// first gave none).  [dcl.enum] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckEnumRedeclaration(CCPPSymbol* pcSymbol, CCPPSyntaxEnumSpecifier* pcEnum)
{
	SCPPEnumFirst*	psFirst;
	SCPPEnumFirst	sFirst;
	int				iFirst;
	int				iLast;

	if (!pcSymbol || (miTentative > 0) || IsSkimming(pcEnum->miFirstToken))
	{
		return;
	}
	iFirst = pcEnum->mpcBase ? pcEnum->mpcBase->miFirstToken : -1;
	iLast = pcEnum->mpcBase ? pcEnum->mpcBase->miLastToken : -1;
	psFirst = mcEnumFirsts.Get((int64)(size)pcSymbol);
	if (!psFirst)
	{
		sFirst.bScoped = pcEnum->mbScoped;
		sFirst.iBaseFirst = iFirst;
		sFirst.iBaseLast = iLast;
		mcEnumFirsts.Put((int64)(size)pcSymbol, &sFirst);
		return;
	}
	if ((psFirst->bScoped != pcEnum->mbScoped) || ((psFirst->iBaseFirst == -1) != (iFirst == -1) && (pcEnum->mbScoped || (iFirst != -1))) ||
		((psFirst->iBaseFirst != -1) && (iFirst != -1) && !SameTokens(psFirst->iBaseFirst, psFirst->iBaseLast, iFirst, iLast)))
	{
		ErrorAt(pcEnum->miFirstToken, (char*)"An enum declared again is scoped as it was, with the same underlying type");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a class's members may not be, seen in its definition alone: a
// union has no base and no virtual function; a local class has no static
// data member and no member template; a static member function isn't
// const, volatile or ref-qualified; a static data member doesn't have the
// class's name; and a data member is declared once.  [class.union] 2,
// [class.local] 4, [temp.mem] 2, [class.static.mfct] 2, [class.mem] 1, 13
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckClassMembers(CCPPSyntaxClassSpecifier* pcClass)
{
	CCPPSyntax*						pcMember;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	CArrayInt64						alliNames;
	size							i;
	size							j;
	bool							bLocal;
	bool							bUnion;
	int64							lliName;

	if ((miTentative > 0) || IsSkimming(pcClass->miFirstToken))
	{
		return;
	}
	bUnion = pcClass->meKey == CT_union;
	if (bUnion && (pcClass->macpBases.NumElements() > 0))
	{
		ErrorAt(pcClass->miFirstToken, (char*)"A union has no base class");
	}
	bLocal = IsLocalClass(pcClass);
	alliNames.Init();
	for (i = 0; i < pcClass->macpMembers.NumElements(); i++)
	{
		pcMember = pcClass->macpMembers.GetPtr(i);
		if (pcMember->GetKind() == CPPS_TemplateDeclaration)
		{
			if (bLocal)
			{
				ErrorAt(pcMember->miFirstToken, (char*)"A local class has no member template");
			}
			continue;
		}
		if (pcMember->GetKind() == CPPS_FunctionDefinition)
		{
			pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcMember)->mpcSpecifiers;
			pcDeclarator = ((CCPPSyntaxFunctionDefinition*)pcMember)->mpcDeclarator;
		}
		else if (pcMember->GetKind() == CPPS_SimpleDeclaration)
		{
			pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcMember)->mpcSpecifiers;
			pcDeclarator = NULL;
		}
		else
		{
			continue;
		}
		if (!pcSpecifiers || (pcSpecifiers->muiFlags & (CPPDS_FRIEND | CPPDS_TYPEDEF)))
		{
			continue;
		}
		if (bUnion && (pcSpecifiers->muiFlags & CPPDS_VIRTUAL))
		{
			ErrorAt(pcMember->miFirstToken, (char*)"A union has no virtual function");
		}
		for (j = 0; pcDeclarator || (j < ((pcMember->GetKind() == CPPS_SimpleDeclaration) ? ((CCPPSyntaxSimpleDeclaration*)pcMember)->macpDeclarators.NumElements() : 0)); j++)
		{
			pcInit = pcDeclarator ? NULL : (CCPPSyntaxInitDeclarator*)((CCPPSyntaxSimpleDeclaration*)pcMember)->macpDeclarators.GetPtr(j);
			if (pcInit)
			{
				pcDeclarator = pcInit->mpcDeclarator;
			}
			if (pcDeclarator->IsFunction())
			{
				pcSuffix = pcDeclarator->GetFunctionSuffix();
				if ((pcSpecifiers->muiFlags & CPPDS_STATIC) && pcSuffix && (pcSuffix->macpQualifiers.NumElements() > 0))
				{
					ErrorAt(pcDeclarator->miFirstToken, (char*)"A static member function isn't const, volatile, & or &&: it has no object");
				}
				CheckSpecialMember(pcClass, pcSpecifiers, pcDeclarator);
			}
			else if (pcInit && pcInit->mpcSymbol && !pcInit->mpcBitField)
			{
				lliName = pcInit->mpcSymbol->mlliName;
				if (pcSpecifiers->muiFlags & CPPDS_STATIC)
				{
					if (bLocal)
					{
						ErrorAt(pcDeclarator->miFirstToken, (char*)"A local class has no static data member");
					}
					if (pcClass->mpcSymbol && (lliName == pcClass->mpcSymbol->mlliName))
					{
						ErrorAt(pcDeclarator->miFirstToken, (char*)"A static data member doesn't have its class's name");
					}
				}
				if (alliNames.Contains(&lliName))
				{
					ErrorAt(pcDeclarator->miFirstToken, (char*)"The class's member is declared twice");
				}
				alliNames.Add(lliName);
			}
			pcDeclarator = NULL;
		}
	}
	alliNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What a constructor, conversion function or destructor's declaration
// isn't: a constructor virtual or static, a constructor or conversion
// function with a return type written, a conversion function or destructor
// with a parameter.  [class.ctor] 4, 6, [class.conv.fct] 1, [class.dtor] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckSpecialMember(CCPPSyntaxClassSpecifier* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	bool						bType;
	bool						bConstructor;

	pcName = pcDeclarator->GetName();
	pcComponent = pcName ? pcName->GetLast() : NULL;
	pcSuffix = pcDeclarator->GetFunctionSuffix();
	if (!pcComponent || !pcSuffix || (pcName->macpComponents.NumElements() != 1))
	{
		return;
	}
	bType = pcSpecifiers->mpcType || (pcSpecifiers->muiTypeKeywords != 0) || (pcSpecifiers->miLongs > 0);
	bConstructor = (pcComponent->meKind == CPPNC_Identifier) && (pcComponent->miName != -1) && pcClass->mpcSymbol && (GetTokenIdentifier(pcComponent->miName) == pcClass->mpcSymbol->mlliName);
	if (bConstructor && (pcSpecifiers->muiFlags & (CPPDS_VIRTUAL | CPPDS_STATIC)))
	{
		ErrorAt(pcDeclarator->miFirstToken, (char*)"A constructor is neither virtual nor static");
	}
	else if (bConstructor && bType)
	{
		ErrorAt(pcDeclarator->miFirstToken, (char*)"A constructor has no return type");
	}
	else if ((pcComponent->meKind == CPPNC_Conversion) && bType)
	{
		ErrorAt(pcDeclarator->miFirstToken, (char*)"A conversion function has no return type written: its name says it");
	}
	else if (((pcComponent->meKind == CPPNC_Conversion) || (pcComponent->meKind == CPPNC_Destructor)) && (pcSuffix->NumParameters() > 0))
	{
		ErrorAt(pcDeclarator->miFirstToken, (char*)"A conversion function or destructor takes no parameter");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// union { ... }; is an anonymous union: it has only public data members
// that aren't static, and at namespace scope it is static (in an unnamed
// namespace too, as g++ has it).  [class.union] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckAnonymousUnion(CCPPSyntaxDeclSpecifiers* pcSpecifiers, ECPPDeclarationContext eContext)
{
	CCPPSyntaxClassSpecifier*	pcUnion;
	CCPPSyntax*					pcMember;
	CCPPSyntaxDeclSpecifiers*	pcMemberSpecifiers;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	size						i;
	size						j;
	bool						bData;

	if (!pcSpecifiers->mpcType || (pcSpecifiers->mpcType->GetKind() != CPPS_ClassSpecifier) || (miTentative > 0) || IsSkimming(pcSpecifiers->miFirstToken))
	{
		return;
	}
	pcUnion = (CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType;
	if ((pcUnion->meKey != CT_union) || pcUnion->mpcName)
	{
		return;
	}
	if ((eContext == CPPDC_Namespace) && !(pcSpecifiers->muiFlags & CPPDS_STATIC))
	{
		ErrorAt(pcSpecifiers->miFirstToken, (char*)"An anonymous union at namespace scope is static");
	}
	for (i = 0; i < pcUnion->macpMembers.NumElements(); i++)
	{
		pcMember = pcUnion->macpMembers.GetPtr(i);
		if (pcMember->GetKind() == CPPS_AccessSpecifier)
		{
			bData = ((CCPPSyntaxAccessSpecifier*)pcMember)->meAccess == CPPA_Public;
		}
		else if (pcMember->GetKind() == CPPS_SimpleDeclaration)
		{
			pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcMember;
			pcMemberSpecifiers = pcDeclaration->mpcSpecifiers;
			bData = pcMemberSpecifiers && !(pcMemberSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_TYPEDEF | CPPDS_FRIEND)) && (pcDeclaration->macpDeclarators.NumElements() > 0) &&
					!(pcMemberSpecifiers->mpcType && (pcMemberSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier) && ((CCPPSyntaxClassSpecifier*)pcMemberSpecifiers->mpcType)->mpcName);
			for (j = 0; bData && (j < pcDeclaration->macpDeclarators.NumElements()); j++)
			{
				bData = !((CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(j))->mpcDeclarator->IsFunction();
			}
		}
		else
		{
			bData = (pcMember->GetKind() == CPPS_EmptyDeclaration) || (pcMember->GetKind() == CPPS_StaticAssert);
		}
		if (!bData)
		{
			ErrorAt(pcMember->miFirstToken, (char*)"An anonymous union has only public data members that aren't static");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a class is defined in a function (or is a member of one that
// is).  [class.local]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsLocalClass(CCPPSyntaxClassSpecifier* pcClass)
{
	CCPPScope*	pcScope;

	for (pcScope = pcClass->mpcScope ? pcClass->mpcScope->mpcParent : NULL; pcScope; pcScope = pcScope->mpcParent)
	{
		if ((pcScope->meKind == CPPSC_Function) || (pcScope->meKind == CPPSC_Block))
		{
			return true;
		}
		if (pcScope->meKind == CPPSC_Namespace)
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two runs of tokens are the same words and names.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::SameTokens(int iFirst, int iLast, int iOtherFirst, int iOtherLast)
{
	CCPPToken*	pcToken;
	CCPPToken*	pcOther;
	int			i;

	if ((iLast - iFirst) != (iOtherLast - iOtherFirst))
	{
		return false;
	}
	for (i = 0; i <= iLast - iFirst; i++)
	{
		pcToken = mpcTokeniser->Get(iFirst + i);
		pcOther = mpcTokeniser->Get(iOtherFirst + i);
		if (!pcToken || !pcOther)
		{
			return false;
		}
		if (pcToken->IsReservedWord() && pcOther->IsReservedWord())
		{
			if (((CCTReservedWord*)pcToken)->GetToken() != ((CCTReservedWord*)pcOther)->GetToken())
			{
				return false;
			}
		}
		else if (pcToken->IsIdentifier() && pcOther->IsIdentifier())
		{
			if (GetTokenIdentifier(iFirst + i) != GetTokenIdentifier(iOtherFirst + i))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}
