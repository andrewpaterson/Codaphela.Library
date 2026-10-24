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
#include "CTString.h"
#include "CppParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::ParseDeclarations(CArraySyntax* papcDeclarations, CCPPSyntax* pcParent, bool bInBraces)
{
	CCPPSyntaxDeclaration*		pcDeclaration;
	SCPPParserPosition			sStart;
	CCPPScope*					pcScope;
	int							iClassDepth;
	bool						bNoGreater;
	CCPPSyntaxDeclaration*		pcSkipped;

	//[dcl.dcl]  A declaration that can't be parsed is skipped and kept as its tokens.
	while (!IsEnd())
	{
		if (bInBraces && Is(CT_CloseCurlyBracket))
		{
			break;
		}

		sStart = Mark();
		pcScope = mpcScope;
		iClassDepth = miClassDepth;
		bNoGreater = mbNoGreater;
		pcDeclaration = ParseDeclaration(pcParent, CPPDC_Namespace);
		if (pcDeclaration)
		{
			papcDeclarations->Add(pcDeclaration);
		}
		else
		{
			mpcScope = pcScope;
			miClassDepth = iClassDepth;
			mbNoGreater = bNoGreater;
			Rewind(sStart);
			SkipDeclaration();
			if (CurrentToken() == sStart.iToken)
			{
				//A stray '}' or ')'.
				Next();
			}
			pcSkipped = mpcMemory->CreateSkippedDeclaration(pcParent);
			pcSkipped->SetTokens(sStart.iToken, PreviousToken());
			papcDeclarations->Add(pcSkipped);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseDeclaration(CCPPSyntax* pcParent, ECPPDeclarationContext eContext)
{
	CCPPSyntaxEmptyDeclaration*		pcEmpty;
	CCPPSyntaxExtensionDeclaration*	pcExtension;
	CCPPToken*						pcNext;

	if (Is(CT_SemiColon))
	{
		pcEmpty = mpcMemory->CreateEmptyDeclaration(pcParent);
		pcEmpty->SetTokens(CurrentToken(), CurrentToken());
		Next();
		return pcEmpty;
	}

	if (Is(CT_namespace) || (Is(CT_inline) && IsAhead(1, CT_namespace)))
	{
		return ParseNamespace(pcParent);
	}

	if (Is(CT___extension__) && (IsAhead(1, CT_template) || IsAhead(1, CT_namespace) || IsAhead(1, CT_extern) || IsAhead(1, CT_using)))
	{
		pcExtension = mpcMemory->CreateExtensionDeclaration(pcParent);
		pcExtension->miFirstToken = CurrentToken();
		Next();
		pcExtension->mpcDeclaration = ParseDeclaration(pcExtension, eContext);
		if (!pcExtension->mpcDeclaration)
		{
			return NULL;
		}
		pcExtension->miLastToken = PreviousToken();
		return pcExtension;
	}

	if (Is(CT_extern))
	{
		pcNext = Peek(1);
		if (pcNext && pcNext->IsString())
		{
			return ParseLinkageSpecification(pcParent);
		}
		if (pcNext && pcNext->Is(CT_template))
		{
			return ParseTemplateDeclaration(pcParent, eContext);
		}
	}

	if (Is(CT_template))
	{
		return ParseTemplateDeclaration(pcParent, eContext);
	}

	if (Is(CT_using))
	{
		return ParseUsing(pcParent);
	}

	if (Is(CT_static_assert))
	{
		return ParseStaticAssert(pcParent);
	}

	if ((Is(CT_asm) || Is(CT___asm__)) && IsAhead(1, CT_OpenRoundBracket))
	{
		return ParseAsmDeclaration(pcParent);
	}

	return ParseSimpleDeclaration(pcParent, eContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseNamespace(CCPPSyntax* pcParent)
{
	CCPPSyntaxNamespace*	pcNamespace;
	CCPPSyntaxName*			pcAlias;
	CCPPSymbol*				pcSymbol;
	CCPPScope*				pcScope;
	CCPPScope*				pcSaved;
	int64					lliName;
	CCPPSyntaxSkippedDeclaration*	pcSkipped;

	//[namespace.def]
	pcNamespace = mpcMemory->CreateNamespace(pcParent);
	pcNamespace->miFirstToken = CurrentToken();
	pcNamespace->mbInline = Accept(CT_inline);
	Expect(CT_namespace);

	lliName = -1;
	if (IsIdentifier())
	{
		pcNamespace->miName = CurrentToken();
		lliName = GetIdentifier();
		Next();
	}

	if ((lliName != -1) && Accept(CT_Assignment))
	{
		//namespace X = Y;  [namespace.alias]
		pcAlias = ParseName(pcNamespace, false, NULL);
		if (!pcAlias || !Expect(CT_SemiColon))
		{
			return NULL;
		}
		pcSymbol = Declare(lliName, pcNamespace->miName, CPPSK_Namespace, pcNamespace, mpcScope);
		if (pcSymbol && pcAlias->mpcSymbol)
		{
			pcSymbol->mpcMembers = pcAlias->mpcSymbol->mpcMembers;
		}
		pcSkipped = mpcMemory->CreateSkippedDeclaration(pcParent);
		pcSkipped->SetTokens(pcNamespace->miFirstToken, PreviousToken());
		return pcSkipped;
	}

	//Attributes on a namespace, as in g++'s inline namespace __cxx11 __attribute__((__abi_tag__ ("cxx11"))).
	ParseAttributes(&pcNamespace->macpAttributes, pcNamespace);

	if (!Expect(CT_OpenCurlyBracket))
	{
		return NULL;
	}

	//A namespace can be opened many times; its scope is the same each time.
	pcScope = NULL;
	if (lliName != -1)
	{
		pcSymbol = mpcScope->FindHere(lliName);
		if (pcSymbol && pcSymbol->IsNamespace() && pcSymbol->mpcMembers)
		{
			pcScope = pcSymbol->mpcMembers;
			Resolve(pcNamespace->miName, pcSymbol, CPPTR_Declaration);
		}
		else
		{
			pcSymbol = Declare(lliName, pcNamespace->miName, CPPSK_Namespace, pcNamespace, mpcScope);
			pcScope = mpcSymbols->CreateScope(CPPSC_Namespace, mpcScope, pcSymbol);
			if (pcSymbol)
			{
				pcSymbol->mpcMembers = pcScope;
			}
		}
	}
	else
	{
		//The members of an unnamed namespace are visible in the enclosing namespace.  [namespace.unnamed]
		pcScope = mpcSymbols->CreateScope(CPPSC_Namespace, mpcScope, NULL);
		mpcScope->mapcUsings.Add(pcScope);
	}

	if (pcNamespace->mbInline && (lliName != -1))
	{
		mpcScope->mapcUsings.Add(pcScope);
	}

	pcNamespace->mpcScope = pcScope;
	pcSaved = mpcScope;
	mpcScope = pcScope;
	ParseDeclarations(&pcNamespace->macpDeclarations, pcNamespace, true);
	mpcScope = pcSaved;

	if (!Expect(CT_CloseCurlyBracket))
	{
		return NULL;
	}
	pcNamespace->SetTokens(pcNamespace->miFirstToken, PreviousToken());
	return pcNamespace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseLinkageSpecification(CCPPSyntax* pcParent)
{
	CCPPSyntaxLinkageSpecification*		pcLinkage;
	CCPPSyntaxDeclaration*				pcDeclaration;

	//extern "C" { ... } or extern "C" declaration.  [dcl.link]
	pcLinkage = mpcMemory->CreateLinkageSpecification(pcParent);
	pcLinkage->miFirstToken = CurrentToken();
	Expect(CT_extern);
	pcLinkage->miLanguage = CurrentToken();
	Next();

	if (Accept(CT_OpenCurlyBracket))
	{
		pcLinkage->mbBraced = true;
		ParseDeclarations(&pcLinkage->macpDeclarations, pcLinkage, true);
		if (!Expect(CT_CloseCurlyBracket))
		{
			return NULL;
		}
	}
	else
	{
		pcDeclaration = ParseDeclaration(pcLinkage, CPPDC_Namespace);
		if (!pcDeclaration)
		{
			return NULL;
		}
		pcLinkage->macpDeclarations.Add(pcDeclaration);
	}

	pcLinkage->SetTokens(pcLinkage->miFirstToken, PreviousToken());
	return pcLinkage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseUsing(CCPPSyntax* pcParent)
{
	CCPPSyntaxUsingDirective*		pcDirective;
	CCPPSyntaxUsingDeclaration*		pcUsing;
	CCPPSyntaxAliasDeclaration*		pcAlias;
	CCPPSymbol*						pcSymbol;
	CCPPSymbol*						pcTypeSymbol;
	CCPPSyntaxNameComponent*		pcLast;
	int								iFirstToken;
	bool							bTemplate;
	int64							lliName;

	iFirstToken = CurrentToken();
	Expect(CT_using);
	bTemplate = IsDeclaringTemplate();

	if (Accept(CT_namespace))
	{
		//using namespace X;  [namespace.udir]
		pcDirective = mpcMemory->CreateUsingDirective(pcParent);
		pcDirective->mpcName = ParseName(pcDirective, false, NULL);
		if (!pcDirective->mpcName || !Expect(CT_SemiColon))
		{
			return NULL;
		}
		pcSymbol = pcDirective->mpcName->mpcSymbol;
		if ((miTentative == 0) && pcSymbol && pcSymbol->mpcMembers)
		{
			mpcScope->mapcUsings.Add(pcSymbol->mpcMembers);
		}
		pcDirective->SetTokens(iFirstToken, PreviousToken());
		return pcDirective;
	}

	if (IsIdentifier() && IsAhead(1, CT_Assignment))
	{
		//using X = type;  [dcl.typedef] 2
		pcAlias = mpcMemory->CreateAliasDeclaration(pcParent);
		pcAlias->miName = CurrentToken();
		lliName = GetIdentifier();
		Next();
		Next();
		pcAlias->mpcType = ParseTypeId(pcAlias);
		if (!pcAlias->mpcType || !Expect(CT_SemiColon))
		{
			return NULL;
		}
		pcSymbol = Declare(lliName, pcAlias->miName, bTemplate ? CPPSK_AliasTemplate : CPPSK_Typedef, pcAlias, mpcScope->GetDeclarationScope());
		pcAlias->mpcSymbol = pcSymbol;
		pcTypeSymbol = GetTypeSymbol(pcAlias->mpcType->mpcSpecifiers);
		if (pcSymbol && pcTypeSymbol && (!pcAlias->mpcType->mpcDeclarator || pcAlias->mpcType->mpcDeclarator->IsEmpty()))
		{
			pcSymbol->mpcMembers = pcTypeSymbol->mpcMembers;
		}
		pcAlias->SetTokens(iFirstToken, PreviousToken());
		return pcAlias;
	}

	//using X::y;  [namespace.udecl]
	pcUsing = mpcMemory->CreateUsingDeclaration(pcParent);
	pcUsing->mbTypename = Accept(CT_typename);
	pcUsing->mpcName = ParseName(pcUsing, false, NULL);
	if (!pcUsing->mpcName || !Expect(CT_SemiColon))
	{
		return NULL;
	}
	pcLast = pcUsing->mpcName->GetLast();
	if ((miTentative == 0) && pcLast && (pcLast->miName != -1) && (pcLast->meKind == CPPNC_Identifier))
	{
		pcSymbol = pcUsing->mpcName->mpcSymbol;
		if (pcSymbol)
		{
			mpcScope->AddAlias(pcSymbol->mlliName, pcSymbol);
		}
	}
	pcUsing->SetTokens(iFirstToken, PreviousToken());
	return pcUsing;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseStaticAssert(CCPPSyntax* pcParent)
{
	CCPPSyntaxStaticAssert*		pcAssert;

	//[dcl.dcl] 4
	pcAssert = mpcMemory->CreateStaticAssert(pcParent);
	pcAssert->miFirstToken = CurrentToken();
	Expect(CT_static_assert);
	if (!Expect(CT_OpenRoundBracket))
	{
		return NULL;
	}
	pcAssert->mpcCondition = ParseConstantExpression(pcAssert);
	if (!pcAssert->mpcCondition)
	{
		return NULL;
	}
	if (Accept(CT_Comma))
	{
		pcAssert->mpcMessage = ParseStringExpression(pcAssert);
		if (!pcAssert->mpcMessage)
		{
			return NULL;
		}
	}
	if (!Expect(CT_CloseRoundBracket) || !Expect(CT_SemiColon))
	{
		return NULL;
	}
	pcAssert->SetTokens(pcAssert->miFirstToken, PreviousToken());
	return pcAssert;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseAsmDeclaration(CCPPSyntax* pcParent)
{
	CCPPSyntaxSkippedDeclaration*	pcAsm;
	int								iFirstToken;

	//asm("..."); is kept as its tokens.  [dcl.asm]
	iFirstToken = CurrentToken();
	Next();
	while (Is(CT_volatile) || Is(CT___volatile__) || Is(CT_goto) || Is(CT_inline))
	{
		Next();
	}
	if (!Is(CT_OpenRoundBracket) || !SkipBalanced() || !Expect(CT_SemiColon))
	{
		return NULL;
	}
	pcAsm = mpcMemory->CreateSkippedDeclaration(pcParent);
	pcAsm->SetTokens(iFirstToken, PreviousToken());
	return pcAsm;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsDeclaringTemplate(void)
{
	return mbDeclaringTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseSimpleDeclaration(CCPPSyntax* pcParent, ECPPDeclarationContext eContext)
{
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxDeclarator*			pcDeclarator;
	CCPPSyntaxInitDeclarator*		pcInit;
	int								iFirstToken;
	ECPPDeclaratorKind				eDeclaratorKind;

	//[dcl.dcl] simple-declaration, [dcl.fct.def] function-definition and [class.mem] member-declaration.
	iFirstToken = CurrentToken();
	pcSimple = mpcMemory->CreateSimpleDeclaration(pcParent);
	pcSpecifiers = ParseDeclSpecifiers(pcSimple, eContext);
	if (!pcSpecifiers)
	{
		return NULL;
	}
	pcSimple->mpcSpecifiers = pcSpecifiers;

	if (Is(CT_SemiColon))
	{
		//class X { ... }; struct Y; friend class Z;
		if (pcSpecifiers->macpSpecifiers.NumElements() == 0)
		{
			Error("Expected a declaration.");
			return NULL;
		}
		Next();
		pcSimple->SetTokens(iFirstToken, PreviousToken());
		return pcSimple;
	}

	//An unnamed bit-field, as in int : 0;
	eDeclaratorKind = ((eContext == CPPDC_Class) && Is(CT_Colon)) ? CPPDK_Abstract : CPPDK_Named;
	pcDeclarator = ParseDeclarator(pcSimple, eDeclaratorKind, eContext);
	if (!pcDeclarator)
	{
		return NULL;
	}

	if (pcDeclarator->IsFunction() && IsFunctionDefinitionStart() && (eContext != CPPDC_Condition) && (eContext != CPPDC_ForInit))
	{
		return ParseFunctionDefinition(pcParent, pcSpecifiers, pcDeclarator, eContext, iFirstToken);
	}

	for (;;)
	{
		pcInit = mpcMemory->CreateInitDeclarator(pcSimple);
		pcInit->miFirstToken = pcDeclarator->miFirstToken;
		pcInit->mpcDeclarator = pcDeclarator;
		pcDeclarator->mpcParent = pcInit;

		//The name is declared before its initialiser.  [basic.scope.pdecl]
		pcInit->mpcSymbol = DeclareDeclarator(pcSpecifiers, pcDeclarator, pcInit, eContext);

		if ((eContext == CPPDC_Class) && Accept(CT_Colon))
		{
			pcInit->mpcBitField = ParseConstantExpression(pcInit);
			if (!pcInit->mpcBitField)
			{
				return NULL;
			}
		}

		if (Is(CT_Assignment) || Is(CT_OpenRoundBracket) || Is(CT_OpenCurlyBracket))
		{
			pcInit->mpcInitializer = ParseInitializer(pcInit, &pcInit->meInitializer);
			if (!pcInit->mpcInitializer && (pcInit->meInitializer != CPPI_Default) && (pcInit->meInitializer != CPPI_Delete))
			{
				return NULL;
			}
		}
		pcInit->miLastToken = PreviousToken();
		pcSimple->macpDeclarators.Add(pcInit);

		if (!Accept(CT_Comma))
		{
			break;
		}

		pcDeclarator = ParseDeclarator(pcSimple, CPPDK_Named, eContext);
		if (!pcDeclarator)
		{
			return NULL;
		}
	}

	if (eContext == CPPDC_Condition)
	{
		pcSimple->mbNoSemicolon = true;
	}
	else if (!Expect(CT_SemiColon))
	{
		return NULL;
	}
	pcSimple->SetTokens(iFirstToken, PreviousToken());
	return pcSimple;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsFunctionDefinitionStart(void)
{
	return Is(CT_OpenCurlyBracket) || Is(CT_Colon) || Is(CT_try) || (Is(CT_Assignment) && (IsAhead(1, CT_default) || IsAhead(1, CT_delete)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseFunctionDefinition(CCPPSyntax* pcParent, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, ECPPDeclarationContext eContext, int iFirstToken)
{
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	SCPPDeferredBody*				psDeferred;

	//[dcl.fct.def]
	pcFunction = mpcMemory->CreateFunctionDefinition(pcParent);
	pcFunction->miFirstToken = iFirstToken;
	pcFunction->mpcSpecifiers = pcSpecifiers;
	pcFunction->mpcDeclarator = pcDeclarator;
	pcSpecifiers->mpcParent = pcFunction;
	pcDeclarator->mpcParent = pcFunction;

	DeclareDeclarator(pcSpecifiers, pcDeclarator, pcFunction, eContext);
	pcSuffix = pcDeclarator->GetFunctionSuffix();
	pcFunction->mpcScope = pcSuffix ? pcSuffix->mpcScope : mpcScope;

	if (Accept(CT_Assignment))
	{
		pcFunction->meBody = Is(CT_default) ? CPPFB_Default : CPPFB_Delete;
		Next();
		if (!Expect(CT_SemiColon))
		{
			return NULL;
		}
		pcFunction->SetTokens(iFirstToken, PreviousToken());
		return pcFunction;
	}

	if (IsSkimming() || (eContext == CPPDC_Class))
	{
		//System header bodies are skipped.  Member function bodies are parsed when their class is complete.
		pcFunction->meBody = CPPFB_Skipped;
		pcFunction->miBodyStart = CurrentToken();
		if (!SkipFunctionBody())
		{
			Error("Expected a function body.");
			return NULL;
		}
		pcFunction->miBodyEnd = PreviousToken();

		if (!IsSkimming(pcFunction->miBodyStart) && (miTentative == 0))
		{
			psDeferred = macDeferred.Add();
			psDeferred->pcFunction = pcFunction;
			psDeferred->pcScope = pcFunction->mpcScope;
			psDeferred->iStart = pcFunction->miBodyStart;
			psDeferred->iEnd = pcFunction->miBodyEnd;
		}
	}
	else if (!ParseFunctionBody(pcFunction))
	{
		return NULL;
	}

	pcFunction->SetTokens(iFirstToken, PreviousToken());
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::SkipFunctionBody(void)
{
	//Moves past [: mem-initializers] { body } or try ... catch handlers.
	Accept(CT_try);
	if (Accept(CT_Colon))
	{
		for (;;)
		{
			while (!IsEnd() && !Is(CT_OpenRoundBracket) && !Is(CT_OpenCurlyBracket))
			{
				Next();
			}
			if (!SkipBalanced())
			{
				return false;
			}
			if (Accept(CT_Elipsis))
			{
			}
			if (!Accept(CT_Comma))
			{
				break;
			}
		}
	}

	if (!Is(CT_OpenCurlyBracket) || !SkipBalanced())
	{
		return false;
	}

	while (Is(CT_catch))
	{
		Next();
		if (!Is(CT_OpenRoundBracket) || !SkipBalanced())
		{
			return false;
		}
		if (!Is(CT_OpenCurlyBracket) || !SkipBalanced())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ParseFunctionBody(CCPPSyntaxFunctionDefinition* pcFunction)
{
	CCPPScope*	pcSaved;
	bool		bDeclaringTemplate;

	pcSaved = mpcScope;
	mpcScope = pcFunction->mpcScope;
	bDeclaringTemplate = mbDeclaringTemplate;
	mbDeclaringTemplate = false;

	if (Is(CT_try))
	{
		pcFunction->mpcBody = ParseTryStatement(pcFunction);
	}
	else
	{
		if (Is(CT_Colon))
		{
			if (!ParseMemberInitializers(pcFunction))
			{
				mpcScope = pcSaved;
				mbDeclaringTemplate = bDeclaringTemplate;
				return false;
			}
		}
		pcFunction->mpcBody = ParseCompoundStatement(pcFunction, true);
	}

	mpcScope = pcSaved;
	mbDeclaringTemplate = bDeclaringTemplate;
	if (!pcFunction->mpcBody)
	{
		return false;
	}
	pcFunction->meBody = CPPFB_Body;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ParseMemberInitializers(CCPPSyntaxFunctionDefinition* pcFunction)
{
	CCPPSyntaxMemberInitializer*	pcInitializer;
	ECPPInitializer					eKind;

	//: mem-initializer, ...  [class.base.init]
	Expect(CT_Colon);
	for (;;)
	{
		pcInitializer = mpcMemory->CreateMemberInitializer(pcFunction);
		pcInitializer->miFirstToken = CurrentToken();
		pcInitializer->mpcName = ParseName(pcInitializer, false, NULL);
		if (!pcInitializer->mpcName)
		{
			return false;
		}
		if (!Is(CT_OpenRoundBracket) && !Is(CT_OpenCurlyBracket))
		{
			return Error("Expected '(' or '{' after the member initializer name.");
		}
		pcInitializer->mpcInitializer = ParseInitializer(pcInitializer, &eKind);
		if (!pcInitializer->mpcInitializer)
		{
			return false;
		}
		Accept(CT_Elipsis);
		pcInitializer->miLastToken = PreviousToken();
		pcFunction->macpMemberInitializers.Add(pcInitializer);
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
CCPPSymbol* CCPPParser::GetTypeSymbol(CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	CCPPSyntaxTypeSpecifier*	pcType;

	//The class, enum or typedef named by the type specifier.
	if (!pcSpecifiers || !pcSpecifiers->mpcType)
	{
		return NULL;
	}

	pcType = pcSpecifiers->mpcType;
	switch (pcType->GetKind())
	{
	case CPPS_NamedTypeSpecifier:
		return ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName ? ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName->mpcSymbol : NULL;
	case CPPS_ClassSpecifier:
		return ((CCPPSyntaxClassSpecifier*)pcType)->mpcSymbol;
	case CPPS_EnumSpecifier:
		return ((CCPPSyntaxEnumSpecifier*)pcType)->mpcSymbol;
	case CPPS_ElaboratedTypeSpecifier:
		return ((CCPPSyntaxElaboratedTypeSpecifier*)pcType)->mpcName ? ((CCPPSyntaxElaboratedTypeSpecifier*)pcType)->mpcName->mpcSymbol : NULL;
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPParser::DeclareDeclarator(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcDeclaration, ECPPDeclarationContext eContext)
{
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	ECPPSymbolKind				eKind;
	CCPPSymbol*					pcSymbol;
	CCPPSymbol*					pcType;
	int64						lliName;
	CCPPScope*					pcScope;
	uint32						uiFlags;

	pcName = pcDeclarator->GetName();
	if (!pcName)
	{
		return NULL;
	}

	pcComponent = pcName->GetLast();
	if (!pcName->IsSimple())
	{
		//A qualified name declares a member declared before, as in void CChars::Init(void).  Operators, destructors and
		//specialisations are not looked up by the parser so they are not declared.
		if (pcName->mpcSymbol && pcComponent && (pcComponent->miName != -1))
		{
			Resolve(pcComponent->miName, pcName->mpcSymbol, CPPTR_Declaration);
		}
		return pcName->mpcSymbol;
	}

	uiFlags = pcSpecifiers ? pcSpecifiers->muiFlags : 0;
	if (uiFlags & CPPDS_FRIEND)
	{
		//A friend is not a member.  [class.friend]
		return pcName->mpcSymbol;
	}

	if (uiFlags & CPPDS_TYPEDEF)
	{
		eKind = CPPSK_Typedef;
	}
	else if (pcDeclarator->IsFunction())
	{
		eKind = IsDeclaringTemplate() ? CPPSK_FunctionTemplate : CPPSK_Function;
	}
	else if (eContext == CPPDC_Parameter)
	{
		eKind = CPPSK_Parameter;
	}
	else if (eContext == CPPDC_Class)
	{
		eKind = CPPSK_Field;
	}
	else
	{
		eKind = CPPSK_Variable;
	}

	//A template's non-type parameter is in the template's scope, not the one enclosing it.
	lliName = ((CCTIdentifier*)mpcTokeniser->Get(pcComponent->miName))->mlliIdentifier;
	pcScope = (eContext == CPPDC_Parameter) ? mpcScope : mpcScope->GetDeclarationScope();
	pcSymbol = Declare(lliName, pcComponent->miName, eKind, pcDeclaration, pcScope);
	pcName->mpcSymbol = pcSymbol;
	pcComponent->mpcSymbol = pcSymbol;

	if (pcSymbol && (eKind == CPPSK_Typedef))
	{
		//typedef struct X Y; Y::z finds X's members.
		pcType = GetTypeSymbol(pcSpecifiers);
		if (pcType && (pcDeclarator->macpPrefix.NumElements() == 0) && (pcDeclarator->macpSuffixes.NumElements() == 0) && !pcDeclarator->mpcNested)
		{
			pcSymbol->mpcMembers = pcType->mpcMembers;
		}
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsDeclSpecifierKeyword(ECppTokens eToken)
{
	switch (eToken)
	{
	case CT_typedef:
	case CT_friend:
	case CT_constexpr:
	case CT_static:
	case CT_extern:
	case CT_register:
	case CT_mutable:
	case CT_thread_local:
	case CT___thread:
	case CT_inline:
	case CT___inline:
	case CT___inline__:
	case CT___forceinline:
	case CT_virtual:
	case CT_explicit:
	case CT_const:
	case CT___const__:
	case CT_volatile:
	case CT___volatile__:
	case CT___restrict:
	case CT___restrict__:
	case CT___unaligned:
	case CT___w64:
	case CT___ptr32:
	case CT___ptr64:
	case CT___cdecl:
	case CT___stdcall:
	case CT___fastcall:
	case CT___thiscall:
	case CT___vectorcall:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsSimpleTypeKeyword(ECppTokens eToken)
{
	switch (eToken)
	{
	case CT_void:
	case CT_bool:
	case CT_char:
	case CT_wchar_t:
	case CT_char16_t:
	case CT_char32_t:
	case CT_short:
	case CT_int:
	case CT_long:
	case CT_signed:
	case CT___signed__:
	case CT_unsigned:
	case CT_float:
	case CT_double:
	case CT_auto:
	case CT___int8:
	case CT___int16:
	case CT___int32:
	case CT___int64:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsConstructorName(CCPPSyntaxName* pcName)
{
	CCPPSyntaxNameComponent*	pcLast;
	CCPPSyntaxNameComponent*	pcPrevious;
	size						uiNumComponents;
	CCPPScope*					pcScope;
	int64						lliName;

	//X(...) in class X, or X::X(...) outside it, declares a constructor.  [class.ctor]
	uiNumComponents = pcName->macpComponents.NumElements();
	pcLast = pcName->GetLast();
	if (!pcLast || (pcLast->meKind != CPPNC_Identifier) || (pcLast->miName == -1))
	{
		return false;
	}
	lliName = ((CCTIdentifier*)mpcTokeniser->Get(pcLast->miName))->mlliIdentifier;

	if (uiNumComponents >= 2)
	{
		pcPrevious = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(uiNumComponents - 2);
		return (pcPrevious->meKind == CPPNC_Identifier) && (pcPrevious->miName != -1) && (((CCTIdentifier*)mpcTokeniser->Get(pcPrevious->miName))->mlliIdentifier == lliName);
	}

	for (pcScope = mpcScope; pcScope && (pcScope->meKind == CPPSC_Template); pcScope = pcScope->mpcParent)
	{
	}
	if (pcScope && (pcScope->meKind == CPPSC_Class) && pcScope->mpcOwner)
	{
		return pcScope->mpcOwner->mlliName == lliName;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsAttributeStart(void)
{
	return Is(CT___attribute__) || Is(CT___declspec) || Is(CT___pragma) || Is(CT___extension__) || (Is(CT_OpenSquareBracket) && IsAhead(1, CT_OpenSquareBracket));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxAttribute* CCPPParser::ParseAttribute(CCPPSyntax* pcParent)
{
	CCPPSyntaxAttribute*	pcAttribute;
	int						iFirstToken;

	//__attribute__((...)), __declspec(...), __pragma(...), __extension__ and [[...]] are kept as their tokens.
	iFirstToken = CurrentToken();
	if (Is(CT_OpenSquareBracket))
	{
		if (!SkipBalanced())
		{
			return NULL;
		}
	}
	else if (Accept(CT___extension__))
	{
	}
	else
	{
		Next();
		if (!Is(CT_OpenRoundBracket) || !SkipBalanced())
		{
			Error("Expected '(' after the attribute.");
			return NULL;
		}
	}

	pcAttribute = mpcMemory->CreateAttribute(pcParent);
	pcAttribute->SetTokens(iFirstToken, PreviousToken());
	return pcAttribute;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::ParseAttributes(CArraySyntax* papcAttributes, CCPPSyntax* pcParent)
{
	CCPPSyntaxAttribute*	pcAttribute;
	int						iFirstToken;

	for (;;)
	{
		if (IsAttributeStart())
		{
			pcAttribute = ParseAttribute(pcParent);
			if (!pcAttribute)
			{
				return;
			}
			papcAttributes->Add(pcAttribute);
		}
		else if ((Is(CT_asm) || Is(CT___asm__)) && IsAhead(1, CT_OpenRoundBracket))
		{
			//g++ asm labels, as in int f(void) __asm__("_f");
			iFirstToken = CurrentToken();
			Next();
			if (!SkipBalanced())
			{
				return;
			}
			pcAttribute = mpcMemory->CreateAttribute(pcParent);
			pcAttribute->SetTokens(iFirstToken, PreviousToken());
			papcAttributes->Add(pcAttribute);
		}
		else
		{
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclSpecifiers* CCPPParser::ParseDeclSpecifiers(CCPPSyntax* pcParent, ECPPDeclarationContext eContext)
{
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxKeywordSpecifier*		pcKeyword;
	CCPPSyntaxNamedTypeSpecifier*	pcNamed;
	CCPPSyntaxDecltypeSpecifier*	pcDecltype;
	CCPPSyntaxAlignasSpecifier*		pcAlignas;
	CCPPSyntaxTypeSpecifier*		pcType;
	CCPPSyntaxAttribute*			pcAttribute;
	CCPPSyntaxName*					pcName;
	CCPPToken*						pcToken;
	ECppTokens						eToken;
	bool							bType;
	SCPPParserPosition				sPosition;
	CCPPSymbol*						pcSymbol;
	bool							bTypeLike;

	//[dcl.spec]
	pcSpecifiers = mpcMemory->CreateDeclSpecifiers(pcParent);
	pcSpecifiers->miFirstToken = CurrentToken();
	bType = false;
	for (;;)
	{
		if (IsAttributeStart())
		{
			pcAttribute = ParseAttribute(pcSpecifiers);
			if (!pcAttribute)
			{
				return NULL;
			}
			pcSpecifiers->macpSpecifiers.Add(pcAttribute);
			continue;
		}

		pcToken = Peek(0);
		if (!pcToken)
		{
			break;
		}

		if (pcToken->IsReservedWord())
		{
			eToken = ((CCTReservedWord*)pcToken)->GetToken();
			if (IsDeclSpecifierKeyword(eToken) && ((eContext != CPPDC_TypeId) || (eToken == CT_const) || (eToken == CT_volatile) || (eToken == CT___const__) || (eToken == CT___volatile__)))
			{
				switch (eToken)
				{
				case CT_typedef:		pcSpecifiers->muiFlags |= CPPDS_TYPEDEF;		break;
				case CT_friend:			pcSpecifiers->muiFlags |= CPPDS_FRIEND;			break;
				case CT_constexpr:		pcSpecifiers->muiFlags |= CPPDS_CONSTEXPR;		break;
				case CT_static:			pcSpecifiers->muiFlags |= CPPDS_STATIC;			break;
				case CT_extern:			pcSpecifiers->muiFlags |= CPPDS_EXTERN;			break;
				case CT_register:		pcSpecifiers->muiFlags |= CPPDS_REGISTER;		break;
				case CT_mutable:		pcSpecifiers->muiFlags |= CPPDS_MUTABLE;		break;
				case CT_thread_local:
				case CT___thread:		pcSpecifiers->muiFlags |= CPPDS_THREAD_LOCAL;	break;
				case CT_inline:
				case CT___inline:
				case CT___inline__:
				case CT___forceinline:	pcSpecifiers->muiFlags |= CPPDS_INLINE;			break;
				case CT_virtual:		pcSpecifiers->muiFlags |= CPPDS_VIRTUAL;		break;
				case CT_explicit:		pcSpecifiers->muiFlags |= CPPDS_EXPLICIT;		break;
				case CT_const:
				case CT___const__:		pcSpecifiers->muiFlags |= CPPDS_CONST;			break;
				case CT_volatile:
				case CT___volatile__:	pcSpecifiers->muiFlags |= CPPDS_VOLATILE;		break;
				default:																break;
				}
				pcKeyword = mpcMemory->CreateKeywordSpecifier(pcSpecifiers);
				pcKeyword->meKeyword = eToken;
				pcKeyword->SetTokens(CurrentToken(), CurrentToken());
				pcSpecifiers->macpSpecifiers.Add(pcKeyword);
				Next();
				continue;
			}

			if (IsSimpleTypeKeyword(eToken))
			{
				switch (eToken)
				{
				case CT_void:		pcSpecifiers->muiTypeKeywords |= CPPTK_VOID;		break;
				case CT_bool:		pcSpecifiers->muiTypeKeywords |= CPPTK_BOOL;		break;
				case CT_char:		pcSpecifiers->muiTypeKeywords |= CPPTK_CHAR;		break;
				case CT_wchar_t:	pcSpecifiers->muiTypeKeywords |= CPPTK_WCHAR_T;		break;
				case CT_char16_t:	pcSpecifiers->muiTypeKeywords |= CPPTK_CHAR16_T;	break;
				case CT_char32_t:	pcSpecifiers->muiTypeKeywords |= CPPTK_CHAR32_T;	break;
				case CT_short:		pcSpecifiers->muiTypeKeywords |= CPPTK_SHORT;		break;
				case CT_int:		pcSpecifiers->muiTypeKeywords |= CPPTK_INT;			break;
				case CT_long:		pcSpecifiers->miLongs++;							break;
				case CT_signed:
				case CT___signed__:	pcSpecifiers->muiTypeKeywords |= CPPTK_SIGNED;		break;
				case CT_unsigned:	pcSpecifiers->muiTypeKeywords |= CPPTK_UNSIGNED;	break;
				case CT_float:		pcSpecifiers->muiTypeKeywords |= CPPTK_FLOAT;		break;
				case CT_double:		pcSpecifiers->muiTypeKeywords |= CPPTK_DOUBLE;		break;
				case CT_auto:		pcSpecifiers->muiTypeKeywords |= CPPTK_AUTO;		break;
				case CT___int8:		pcSpecifiers->muiTypeKeywords |= CPPTK_INT8;		break;
				case CT___int16:	pcSpecifiers->muiTypeKeywords |= CPPTK_INT16;		break;
				case CT___int32:	pcSpecifiers->muiTypeKeywords |= CPPTK_INT32;		break;
				case CT___int64:	pcSpecifiers->muiTypeKeywords |= CPPTK_INT64;		break;
				default:																break;
				}
				pcKeyword = mpcMemory->CreateKeywordSpecifier(pcSpecifiers);
				pcKeyword->meKeyword = eToken;
				pcKeyword->SetTokens(CurrentToken(), CurrentToken());
				pcSpecifiers->macpSpecifiers.Add(pcKeyword);
				Next();
				bType = true;
				continue;
			}

			if ((eToken == CT_class) || (eToken == CT_struct) || (eToken == CT_union))
			{
				if (bType && !pcSpecifiers->mpcType)
				{
					break;
				}
				pcType = ParseClassSpecifier(pcSpecifiers, eContext, (pcSpecifiers->muiFlags & CPPDS_FRIEND) != 0);
				if (!pcType)
				{
					return NULL;
				}
				pcSpecifiers->mpcType = pcType;
				pcSpecifiers->macpSpecifiers.Add(pcType);
				bType = true;
				continue;
			}

			if (eToken == CT_enum)
			{
				pcType = ParseEnumSpecifier(pcSpecifiers, eContext);
				if (!pcType)
				{
					return NULL;
				}
				pcSpecifiers->mpcType = pcType;
				pcSpecifiers->macpSpecifiers.Add(pcType);
				bType = true;
				continue;
			}

			if ((eToken == CT_typename) && !bType)
			{
				//typename X::Y  [temp.res]
				pcKeyword = mpcMemory->CreateKeywordSpecifier(pcSpecifiers);
				pcKeyword->meKeyword = eToken;
				pcKeyword->SetTokens(CurrentToken(), CurrentToken());
				pcSpecifiers->macpSpecifiers.Add(pcKeyword);
				Next();

				pcNamed = mpcMemory->CreateNamedTypeSpecifier(pcSpecifiers);
				pcNamed->miFirstToken = CurrentToken();
				pcNamed->mpcName = ParseName(pcNamed, false, NULL);
				if (!pcNamed->mpcName)
				{
					return NULL;
				}
				pcNamed->miLastToken = PreviousToken();
				pcSpecifiers->mpcType = pcNamed;
				pcSpecifiers->macpSpecifiers.Add(pcNamed);
				bType = true;
				continue;
			}

			if (((eToken == CT_decltype) || (eToken == CT___typeof__)) && !bType)
			{
				//decltype(e) and g++'s __typeof__(e) or __typeof__(type).
				pcDecltype = mpcMemory->CreateDecltypeSpecifier(pcSpecifiers);
				pcDecltype->miFirstToken = CurrentToken();
				pcDecltype->meKeyword = eToken;
				Next();
				if (!Expect(CT_OpenRoundBracket))
				{
					return NULL;
				}
				if (IsTypeIdStart() && IsTypeIdAhead(CT_CloseRoundBracket))
				{
					pcDecltype->mpcOperand = ParseTypeId(pcDecltype);
				}
				else
				{
					pcDecltype->mpcOperand = ParseExpression(pcDecltype);
				}
				if (!pcDecltype->mpcOperand || !Expect(CT_CloseRoundBracket))
				{
					return NULL;
				}
				pcDecltype->miLastToken = PreviousToken();
				pcSpecifiers->mpcType = pcDecltype;
				pcSpecifiers->macpSpecifiers.Add(pcDecltype);
				bType = true;
				continue;
			}

			if (eToken == CT_alignas)
			{
				//alignas(n) or alignas(type).  [dcl.align]
				pcAlignas = mpcMemory->CreateAlignasSpecifier(pcSpecifiers);
				pcAlignas->miFirstToken = CurrentToken();
				Next();
				if (!Expect(CT_OpenRoundBracket))
				{
					return NULL;
				}
				if (IsTypeIdStart() && IsTypeIdAhead(CT_CloseRoundBracket))
				{
					pcAlignas->mpcAlignment = ParseTypeId(pcAlignas);
				}
				else
				{
					pcAlignas->mpcAlignment = ParseConstantExpression(pcAlignas);
				}
				if (!pcAlignas->mpcAlignment || !Expect(CT_CloseRoundBracket))
				{
					return NULL;
				}
				pcAlignas->miLastToken = PreviousToken();
				pcSpecifiers->macpSpecifiers.Add(pcAlignas);
				continue;
			}
			break;
		}

		if ((pcToken->IsIdentifier() || pcToken->Is(CT_ScopeResolution)) && !bType)
		{
			//A type name, unless it is the name being declared.
			sPosition = Mark();
			pcNamed = mpcMemory->CreateNamedTypeSpecifier(pcSpecifiers);
			pcNamed->miFirstToken = CurrentToken();

			pcName = ParseName(pcNamed, false, NULL);

			if (!pcName)
			{
				Rewind(sPosition);
				break;
			}

			if ((IsConstructorName(pcName) && Is(CT_OpenRoundBracket)) || (pcName->GetLast()->meKind != CPPNC_Identifier))
			{
				Rewind(sPosition);
				break;
			}

			pcSymbol = pcName->mpcSymbol;
			if (!pcSymbol && IsSkimming(sPosition.iToken) && Peek(0) && Peek(0)->IsReservedWord() && pcName->IsSimple() && (IsDeclSpecifierKeyword(((CCTReservedWord*)Peek(0))->GetToken()) || IsSimpleTypeKeyword(((CCTReservedWord*)Peek(0))->GetToken())))
			{
				//An unknown name before a type keyword in a skimmed header is an undefined macro, as in __MINGW_IMPORT int x;
				pcAttribute = mpcMemory->CreateAttribute(pcSpecifiers);
				pcAttribute->SetTokens(sPosition.iToken, sPosition.iToken);
				pcSpecifiers->macpSpecifiers.Add(pcAttribute);
				continue;
			}

			pcSymbol = pcName->mpcSymbol;
			bTypeLike = pcSymbol && pcSymbol->IsType();
			if (!bTypeLike && !pcSymbol)
			{
				//An unknown name, from a dependent scope or a skimmed header, is a type if a declarator follows.
				if (IsIdentifier() || ((eContext != CPPDC_Block) && (eContext != CPPDC_Condition) && (eContext != CPPDC_ForInit) && (Is(CT_Multiply) || Is(CT_BitwiseAnd) || Is(CT_LogicalAnd) || Is(CT___cdecl) || Is(CT___stdcall))))
				{
					bTypeLike = true;
				}
				else if ((eContext == CPPDC_Parameter) && (Is(CT_Comma) || Is(CT_CloseRoundBracket) || Is(CT_OpenSquareBracket)))
				{
					bTypeLike = true;
				}
			}

			if (!bTypeLike)
			{
				Rewind(sPosition);
				break;
			}

			pcNamed->mpcName = pcName;
			pcNamed->miLastToken = PreviousToken();
			pcSpecifiers->mpcType = pcNamed;
			pcSpecifiers->macpSpecifiers.Add(pcNamed);
			bType = true;
			continue;
		}
		break;
	}

	pcSpecifiers->miLastToken = PreviousToken();
	return pcSpecifiers;
}

