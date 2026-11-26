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
	SCPPParserState				sState;
	CCPPSyntaxDeclaration*		pcSkipped;
	bool						bRecord;
	CArrayTemplate<CCPPSymbol*>	apcDeclared;
	CArrayTemplate<CCPPSymbol*>*	papcRecorded;
	CCPPScope*					pcRecordScope;

	papcRecorded = NULL;
	pcRecordScope = NULL;

	//[dcl.dcl]  A declaration that can't be parsed is skipped and kept as its tokens.
	while (!IsEnd())
	{
		if (bInBraces && Is(CT_CloseCurlyBracket))
		{
			break;
		}
		if (SkipPragma())
		{
			continue;
		}

		sStart = Mark();
		sState = SaveState();
		bRecord = mpcHeaderIndex && (miTentative == 0) && (mpcScope->meKind == CPPSC_Namespace);
		if (bRecord && SkipLazy(papcDeclarations, pcParent))
		{
			continue;
		}
		if (bRecord)
		{
			apcDeclared.Init();
			papcRecorded = mpapcRecorded;
			pcRecordScope = mpcRecordScope;
			mpapcRecorded = &apcDeclared;
			mpcRecordScope = mpcScope;
		}
		pcDeclaration = ParseDeclaration(pcParent, CPPDC_Namespace);
		if (bRecord)
		{
			mpapcRecorded = papcRecorded;
			mpcRecordScope = pcRecordScope;
			if (pcDeclaration)
			{
				RecordHeaderDeclaration(sStart.iToken, PreviousToken(), pcDeclaration, &apcDeclared);
			}
			apcDeclared.Kill();
		}
		if (pcDeclaration)
		{
			papcDeclarations->Add(pcDeclaration);
		}
		else
		{
			RecoverAt(sStart, &sState, false);
			pcSkipped = mpcMemory->Create<CCPPSyntaxSkippedDeclaration>(pcParent);
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
		pcEmpty = mpcMemory->Create<CCPPSyntaxEmptyDeclaration>(pcParent);
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
		pcExtension = mpcMemory->Create<CCPPSyntaxExtensionDeclaration>(pcParent);
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
	pcNamespace = mpcMemory->Create<CCPPSyntaxNamespace>(pcParent);
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
		meNameLookup = CPPL_Namespace;
		pcAlias = ParseName(pcNamespace, false, NULL);
		if (!pcAlias || !Expect(CT_SemiColon))
		{
			return NULL;
		}
		pcSymbol = mpcScope->FindHere(lliName);
		if ((miTentative == 0) && pcSymbol && (pcSymbol->meKind == CPPSK_Namespace) && pcAlias->mpcSymbol && (pcSymbol->mpcMembers != pcAlias->mpcSymbol->mpcMembers))
		{
			//namespace B = A; and then namespace B = D;  [namespace.alias] 3
			ErrorAt(pcNamespace->miName, "The name is a namespace already, and not that one");
			return NULL;
		}
		pcSymbol = Declare(lliName, pcNamespace->miName, CPPSK_Namespace, pcNamespace, mpcScope);
		if (pcSymbol && pcAlias->mpcSymbol)
		{
			pcSymbol->mpcMembers = pcAlias->mpcSymbol->mpcMembers;
		}
		pcSkipped = mpcMemory->Create<CCPPSyntaxSkippedDeclaration>(pcParent);
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
		//The members of an unnamed namespace are visible in the enclosing namespace, which has only one.  [namespace.unnamed]
		pcScope = mpcScope->FindUnnamedNamespace();
		if (!pcScope)
		{
			pcScope = mpcSymbols->CreateScope(CPPSC_Namespace, mpcScope, NULL);
			mpcScope->mapcUsings.Add(pcScope);
		}
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
	pcLinkage = mpcMemory->Create<CCPPSyntaxLinkageSpecification>(pcParent);
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
		pcDirective = mpcMemory->Create<CCPPSyntaxUsingDirective>(pcParent);
		meNameLookup = CPPL_Namespace;
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
		pcAlias = mpcMemory->Create<CCPPSyntaxAliasDeclaration>(pcParent);
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
	pcUsing = mpcMemory->Create<CCPPSyntaxUsingDeclaration>(pcParent);
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
		if (pcSymbol && (pcSymbol->meKind == CPPSK_Namespace))
		{
			//using A::B; for a namespace B.  [namespace.udecl] 6
			ErrorAt(iFirstToken, "A using-declaration can't name a namespace");
			return NULL;
		}
		if (pcSymbol && IsUsingConflict(pcSymbol))
		{
			ErrorAt(iFirstToken, "The using-declaration's name is declared here already");
			return NULL;
		}
		if (pcSymbol && (mpcScope->meKind == CPPSC_Class) && pcSymbol->mpcScope && (pcSymbol->mpcScope->meKind == CPPSC_Namespace) && pcUsing->mpcName->mpcLookup && (pcUsing->mpcName->mpcLookup->meKind == CPPSC_Namespace) && !IsSkimming(iFirstToken))
		{
			//In a class, a using-declaration names a base's member.  [namespace.udecl] 3
			ErrorAt(iFirstToken, "A class's using-declaration names a member of one of its bases");
			return NULL;
		}
		if (pcSymbol)
		{
			mpcScope->AddAlias(pcSymbol->mlliName, pcSymbol, iFirstToken);
		}
	}
	pcUsing->SetTokens(iFirstToken, PreviousToken());
	return pcUsing;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a using-declaration of pcSymbol, in a namespace or a block,
// conflicts with what the scope declares of the name itself: int x; using
// A::x;.  Functions are overloads; a class's members are hidden instead.
// [namespace.udecl] 14
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsUsingConflict(CCPPSymbol* pcSymbol)
{
	CCPPSymbol*	pcHere;

	if (mpcScope->meKind == CPPSC_Class)
	{
		return false;
	}
	pcHere = mpcScope->FindHere(pcSymbol->mlliName);
	return pcHere && (pcHere != pcSymbol) && (pcHere->mpcScope == mpcScope) && !pcHere->IsFunction() && !pcSymbol->IsFunction();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclaration* CCPPParser::ParseStaticAssert(CCPPSyntax* pcParent)
{
	CCPPSyntaxStaticAssert*		pcAssert;

	//[dcl.dcl] 4
	pcAssert = mpcMemory->Create<CCPPSyntaxStaticAssert>(pcParent);
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
	pcAsm = mpcMemory->Create<CCPPSyntaxSkippedDeclaration>(pcParent);
	pcAsm->SetTokens(iFirstToken, PreviousToken());
	return pcAsm;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsDeclaringTemplate(void) const
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
	CCPPScope*						pcQualifierScope;

	//[dcl.dcl] simple-declaration, [dcl.fct.def] function-definition and [class.mem] member-declaration.
	iFirstToken = CurrentToken();
	pcSpecifiers = ParseDeclSpecifiers(pcParent, eContext);
	if (!pcSpecifiers)
	{
		return NULL;
	}

	if (Is(CT_SemiColon))
	{
		//class X { ... }; struct Y; friend class Z;
		if (pcSpecifiers->macpSpecifiers.NumElements() == 0)
		{
			Error("Expected a declaration.");
			return NULL;
		}
		if (!IsDeclaringSpecifiers(pcSpecifiers))
		{
			//const int; and static;  [dcl.dcl] 5
			ErrorAt(iFirstToken, "The declaration does not declare anything.");
		}
		CheckAnonymousUnion(pcSpecifiers, eContext);
		Next();
		pcSimple = NewSimpleDeclaration(pcParent, pcSpecifiers);
		pcSimple->SetTokens(iFirstToken, PreviousToken());
		return pcSimple;
	}

	//An unnamed bit-field, as in int : 0;
	eDeclaratorKind = ((eContext == CPPDC_Class) && Is(CT_Colon)) ? CPPDK_Abstract : CPPDK_Named;
	pcDeclarator = ParseDeclarator(pcParent, eDeclaratorKind, eContext);
	if (!pcDeclarator)
	{
		return NULL;
	}

	if (pcDeclarator->IsFunction() && IsFunctionDefinitionStart() && (eContext != CPPDC_Condition) && (eContext != CPPDC_ForInit))
	{
		return ParseFunctionDefinition(pcParent, pcSpecifiers, pcDeclarator, eContext, iFirstToken);
	}

	pcSimple = NewSimpleDeclaration(pcParent, pcSpecifiers);
	for (;;)
	{
		pcInit = mpcMemory->Create<CCPPSyntaxInitDeclarator>(pcSimple);
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
			pcQualifierScope = QualifierScope(pcDeclarator);
			if (pcQualifierScope)
			{
				//int S::a = k; looks k up in S before the scopes around the definition, as a member function's body
				//does, and int N::x = y; in N.  [basic.lookup.unqual] 13, 14
				PushScope(CPPSC_Block, NULL)->mpcQualifier = pcQualifierScope;
			}
			pcInit->mpcInitializer = ParseInitializer(pcInit, &pcInit->meInitializer);
			if (pcQualifierScope)
			{
				PopScope(mpcScope);
			}
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
// The class or namespace a declarator's qualified name is a member of (the
// S of int S::a), or NULL for a simple name or one the parser can't look in.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPParser::QualifierScope(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxName*	pcName;

	pcName = pcDeclarator->GetName();
	if (!pcName || pcName->IsSimple() || !pcName->mpcLookup || (pcName->mpcLookup == mpcScope))
	{
		return NULL;
	}
	if ((pcName->mpcLookup->meKind != CPPSC_Class) && (pcName->mpcLookup->meKind != CPPSC_Namespace))
	{
		return NULL;
	}
	return pcName->mpcLookup;
}


//////////////////////////////////////////////////////////////////////////
//
// The node for a declaration, once it isn't a function definition, which is
// its own kind of node.  The specifiers were parsed before that was known.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxSimpleDeclaration* CCPPParser::NewSimpleDeclaration(CCPPSyntax* pcParent, CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	CCPPSyntaxSimpleDeclaration*	pcSimple;

	pcSimple = mpcMemory->Create<CCPPSyntaxSimpleDeclaration>(pcParent);
	pcSimple->mpcSpecifiers = pcSpecifiers;
	pcSpecifiers->mpcParent = pcSimple;
	return pcSimple;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether specifiers without declarators declare something: a class or an
// enum, a friend, or only attributes.  [dcl.dcl] 5 [class.friend] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsDeclaringSpecifiers(CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	ECPPSyntax	eKind;
	size		i;

	if (pcSpecifiers->mpcType)
	{
		eKind = pcSpecifiers->mpcType->GetKind();
		if ((eKind == CPPS_ClassSpecifier) || (eKind == CPPS_EnumSpecifier) || (eKind == CPPS_ElaboratedTypeSpecifier))
		{
			return true;
		}
		return (pcSpecifiers->muiFlags & CPPDS_FRIEND) != 0;
	}

	for (i = 0; i < pcSpecifiers->macpSpecifiers.NumElements(); i++)
	{
		if (pcSpecifiers->macpSpecifiers.GetPtr(i)->GetKind() != CPPS_Attribute)
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
	pcFunction = mpcMemory->Create<CCPPSyntaxFunctionDefinition>(pcParent);
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
		if (pcFunction->meBody == CPPFB_Default)
		{
			CheckDefaulted(pcDeclarator);
		}
		Next();
		if (!Expect(CT_SemiColon))
		{
			return NULL;
		}
		pcFunction->SetTokens(iFirstToken, PreviousToken());
		return pcFunction;
	}

	if ((IsSkimming() && (miTemplates == 0)) || (eContext == CPPDC_Class))
	{
		//System header bodies are skipped, but a template's: an instance needs its body.  Member function bodies are
		//parsed when their class is complete.
		pcFunction->meBody = CPPFB_Skipped;
		pcFunction->miBodyStart = CurrentToken();
		if (!SkipFunctionBody())
		{
			Error("Expected a function body.");
			return NULL;
		}
		pcFunction->miBodyEnd = PreviousToken();

		if ((!IsSkimming(pcFunction->miBodyStart) || (miTemplates > 0)) && (miTentative == 0))
		{
			psDeferred = macDeferred.Add();
			psDeferred->pcFunction = pcFunction;
			psDeferred->pcParameter = NULL;
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
	size		uiLabels;

	pcSaved = mpcScope;
	mpcScope = pcFunction->mpcScope;
	bDeclaringTemplate = mbDeclaringTemplate;
	mbDeclaringTemplate = false;
	uiLabels = BeginLabels();

	if (Is(CT_try))
	{
		//A function-try-block: try [: mem-initializers] { body } handlers.  [except]
		pcFunction->mpcBody = ParseTryStatement(pcFunction, pcFunction);
	}
	else
	{
		if (Is(CT_Colon))
		{
			if (!ParseMemberInitializers(pcFunction))
			{
				mpcScope = pcSaved;
				mbDeclaringTemplate = bDeclaringTemplate;
				EndLabels(uiLabels);
				return false;
			}
		}
		pcFunction->mpcBody = ParseCompoundStatement(pcFunction, true);
	}

	mpcScope = pcSaved;
	mbDeclaringTemplate = bDeclaringTemplate;
	EndLabels(uiLabels);
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
		pcInitializer = mpcMemory->Create<CCPPSyntaxMemberInitializer>(pcFunction);
		pcInitializer->miFirstToken = CurrentToken();
		pcInitializer->mpcName = ParseName(pcInitializer, false, NULL);
		if (!pcInitializer->mpcName)
		{
			return false;
		}
		ResolveMemberInitializerName(pcInitializer->mpcName);
		if (!Is(CT_OpenRoundBracket) && !Is(CT_OpenCurlyBracket))
		{
			return Error("Expected '(' or '{' after the member initializer name.");
		}
		pcInitializer->mpcInitializer = ParseInitializer(pcInitializer, &eKind);
		if (!pcInitializer->mpcInitializer)
		{
			return false;
		}
		pcInitializer->mbPack = Accept(CT_Elipsis);
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
// A member initialiser names a member of the constructor's class before
// anything else of the name: the parameter in S(int v) : v(v) hides the
// member in the initialiser's value and in the body, but not there.
// [class.base.init] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::ResolveMemberInitializerName(CCPPSyntaxName* pcName)
{
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPScope*					pcClass;
	CCPPSymbol*					pcMember;

	pcComponent = pcName->GetLast();
	pcClass = mpcScope->GetMemberFunctionClass();
	if (!pcClass || (pcName->macpComponents.NumElements() != 1) || (pcComponent->meKind != CPPNC_Identifier))
	{
		return;
	}

	pcMember = pcClass->FindMember(GetTokenIdentifier(pcComponent->miName));
	if (pcMember && (pcMember != pcName->mpcSymbol))
	{
		pcComponent->mpcSymbol = pcMember;
		pcName->mpcSymbol = pcMember;
		Resolve(pcComponent->miName, pcMember, CPPTR_Reference);
	}
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
	CCPPSyntaxDeclSpecifiers*	pcTypeSpecifiers;
	CCPPSyntaxDeclarator*		pcShape;
	int64						lliName;
	CCPPScope*					pcScope;
	uint32						uiFlags;

	pcName = pcDeclarator->GetName();
	if (!pcName)
	{
		return NULL;
	}

	pcComponent = pcName->GetLast();
	if ((pcName->macpComponents.NumElements() > 1) && pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_STATIC) && pcName->mpcLookup && (pcName->mpcLookup->meKind == CPPSC_Class) && (miTentative == 0) && !IsSkimming(pcName->miFirstToken))
	{
		//[dcl.stc] 5, [class.static.data] 5
		ErrorAt(pcName->miFirstToken, (char*)"static is written on a member's declaration in its class, not on its definition outside it");
	}
	if (pcName->IsSimple() && pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_EXPLICIT) && (eContext == CPPDC_Class) && (miTentative == 0) && pcComponent &&
		(pcComponent->meKind != CPPNC_Conversion) && !(pcDeclarator->IsFunction() && IsConstructorName(pcName)))
	{
		//[dcl.fct.spec] 6
		ErrorAt(pcName->miFirstToken, (char*)"explicit is for a constructor or a conversion function");
	}
	if (!pcName->IsSimple() && !pcName->mpcSymbol && pcComponent && (pcComponent->meKind == CPPNC_Identifier) && !pcComponent->mbTemplateArguments && (pcComponent->miName != -1) && pcName->mpcLookup &&
		((pcName->mpcLookup->meKind == CPPSC_Namespace) || ((pcName->mpcLookup->meKind == CPPSC_Class) && !pcName->mpcLookup->IsDependent())) && (pcName->mpcLookup != mpcScope) &&
		(miTentative == 0) && !IsSkimming(pcName->miFirstToken) && !(pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_FRIEND)))
	{
		//N::F or C::F defines what N or C declares.  [namespace.memdef] 2, [class.mfct] 2
		ErrorAt(pcComponent->miName, (char*)"The qualified name is of nothing its namespace or class declares");
	}
	if (!pcName->IsSimple())
	{
		//A qualified name declares a member declared before, as in void CChars::Init(void).  Operators, destructors and
		//specialisations are not looked up by the parser so they are not declared.
		if (pcName->mpcSymbol && pcComponent && (pcComponent->miName != -1))
		{
			//X::X finds X's injected class name: what it declares is a constructor, which is declared after the class
			//in the class's own scope.  [class.qual] 2
			pcSymbol = pcName->mpcSymbol;
			if (pcDeclarator->IsFunction() && !pcSymbol->IsFunction() && pcSymbol->mpcMembers && IsConstructorName(pcName))
			{
				for (pcType = pcSymbol->mpcNext; pcType; pcType = pcType->mpcNext)
				{
					if (pcType->IsFunction() && (pcType->mpcScope == pcSymbol->mpcMembers))
					{
						pcSymbol = pcType;
						break;
					}
				}
			}
			Resolve(pcComponent->miName, pcSymbol, CPPTR_Declaration);
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
	else if (eContext == CPPDC_Parameter)
	{
		//A parameter declared as a function, int f(int), is a pointer to one: no function of the name.  [dcl.fct] 5
		eKind = CPPSK_Parameter;
	}
	else if (pcDeclarator->IsFunction() || (pcSpecifiers && pcDeclarator->IsNameAlone() && pcSpecifiers->NamesFunctionType(&pcTypeSpecifiers, &pcShape)))
	{
		//Binary Add; for a typedef of a function's type declares a function too.  [dcl.fct] 10
		eKind = IsDeclaringTemplate() ? CPPSK_FunctionTemplate : CPPSK_Function;
		if (pcSpecifiers && !pcDeclarator->mpcNested && (pcDeclarator->NumPointers() == 0) && (pcDeclarator->macpSuffixes.NumElements() == 1) && pcSpecifiers->NamesArrayOrFunctionType())
		{
			//Said here, as a function nothing calls is never declared to the generator.  [dcl.fct] 8
			ErrorAt(pcComponent->miFirstToken, (char*)"A function can't return an array or a function: it returns a pointer or a reference to one");
		}
	}
	else if (IsDeclaringTemplate())
	{
		//At namespace scope, or a class's static member: template<class T> static const int size = sizeof(T);  [temp] 1
		eKind = CPPSK_VariableTemplate;
	}
	else
	{
		eKind = (eContext == CPPDC_Class) ? CPPSK_Field : CPPSK_Variable;
	}

	//A template's non-type parameter is in the template's scope, not the one enclosing it.
	lliName = GetTokenIdentifier(pcComponent->miName);
	pcScope = (eContext == CPPDC_Parameter) ? mpcScope : mpcScope->GetDeclarationScope();
	CheckRedeclaration(lliName, eKind, (uiFlags & CPPDS_EXTERN) != 0, pcComponent->miName, pcScope);
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
// A declaration of a name of eKind in pcScope that another there forbids,
// reported at iToken: a variable or a function and something else of the
// name, two enumerators, a block's variable declared twice (not extern) or
// declared again in the outermost block of the statement whose condition
// declares it or of the function whose parameter it is, and a template
// parameter's name declared in its template.  Classes and enums are hidden by
// the others instead.  [basic.scope.declarative] 4, [basic.scope.block] 2, 4,
// [temp.local] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::CheckRedeclaration(int64 lliName, ECPPSymbolKind eKind, bool bExtern, int iToken, CCPPScope* pcScope)
{
	const char*	szClash;
	CCPPScope*	pcTemplate;
	CCPPSymbol*	pcParameter;

	if ((miTentative > 0) || (lliName == -1) || !pcScope || (iToken < 0) || IsSkimming(iToken))
	{
		return true;
	}
	szClash = Clash(pcScope, lliName, eKind, bExtern) ? "The name is declared already in this scope, as something it can't be beside" : NULL;
	if (!szClash && !bExtern && ((eKind == CPPSK_Variable) || (eKind == CPPSK_Function)) && (pcScope->meKind == CPPSC_Block) && pcScope->mpcParent)
	{
		if (pcScope->mpcParent->mbControl && OuterDeclares(pcScope->mpcParent, lliName, false))
		{
			szClash = "The name is declared by the statement's condition: it can't be declared again in its block";
		}
		else if ((pcScope->mpcParent->meKind == CPPSC_Function) && OuterDeclares(pcScope->mpcParent, lliName, true))
		{
			szClash = "The name is a parameter's: it can't be declared again in the function's outermost block";
		}
	}
	for (pcTemplate = (pcScope->meKind == CPPSC_Template) ? pcScope->mpcParent : pcScope; !szClash && pcTemplate; pcTemplate = pcTemplate->mpcParent)
	{
		pcParameter = (pcTemplate->meKind == CPPSC_Template) ? pcTemplate->FindParsed(lliName) : NULL;
		if (pcParameter && (pcParameter->mpcScope == pcTemplate))
		{
			szClash = "The name is a template parameter's: it can't be declared again in its template";
		}
	}
	if (szClash)
	{
		ErrorAt(iToken, (char*)szClash);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a declaration of a name of eKind clashes with one in pcScope
// already (CheckRedeclaration).  An unscoped enumerator of an enum in the
// scope is one of the scope's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::Clash(CCPPScope* pcScope, int64 lliName, ECPPSymbolKind eKind, bool bExtern)
{
	CCPPSymbol*		pcOther;
	ECPPSymbolKind	eOther;
	bool			bBlock;

	bBlock = (pcScope->meKind == CPPSC_Block) || (pcScope->meKind == CPPSC_Function);
	for (pcOther = pcScope->FindParsed(lliName); pcOther; pcOther = pcOther->mpcNext)
	{
		if (pcOther->mpcLazy)
		{
			continue;
		}
		if ((pcOther->mpcScope != pcScope) && !((pcOther->meKind == CPPSK_Enumerator) && pcOther->mpcScope && (pcOther->mpcScope->mpcParent == pcScope)))
		{
			continue;
		}
		eOther = pcOther->meKind;
		switch (eKind)
		{
		case CPPSK_Variable:
		case CPPSK_VariableTemplate:
			if ((eOther == CPPSK_Function) || (eOther == CPPSK_FunctionTemplate) || (eOther == CPPSK_Enumerator) || (eOther == CPPSK_Typedef) || (eOther == CPPSK_Namespace))
			{
				return true;
			}
			if (bBlock && !bExtern && ((eOther == CPPSK_Variable) || (eOther == CPPSK_Parameter)) && !IsExternDeclaration(pcOther))
			{
				return true;
			}
			break;
		case CPPSK_Parameter:
			if ((pcScope->meKind == CPPSC_Function) && (eOther == CPPSK_Parameter))
			{
				return true;
			}
			break;
		case CPPSK_Function:
		case CPPSK_FunctionTemplate:
			if ((eOther == CPPSK_Variable) || (eOther == CPPSK_Field) || (eOther == CPPSK_Enumerator) || (eOther == CPPSK_Typedef) || (eOther == CPPSK_Namespace) || (eOther == CPPSK_VariableTemplate))
			{
				return true;
			}
			break;
		case CPPSK_Enumerator:
			if ((eOther == CPPSK_Variable) || (eOther == CPPSK_Field) || (eOther == CPPSK_Function) || (eOther == CPPSK_FunctionTemplate) || (eOther == CPPSK_Enumerator) || (eOther == CPPSK_Typedef) || (eOther == CPPSK_Namespace) || (eOther == CPPSK_VariableTemplate))
			{
				return true;
			}
			break;
		case CPPSK_Typedef:
			if ((eOther == CPPSK_Variable) || (eOther == CPPSK_Field) || (eOther == CPPSK_Function) || (eOther == CPPSK_FunctionTemplate) || (eOther == CPPSK_Enumerator))
			{
				return true;
			}
			break;
		default:
			break;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a statement's own scope declares a variable of the name, or a
// function's parameters' scope a parameter (bParameters: a lambda's
// captures there are no parameters) (CheckRedeclaration).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::OuterDeclares(CCPPScope* pcScope, int64 lliName, bool bParameters)
{
	CCPPSymbol*	pcOther;

	for (pcOther = pcScope->FindParsed(lliName); pcOther; pcOther = pcOther->mpcNext)
	{
		if ((pcOther->mpcScope == pcScope) && (bParameters ? (pcOther->meKind == CPPSK_Parameter) : (pcOther->meKind == CPPSK_Variable)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a variable's symbol was declared extern (a block's extern int x;
// may be declared again).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsExternDeclaration(CCPPSymbol* pcSymbol)
{
	CCPPSyntax*	pcDeclaration;

	pcDeclaration = pcSymbol->mpcDeclaration;
	if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_InitDeclarator) || !pcDeclaration->mpcParent || (pcDeclaration->mpcParent->GetKind() != CPPS_SimpleDeclaration) || !((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers)
	{
		return false;
	}
	return (((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers->muiFlags & CPPDS_EXTERN) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether eToken is a decl-specifier keyword (or a qualifier or calling
// convention kept with them), and the CPPDS_ flag it sets in *puiFlag: 0
// for those that set none (__restrict, __cdecl ...).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::DeclSpecifierFlag(ECppTokens eToken, uint32* puiFlag)
{
	switch (eToken)
	{
	case CT_typedef:		*puiFlag = CPPDS_TYPEDEF;		return true;
	case CT_friend:			*puiFlag = CPPDS_FRIEND;		return true;
	case CT_constexpr:		*puiFlag = CPPDS_CONSTEXPR;		return true;
	case CT_static:			*puiFlag = CPPDS_STATIC;		return true;
	case CT_extern:			*puiFlag = CPPDS_EXTERN;		return true;
	case CT_register:		*puiFlag = CPPDS_REGISTER;		return true;
	case CT_mutable:		*puiFlag = CPPDS_MUTABLE;		return true;
	case CT_thread_local:
	case CT___thread:		*puiFlag = CPPDS_THREAD_LOCAL;	return true;
	case CT_inline:
	case CT___inline:
	case CT___inline__:
	case CT___forceinline:	*puiFlag = CPPDS_INLINE;		return true;
	case CT_virtual:		*puiFlag = CPPDS_VIRTUAL;		return true;
	case CT_explicit:		*puiFlag = CPPDS_EXPLICIT;		return true;
	case CT_const:
	case CT___const__:		*puiFlag = CPPDS_CONST;			return true;
	case CT_volatile:
	case CT___volatile__:	*puiFlag = CPPDS_VOLATILE;		return true;
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
	case CT___vectorcall:	*puiFlag = 0;					return true;
	default:												return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsDeclSpecifierKeyword(ECppTokens eToken)
{
	uint32	uiFlag;

	return DeclSpecifierFlag(eToken, &uiFlag);
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
bool CCPPParser::IsTypeStartKeyword(ECppTokens eToken)
{
	//A keyword that can begin a type: a simple type, a class key, enum, typename, decltype or g++'s __typeof__.
	return IsSimpleTypeKeyword(eToken) || (eToken == CT_class) || (eToken == CT_struct) || (eToken == CT_union) || (eToken == CT_enum) || (eToken == CT_typename) || (eToken == CT_decltype) || (eToken == CT___typeof__);
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
	lliName = GetTokenIdentifier(pcLast->miName);

	if (uiNumComponents >= 2)
	{
		pcPrevious = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(uiNumComponents - 2);
		return (pcPrevious->meKind == CPPNC_Identifier) && (pcPrevious->miName != -1) && (GetTokenIdentifier(pcPrevious->miName) == lliName);
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
// What = default is written on, as far as the declaration alone tells: a
// constructor, a destructor or an operator=, with no default argument.  An
// error if not, said here as a class nothing uses is never laid out by the
// generator (CCPPFunctionDeclarer::CheckDefaultedOrDeleted says the rest,
// of one that is).  [dcl.fct.def.default] 1
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckDefaulted(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcLast;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	size						i;

	pcName = pcDeclarator->GetName();
	pcLast = pcName ? pcName->GetLast() : NULL;
	if (!pcLast)
	{
		return;
	}
	if ((pcLast->meKind != CPPNC_Destructor) && !IsConstructorName(pcName) && !((pcLast->meKind == CPPNC_Operator) && mpcTokeniser->Get(pcLast->miName)->Is(CT_Assignment)))
	{
		ErrorAt(pcLast->miFirstToken, (char*)"Only a constructor that takes nothing or its own class, a destructor or an operator= that takes its own class is = default");
		return;
	}
	pcSuffix = pcDeclarator->GetFunctionSuffix();
	for (i = 0; pcSuffix && (i < pcSuffix->macpParameters.NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		if ((pcParameter->GetKind() == CPPS_Parameter) && pcParameter->mpcDefault)
		{
			ErrorAt(pcParameter->miFirstToken, (char*)"A function that is = default has no default argument");
			return;
		}
	}
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

	pcAttribute = mpcMemory->Create<CCPPSyntaxAttribute>(pcParent);
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
			pcAttribute = mpcMemory->Create<CCPPSyntaxAttribute>(pcParent);
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
CCPPSyntaxAlignasSpecifier* CCPPParser::ParseAlignasSpecifier(CCPPSyntax* pcParent)
{
	CCPPSyntaxAlignasSpecifier*	pcAlignas;

	//alignas(n) or alignas(type).  [dcl.align]
	pcAlignas = mpcMemory->Create<CCPPSyntaxAlignasSpecifier>(pcParent);
	pcAlignas->miFirstToken = CurrentToken();
	Expect(CT_alignas);
	if (!Expect(CT_OpenRoundBracket))
	{
		return NULL;
	}
	pcAlignas->mpcAlignment = ParseTypeIdOrExpression(pcAlignas, true);
	if (!pcAlignas->mpcAlignment || !Expect(CT_CloseRoundBracket))
	{
		return NULL;
	}
	pcAlignas->miLastToken = PreviousToken();
	return pcAlignas;
}


static ECPPDeclSpecifierStep CPPSetTypeSpecifier(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxTypeSpecifier* pcType, bool* pbType);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclSpecifiers* CCPPParser::ParseDeclSpecifiers(CCPPSyntax* pcParent, ECPPDeclarationContext eContext)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxAttribute*		pcAttribute;
	CCPPToken*					pcToken;
	ECPPDeclSpecifierStep		eStep;
	bool						bType;

	//[dcl.spec]
	pcSpecifiers = mpcMemory->Create<CCPPSyntaxDeclSpecifiers>(pcParent);
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
		if (pcToken->IsReservedWord() && !(pcToken->Is(CT_ScopeResolution) && !bType))
		{
			eStep = ParseKeywordDeclSpecifier(pcSpecifiers, eContext, ((CCTReservedWord*)pcToken)->GetToken(), &bType);
		}
		else if (!bType && IsUnderlyingTypeAhead())
		{
			eStep = CPPSetTypeSpecifier(pcSpecifiers, ParseDecltypeSpecifier(pcSpecifiers, CT_decltype, true), &bType);
		}
		else if ((pcToken->IsIdentifier() || pcToken->Is(CT_ScopeResolution)) && !bType)
		{
			eStep = ParseNamedDeclSpecifier(pcSpecifiers, eContext, &bType);
		}
		else
		{
			eStep = CPPDSS_Stop;
		}
		if (eStep == CPPDSS_Failed)
		{
			return NULL;
		}
		if (eStep == CPPDSS_Stop)
		{
			break;
		}
	}

	pcSpecifiers->miLastToken = PreviousToken();
	CheckDeclSpecifiers(pcSpecifiers, eContext);
	return pcSpecifiers;
}


//////////////////////////////////////////////////////////////////////////
//
// What a declaration's specifiers may not be together or where they are:
// two storage classes; mutable on what isn't a member, or with const or
// static; virtual, explicit and friend outside a class; constexpr with
// virtual; type keywords that make no type (unsigned double, long long
// long, short char).  [dcl.stc] 1, 11, [dcl.fct.spec] 5, 6, [dcl.friend] 1,
// [dcl.constexpr] 3, [dcl.type.simple] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::CheckDeclSpecifiers(CCPPSyntaxDeclSpecifiers* pcSpecifiers, ECPPDeclarationContext eContext)
{
	uint32		uiFlags;
	uint32		uiKeys;
	uint32		uiBase;
	int			iStorage;
	const char*	szError;

	if ((miTentative > 0) || (pcSpecifiers->miFirstToken < 0) || IsSkimming(pcSpecifiers->miFirstToken))
	{
		return;
	}
	uiFlags = pcSpecifiers->muiFlags;
	uiKeys = pcSpecifiers->muiTypeKeywords;
	iStorage = ((uiFlags & CPPDS_STATIC) ? 1 : 0) + ((uiFlags & CPPDS_EXTERN) ? 1 : 0) + ((uiFlags & CPPDS_REGISTER) ? 1 : 0) + ((uiFlags & CPPDS_MUTABLE) ? 1 : 0);
	uiBase = uiKeys & (CPPTK_VOID | CPPTK_BOOL | CPPTK_CHAR | CPPTK_WCHAR_T | CPPTK_CHAR16_T | CPPTK_CHAR32_T | CPPTK_INT | CPPTK_FLOAT | CPPTK_DOUBLE | CPPTK_AUTO);
	szError = NULL;
	if ((iStorage > 1) || ((uiFlags & CPPDS_THREAD_LOCAL) && (uiFlags & (CPPDS_REGISTER | CPPDS_MUTABLE))))
	{
		szError = "A declaration has one storage class: static, extern, register or mutable (and thread_local with static or extern)";
	}
	else if ((uiFlags & CPPDS_MUTABLE) && ((eContext != CPPDC_Class) || (uiFlags & (CPPDS_CONST | CPPDS_STATIC))))
	{
		szError = "mutable is for a class's member that is neither static nor const";
	}
	else if ((uiFlags & (CPPDS_VIRTUAL | CPPDS_EXPLICIT | CPPDS_FRIEND)) && (eContext != CPPDC_Class))
	{
		szError = "virtual, explicit and friend are written only in a class";
	}
	else if ((uiFlags & CPPDS_CONSTEXPR) && (uiFlags & CPPDS_VIRTUAL))
	{
		szError = "A constexpr function isn't virtual";
	}
	else if ((uiBase & (uiBase - 1)) || (pcSpecifiers->miLongs > 2) || ((uiKeys & CPPTK_SIGNED) && (uiKeys & CPPTK_UNSIGNED)) ||
			 ((pcSpecifiers->miLongs > 0) && (uiKeys & (CPPTK_SHORT | CPPTK_VOID | CPPTK_BOOL | CPPTK_CHAR | CPPTK_WCHAR_T | CPPTK_CHAR16_T | CPPTK_CHAR32_T | CPPTK_FLOAT | CPPTK_AUTO))) ||
			 ((pcSpecifiers->miLongs > 1) && (uiKeys & CPPTK_DOUBLE)) ||
			 ((uiKeys & CPPTK_SHORT) && (uiKeys & (CPPTK_VOID | CPPTK_BOOL | CPPTK_CHAR | CPPTK_WCHAR_T | CPPTK_CHAR16_T | CPPTK_CHAR32_T | CPPTK_FLOAT | CPPTK_DOUBLE | CPPTK_AUTO))) ||
			 ((uiKeys & (CPPTK_SIGNED | CPPTK_UNSIGNED)) && (uiKeys & (CPPTK_VOID | CPPTK_BOOL | CPPTK_WCHAR_T | CPPTK_CHAR16_T | CPPTK_CHAR32_T | CPPTK_FLOAT | CPPTK_DOUBLE | CPPTK_AUTO))))
	{
		szError = "These type specifiers make no type together";
	}
	if (szError)
	{
		ErrorAt(pcSpecifiers->miFirstToken, (char*)szError);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The keyword at the current token as a specifier of its own.
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::AddKeywordSpecifier(CCPPSyntaxDeclSpecifiers* pcSpecifiers, ECppTokens eToken)
{
	CCPPSyntaxKeywordSpecifier*	pcKeyword;

	pcKeyword = mpcMemory->Create<CCPPSyntaxKeywordSpecifier>(pcSpecifiers);
	pcKeyword->meKeyword = eToken;
	pcKeyword->SetTokens(CurrentToken(), CurrentToken());
	pcSpecifiers->macpSpecifiers.Add(pcKeyword);
	Next();
}


//////////////////////////////////////////////////////////////////////////
//
// pcType is the declaration's type.
//
//////////////////////////////////////////////////////////////////////////
static ECPPDeclSpecifierStep CPPSetTypeSpecifier(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxTypeSpecifier* pcType, bool* pbType)
{
	if (!pcType)
	{
		return CPPDSS_Failed;
	}
	pcSpecifiers->mpcType = pcType;
	pcSpecifiers->macpSpecifiers.Add(pcType);
	*pbType = true;
	return CPPDSS_Next;
}


//////////////////////////////////////////////////////////////////////////
//
// The specifier a keyword, eToken, starts.  CPPDSS_Stop if it doesn't start
// one (or another type can't be here).
//
//////////////////////////////////////////////////////////////////////////
ECPPDeclSpecifierStep CCPPParser::ParseKeywordDeclSpecifier(CCPPSyntaxDeclSpecifiers* pcSpecifiers, ECPPDeclarationContext eContext, ECppTokens eToken, bool* pbType)
{
	CCPPSyntaxAlignasSpecifier*	pcAlignas;
	uint32						uiFlag;

	if (DeclSpecifierFlag(eToken, &uiFlag) && ((eContext != CPPDC_TypeId) || (uiFlag == CPPDS_CONST) || (uiFlag == CPPDS_VOLATILE)))
	{
		pcSpecifiers->muiFlags |= uiFlag;
		AddKeywordSpecifier(pcSpecifiers, eToken);
		return CPPDSS_Next;
	}
	if (IsSimpleTypeKeyword(eToken))
	{
		pcSpecifiers->CountTypeKeyword(eToken);
		AddKeywordSpecifier(pcSpecifiers, eToken);
		*pbType = true;
		return CPPDSS_Next;
	}
	if ((eToken == CT_class) || (eToken == CT_struct) || (eToken == CT_union))
	{
		if (*pbType && !pcSpecifiers->mpcType)
		{
			return CPPDSS_Stop;
		}
		return CPPSetTypeSpecifier(pcSpecifiers, ParseClassSpecifier(pcSpecifiers, eContext, (pcSpecifiers->muiFlags & CPPDS_FRIEND) != 0), pbType);
	}
	if (eToken == CT_enum)
	{
		return CPPSetTypeSpecifier(pcSpecifiers, ParseEnumSpecifier(pcSpecifiers, eContext), pbType);
	}
	if ((eToken == CT_typename) && !*pbType)
	{
		return CPPSetTypeSpecifier(pcSpecifiers, ParseTypenameSpecifier(pcSpecifiers), pbType);
	}
	if ((eToken == CT_decltype) && !*pbType && IsDecltypeQualifier())
	{
		//decltype(d)::T, a type that is a member of d's.
		return ParseNamedDeclSpecifier(pcSpecifiers, eContext, pbType);
	}
	if (((eToken == CT_decltype) || (eToken == CT___typeof__)) && !*pbType)
	{
		return CPPSetTypeSpecifier(pcSpecifiers, ParseDecltypeSpecifier(pcSpecifiers, eToken, false), pbType);
	}
	if (eToken == CT_alignas)
	{
		pcAlignas = ParseAlignasSpecifier(pcSpecifiers);
		if (!pcAlignas)
		{
			return CPPDSS_Failed;
		}
		pcSpecifiers->macpSpecifiers.Add(pcAlignas);
		return CPPDSS_Next;
	}
	return CPPDSS_Stop;
}


//////////////////////////////////////////////////////////////////////////
//
// typename X::Y  [temp.res]
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxNamedTypeSpecifier* CCPPParser::ParseTypenameSpecifier(CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	CCPPSyntaxNamedTypeSpecifier*	pcNamed;

	AddKeywordSpecifier(pcSpecifiers, CT_typename);
	pcNamed = mpcMemory->Create<CCPPSyntaxNamedTypeSpecifier>(pcSpecifiers);
	pcNamed->miFirstToken = CurrentToken();
	pcNamed->mpcName = ParseName(pcNamed, false, NULL);
	if (!pcNamed->mpcName)
	{
		return NULL;
	}
	pcNamed->miLastToken = PreviousToken();
	return pcNamed;
}


//////////////////////////////////////////////////////////////////////////
//
// decltype(e) and g++'s __typeof__(e) or __typeof__(type).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDecltypeSpecifier* CCPPParser::ParseDecltypeSpecifier(CCPPSyntax* pcParent, ECppTokens eToken, bool bUnderlyingType)
{
	CCPPSyntaxDecltypeSpecifier*	pcDecltype;
	bool							bNoGreater;
	bool							bExpression;

	pcDecltype = mpcMemory->Create<CCPPSyntaxDecltypeSpecifier>(pcParent);
	pcDecltype->miFirstToken = CurrentToken();
	pcDecltype->meKeyword = eToken;
	pcDecltype->mbUnderlyingType = bUnderlyingType;
	Next();
	if (!Expect(CT_OpenRoundBracket))
	{
		return NULL;
	}

	//decltype's operand is an expression (S() makes an S; it is no function's type), or auto; __typeof__'s may be a
	//type and __underlying_type's is one.  In the brackets a '>' is an operator, though the specifier is a template
	//argument: X<decltype(a >> 1)>.
	bExpression = (eToken == CT_decltype) && !bUnderlyingType && !Is(CT_auto);
	bNoGreater = mbNoGreater;
	mbNoGreater = false;
	pcDecltype->mpcOperand = bExpression ? ParseExpression(pcDecltype) : ParseTypeIdOrExpression(pcDecltype, false);
	mbNoGreater = bNoGreater;
	if (!pcDecltype->mpcOperand || !Expect(CT_CloseRoundBracket))
	{
		return NULL;
	}
	pcDecltype->miLastToken = PreviousToken();
	return pcDecltype;
}


//////////////////////////////////////////////////////////////////////////
//
// A type name, unless it is the name being declared (CPPDSS_Stop, with the
// parser back where it was).
//
//////////////////////////////////////////////////////////////////////////
ECPPDeclSpecifierStep CCPPParser::ParseNamedDeclSpecifier(CCPPSyntaxDeclSpecifiers* pcSpecifiers, ECPPDeclarationContext eContext, bool* pbType)
{
	CCPPSyntaxNamedTypeSpecifier*	pcNamed;
	CCPPSyntaxAttribute*			pcAttribute;
	CCPPSyntaxName*					pcName;
	CCPPSymbol*						pcSymbol;
	SCPPParserPosition				sPosition;
	int								iErrors;

	sPosition = Mark();
	pcNamed = mpcMemory->Create<CCPPSyntaxNamedTypeSpecifier>(pcSpecifiers);
	pcNamed->miFirstToken = CurrentToken();

	iErrors = miErrors + miSkimmedErrors;
	pcName = ParseName(pcNamed, false, NULL);
	if (!pcName)
	{
		if (iErrors != miErrors + miSkimmedErrors)
		{
			//The error is reported; the declarator would report it again.
			return CPPDSS_Failed;
		}
		Rewind(sPosition);
		return CPPDSS_Stop;
	}

	if ((IsConstructorName(pcName) && Is(CT_OpenRoundBracket)) || (pcName->GetLast()->meKind != CPPNC_Identifier))
	{
		Rewind(sPosition);
		return CPPDSS_Stop;
	}
	if ((miTentative == 0) && (pcName->macpComponents.NumElements() >= 2) && IsConstructorName(pcName) && pcName->mpcSymbol && pcName->mpcSymbol->IsClass())
	{
		//A::A where a type can be written names A's constructor, not A.  [class.qual] 2  A tentative parse takes it as
		//the type it was meant to be, so the declaration is parsed as one and this said.
		ErrorAt(pcName->GetLast()->miFirstToken, (char*)"A::A names the class's constructor, not the class");
		return CPPDSS_Failed;
	}

	pcSymbol = pcName->mpcSymbol;
	if (!pcSymbol && IsSkimming(sPosition.iToken) && Peek(0) && Peek(0)->IsReservedWord() && pcName->IsSimple() && (IsDeclSpecifierKeyword(((CCTReservedWord*)Peek(0))->GetToken()) || IsSimpleTypeKeyword(((CCTReservedWord*)Peek(0))->GetToken())))
	{
		//An unknown name before a type keyword in a skimmed header is an undefined macro, as in __MINGW_IMPORT int x;
		pcAttribute = mpcMemory->Create<CCPPSyntaxAttribute>(pcSpecifiers);
		pcAttribute->SetTokens(sPosition.iToken, sPosition.iToken);
		pcSpecifiers->macpSpecifiers.Add(pcAttribute);
		return CPPDSS_Next;
	}

	//Where only a type can be written, decltype(d)::T is one: the parser doesn't know what d's members are.
	//After operator only a type can be written, s.operator I() naming the object's class's I.  [basic.lookup.classref] 7
	if (pcSymbol ? !pcSymbol->IsType() : !(IsUnknownNameAType(eContext) || ((eContext == CPPDC_TypeId) && (pcName->IsQualifiedByDecltype() || mbConversionType))))
	{
		Rewind(sPosition);
		return CPPDSS_Stop;
	}

	pcNamed->mpcName = pcName;
	pcNamed->miLastToken = PreviousToken();
	return CPPSetTypeSpecifier(pcSpecifiers, pcNamed, pbType);
}


//////////////////////////////////////////////////////////////////////////
//
// An unknown name, from a dependent scope or a skimmed header, is a type if a
// declarator follows.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsUnknownNameAType(ECPPDeclarationContext eContext)
{
	if (IsIdentifier() || ((eContext != CPPDC_Block) && (eContext != CPPDC_Condition) && (eContext != CPPDC_ForInit) && (Is(CT_Multiply) || Is(CT_BitwiseAnd) || Is(CT_LogicalAnd) || Is(CT___cdecl) || Is(CT___stdcall))))
	{
		return true;
	}
	return (eContext == CPPDC_Parameter) && (Is(CT_Comma) || Is(CT_CloseRoundBracket) || Is(CT_OpenSquareBracket));
}

