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
			pcSymbol->mpcDeclaration = pcClass;
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

	pcClass->mpcScope = mpcSymbols->CreateScope(CPPSC_Class, pcQualifier ? pcQualifier : mpcScope, pcSymbol);
	if (pcSymbol && (!pcSymbol->mpcMembers || bSimple))
	{
		pcSymbol->mpcMembers = pcClass->mpcScope;
	}
	if (pcSymbol && (miTentative == 0))
	{
		//The injected class name.  [class] 2
		pcClass->mpcScope->AddAlias(pcSymbol->mlliName, pcSymbol);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ParseBaseSpecifiers(CCPPSyntaxClassSpecifier* pcClass)
{
	CCPPSyntaxBaseSpecifier*	pcBase;
	CCPPSymbol*					pcSymbol;
	CArraySyntax				apcAttributes;

	//: base-specifier, ...  [class.derived]
	Expect(CT_Colon);
	for (;;)
	{
		pcBase = mpcMemory->Create<CCPPSyntaxBaseSpecifier>(pcClass);
		pcBase->miFirstToken = CurrentToken();
		apcAttributes.Init();
		ParseAttributes(&apcAttributes, pcBase);
		apcAttributes.Kill();
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
	CCPPScope*						pcSaved;
	bool							bDeclaringTemplate;
	bool							bNoGreater;
	int								iClassDepth;

	//{ member-specification }  [class.mem]
	if (!Expect(CT_OpenCurlyBracket))
	{
		return false;
	}

	pcSaved = mpcScope;
	mpcScope = pcClass->mpcScope;
	bDeclaringTemplate = mbDeclaringTemplate;
	mbDeclaringTemplate = false;
	bNoGreater = mbNoGreater;
	mbNoGreater = false;
	miClassDepth++;
	iClassDepth = miClassDepth;

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
			mpcScope = pcClass->mpcScope;
			miClassDepth = iClassDepth;
			mbNoGreater = false;
			mbDeclaringTemplate = false;
			Rewind(sStart);
			SkipDeclaration();
			if (CurrentToken() == sStart.iToken)
			{
				Next();
			}
			pcSkipped = mpcMemory->Create<CCPPSyntaxSkippedDeclaration>(pcClass);
			pcSkipped->SetTokens(sStart.iToken, PreviousToken());
			pcClass->macpMembers.Add(pcSkipped);
		}
	}

	mpcScope = pcSaved;
	mbDeclaringTemplate = bDeclaringTemplate;
	mbNoGreater = bNoGreater;
	miClassDepth--;
	if (!Expect(CT_CloseCurlyBracket))
	{
		return false;
	}

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
	CCPPScope*			pcSaved;
	bool				bNoGreater;

	//The bodies of member functions defined in a class, now that the class is complete.  [class.mem] 6
	while (macDeferred.NumElements() > 0)
	{
		sDeferred = *macDeferred.Get(0);
		macDeferred.RemoveAt(0, true);

		sPosition = Mark();
		pcSaved = mpcScope;
		bNoGreater = mbNoGreater;
		mbNoGreater = false;

		sBody.iToken = sDeferred.iStart;
		sBody.bHalf = false;
		Rewind(sBody);
		if (ParseFunctionBody(sDeferred.pcFunction))
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
		mpcScope = pcSaved;
		mbNoGreater = bNoGreater;
	}
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
	CCPPScope*							pcSaved;
	int64								lliName;
	bool								bNoGreater;
	CArraySyntax						apcAttributes;

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
	apcAttributes.Init();
	ParseAttributes(&apcAttributes, pcEnum);
	apcAttributes.Kill();

	if (IsIdentifier() || Is(CT_ScopeResolution))
	{
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
		pcElaborated = mpcMemory->Create<CCPPSyntaxElaboratedTypeSpecifier>(pcParent);
		pcElaborated->miFirstToken = pcEnum->miFirstToken;
		pcElaborated->meKey = CT_enum;
		pcElaborated->mpcName = pcEnum->mpcName;
		pcEnum->mpcName->mpcParent = pcElaborated;
		if ((lliName != -1) && !pcEnum->mpcName->mpcSymbol)
		{
			pcEnum->mpcName->mpcSymbol = Declare(lliName, pcComponent->miName, CPPSK_Enum, pcElaborated, pcDeclarationScope);
		}
		pcElaborated->miLastToken = PreviousToken();
		return pcElaborated;
	}

	pcSymbol = NULL;
	if (lliName != -1)
	{
		pcSymbol = pcDeclarationScope->FindHere(lliName);
		pcSymbol = ReuseOrDeclare((pcSymbol && (pcSymbol->meKind == CPPSK_Enum)) ? pcSymbol : NULL, lliName, pcComponent->miName, CPPSK_Enum, pcEnum, pcDeclarationScope);
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
	if (pcSymbol)
	{
		pcSymbol->mpcMembers = pcEnum->mpcScope;
	}

	Next();
	pcEnum->mbBody = true;
	pcSaved = mpcScope;
	mpcScope = pcEnum->mpcScope;
	bNoGreater = mbNoGreater;
	mbNoGreater = false;
	while (!Is(CT_CloseCurlyBracket))
	{
		if (!IsIdentifier())
		{
			mpcScope = pcSaved;
			mbNoGreater = bNoGreater;
			Error("Expected an enumerator.");
			return NULL;
		}
		pcEnumerator = mpcMemory->Create<CCPPSyntaxEnumerator>(pcEnum);
		pcEnumerator->miFirstToken = CurrentToken();
		pcEnumerator->miName = CurrentToken();
		lliName = GetIdentifier();
		Next();

		//An enumerator can be used in the values of the ones after it.
		pcEnumeratorSymbol = Declare(lliName, pcEnumerator->miName, CPPSK_Enumerator, pcEnumerator, pcEnum->mpcScope);
		pcEnumerator->mpcSymbol = pcEnumeratorSymbol;
		if (pcEnumeratorSymbol && !pcEnum->mbScoped)
		{
			pcDeclarationScope->AddAlias(lliName, pcEnumeratorSymbol);
		}

		apcAttributes.Init();
		ParseAttributes(&apcAttributes, pcEnumerator);
		apcAttributes.Kill();
		if (Accept(CT_Assignment))
		{
			pcEnumerator->mpcValue = ParseConstantExpression(pcEnumerator);
			if (!pcEnumerator->mpcValue)
			{
				mpcScope = pcSaved;
				mbNoGreater = bNoGreater;
				return NULL;
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
	mpcScope = pcSaved;
	mbNoGreater = bNoGreater;

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
	pcTemplate->mpcDeclaration = ParseDeclaration(pcTemplate, eContext);
	mbDeclaringTemplate = bDeclaringTemplate;
	PopScope(pcScope);
	if (!pcTemplate->mpcDeclaration)
	{
		return NULL;
	}
	pcTemplate->miLastToken = PreviousToken();
	return pcTemplate;
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
		Error("Template template parameters are not supported.");
		return NULL;
	}

	pcValue = ParseParameter(pcParent);
	if (pcValue && pcValue->mpcSymbol)
	{
		pcValue->mpcSymbol->meKind = CPPSK_ValueParameter;
	}
	return pcValue;
}

