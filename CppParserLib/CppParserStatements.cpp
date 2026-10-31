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
CCPPSyntaxCompoundStatement* CCPPParser::ParseCompoundStatement(CCPPSyntax* pcParent, bool bNewScope)
{
	CCPPSyntaxCompoundStatement*	pcCompound;
	CCPPSyntaxStatement*			pcStatement;
	CCPPScope*						pcScope;

	//{ statements }  [stmt.block]
	pcCompound = mpcMemory->Create<CCPPSyntaxCompoundStatement>(pcParent);
	pcCompound->miFirstToken = CurrentToken();
	if (!Expect(CT_OpenCurlyBracket))
	{
		return NULL;
	}

	pcScope = NULL;
	if (bNewScope)
	{
		pcScope = PushScope(CPPSC_Block, NULL);
		pcCompound->mpcScope = pcScope;
	}

	while (!IsEnd() && !Is(CT_CloseCurlyBracket))
	{
		pcStatement = ParseStatement(pcCompound);
		if (!pcStatement)
		{
			PopScope(pcScope);
			return NULL;
		}
		pcCompound->macpStatements.Add(pcStatement);
	}

	PopScope(pcScope);
	if (!Expect(CT_CloseCurlyBracket))
	{
		return NULL;
	}
	pcCompound->miLastToken = PreviousToken();
	return pcCompound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxStatement* CCPPParser::ParseStatement(CCPPSyntax* pcParent)
{
	CCPPSyntaxStatement*			pcStatement;
	CCPPSyntaxExpressionStatement*	pcExpression;
	CCPPSyntaxLabelStatement*		pcLabel;
	CCPPSyntaxBreakStatement*		pcBreak;
	CCPPSyntaxContinueStatement*	pcContinue;
	CCPPSyntaxGotoStatement*		pcGoto;
	CCPPSyntaxSkippedStatement*		pcSkipped;
	CCPPSyntaxAttributedStatement*	pcAttributed;
	SCPPParserPosition				sStart;
	CCPPScope*						pcScope;
	int								iErrors;
	bool							bNoGreater;

	//[stmt.stmt]  A statement that can't be parsed is skipped and kept as its tokens.
	sStart = Mark();
	pcScope = mpcScope;
	iErrors = miErrors;
	bNoGreater = mbNoGreater;
	mbNoGreater = false;
	pcStatement = NULL;

	if (Is(CT_OpenSquareBracket) && IsAhead(1, CT_OpenSquareBracket) && !IsDeclarationStatement(CPPDC_Block))
	{
		//[[attributes]] statement, as [[fallthrough]];  [stmt.pre]
		pcAttributed = mpcMemory->Create<CCPPSyntaxAttributedStatement>(pcParent);
		pcAttributed->miFirstToken = CurrentToken();
		ParseAttributes(&pcAttributed->macpAttributes, pcAttributed);
		pcAttributed->mpcStatement = ParseStatement(pcAttributed);
		if (pcAttributed->mpcStatement)
		{
			pcAttributed->miLastToken = PreviousToken();
			pcStatement = pcAttributed;
		}
	}
	else if (Is(CT_OpenCurlyBracket))
	{
		pcStatement = ParseCompoundStatement(pcParent, true);
	}
	else if (Is(CT_if))
	{
		pcStatement = ParseIfStatement(pcParent);
	}
	else if (Is(CT_switch))
	{
		pcStatement = ParseConditionStatement<CCPPSyntaxSwitchStatement>(pcParent, CT_switch);
	}
	else if (Is(CT_while))
	{
		pcStatement = ParseConditionStatement<CCPPSyntaxWhileStatement>(pcParent, CT_while);
	}
	else if (Is(CT_do))
	{
		pcStatement = ParseDoStatement(pcParent);
	}
	else if (Is(CT_for))
	{
		pcStatement = ParseForStatement(pcParent);
	}
	else if (Is(CT_return))
	{
		pcStatement = ParseReturnStatement(pcParent);
	}
	else if (Is(CT_case) || Is(CT_default))
	{
		pcStatement = ParseCaseStatement(pcParent);
	}
	else if (Is(CT_try))
	{
		pcStatement = ParseTryStatement(pcParent);
	}
	else if (Is(CT_break))
	{
		pcBreak = mpcMemory->Create<CCPPSyntaxBreakStatement>(pcParent);
		pcBreak->miFirstToken = CurrentToken();
		Next();
		if (Expect(CT_SemiColon))
		{
			pcBreak->miLastToken = PreviousToken();
			pcStatement = pcBreak;
		}
	}
	else if (Is(CT_continue))
	{
		pcContinue = mpcMemory->Create<CCPPSyntaxContinueStatement>(pcParent);
		pcContinue->miFirstToken = CurrentToken();
		Next();
		if (Expect(CT_SemiColon))
		{
			pcContinue->miLastToken = PreviousToken();
			pcStatement = pcContinue;
		}
	}
	else if (Is(CT_goto))
	{
		pcGoto = mpcMemory->Create<CCPPSyntaxGotoStatement>(pcParent);
		pcGoto->miFirstToken = CurrentToken();
		Next();
		if (IsIdentifier())
		{
			pcGoto->miLabel = CurrentToken();
			Next();
			if (Expect(CT_SemiColon))
			{
				pcGoto->miLastToken = PreviousToken();
				pcStatement = pcGoto;
			}
		}
		else
		{
			Error("Expected a label.");
		}
	}
	else if (Is(CT_SemiColon))
	{
		pcExpression = mpcMemory->Create<CCPPSyntaxExpressionStatement>(pcParent);
		pcExpression->SetTokens(CurrentToken(), CurrentToken());
		Next();
		pcStatement = pcExpression;
	}
	else if (IsIdentifier() && IsAhead(1, CT_Colon))
	{
		//label: statement  [stmt.label]
		pcLabel = mpcMemory->Create<CCPPSyntaxLabelStatement>(pcParent);
		pcLabel->miFirstToken = CurrentToken();
		pcLabel->miName = CurrentToken();
		Next();
		Next();
		if (Is(CT_CloseCurlyBracket))
		{
			pcLabel->miLastToken = PreviousToken();
			pcStatement = pcLabel;
		}
		else
		{
			pcLabel->mpcStatement = ParseStatement(pcLabel);
			if (pcLabel->mpcStatement)
			{
				pcLabel->miLastToken = PreviousToken();
				pcStatement = pcLabel;
			}
		}
	}
	else if ((Is(CT_asm) || Is(CT___asm__)) && (IsAhead(1, CT_OpenRoundBracket) || IsAhead(1, CT_volatile) || IsAhead(1, CT___volatile__)))
	{
		//asm statements are kept as their tokens.
		Next();
		while (Is(CT_volatile) || Is(CT___volatile__) || Is(CT_goto) || Is(CT_inline))
		{
			Next();
		}
		if (Is(CT_OpenRoundBracket) && SkipBalanced() && Expect(CT_SemiColon))
		{
			pcSkipped = mpcMemory->Create<CCPPSyntaxSkippedStatement>(pcParent);
			pcSkipped->SetTokens(sStart.iToken, PreviousToken());
			pcStatement = pcSkipped;
		}
	}
	else if (IsDeclarationStatement(CPPDC_Block))
	{
		pcStatement = ParseDeclarationStatement(pcParent, CPPDC_Block);
	}
	else
	{
		//expression ;  [stmt.expr]
		pcExpression = mpcMemory->Create<CCPPSyntaxExpressionStatement>(pcParent);
		pcExpression->miFirstToken = CurrentToken();
		pcExpression->mpcExpression = ParseExpression(pcExpression);
		if (pcExpression->mpcExpression && Expect(CT_SemiColon))
		{
			pcExpression->miLastToken = PreviousToken();
			pcStatement = pcExpression;
		}
	}

	mbNoGreater = bNoGreater;
	if (pcStatement)
	{
		return pcStatement;
	}

	if (miTentative > 0)
	{
		return NULL;
	}

	//Recover: skip the statement and keep its tokens.
	mpcScope = pcScope;
	Rewind(sStart);
	SkipStatement();
	if (CurrentToken() == sStart.iToken)
	{
		Next();
	}
	if (miErrors == iErrors)
	{
		ErrorAt(sStart.iToken, "Could not parse the statement.");
	}
	pcSkipped = mpcMemory->Create<CCPPSyntaxSkippedStatement>(pcParent);
	pcSkipped->SetTokens(sStart.iToken, PreviousToken());
	return pcSkipped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsDeclarationStatement(ECPPDeclarationContext eContext)
{
	CCPPToken*					pcToken;
	ECppTokens					eToken;
	SCPPTentative				sTentative;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxDeclarator*		pcDeclarator;
	bool						bFollows;

	//A statement that could be a declaration is a declaration.  [stmt.ambig]
	pcToken = Peek(0);
	if (!pcToken)
	{
		return false;
	}

	if (Is(CT_OpenSquareBracket) && IsAhead(1, CT_OpenSquareBracket))
	{
		//[[ starts attributes, never a lambda.  It is a declaration if one follows them.  [dcl.attr.grammar] 7
		BeginTentative(&sTentative);
		while (Is(CT_OpenSquareBracket) && IsAhead(1, CT_OpenSquareBracket))
		{
			if (!SkipBalanced())
			{
				return EndTentative(&sTentative, false);
			}
		}
		bFollows = IsDeclarationStatement(eContext);
		return EndTentative(&sTentative, bFollows);
	}

	if (pcToken->IsReservedWord())
	{
		eToken = ((CCTReservedWord*)pcToken)->GetToken();
		switch (eToken)
		{
		case CT_typedef:
		case CT_static:
		case CT_extern:
		case CT_register:
		case CT_mutable:
		case CT_thread_local:
		case CT___thread:
		case CT_inline:
		case CT_virtual:
		case CT_friend:
		case CT_constexpr:
		case CT_const:
		case CT___const__:
		case CT_volatile:
		case CT___volatile__:
		case CT_class:
		case CT_struct:
		case CT_union:
		case CT_enum:
		case CT_using:
		case CT_static_assert:
		case CT_template:
		case CT_namespace:
		case CT___declspec:
		case CT___attribute__:
		case CT___extension__:
		case CT_alignas:
			return true;
		default:
			break;
		}
		if (!IsTypeStartKeyword(eToken) && (eToken != CT_ScopeResolution))
		{
			return false;
		}
	}
	else if (pcToken->IsIdentifier())
	{
		if (!IsTypeNameAhead() && !IsIdentifierAhead(1))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	if (Is(CT_ScopeResolution) && !IsTypeNameAhead())
	{
		return false;
	}

	BeginTentative(&sTentative);
	pcSpecifiers = ParseDeclSpecifiers(NULL, eContext);
	pcDeclarator = NULL;
	if (pcSpecifiers && (pcSpecifiers->macpSpecifiers.NumElements() > 0))
	{
		pcDeclarator = ParseDeclarator(NULL, CPPDK_Named, eContext);
	}
	bFollows = Is(CT_SemiColon) || Is(CT_Comma) || Is(CT_Assignment) || Is(CT_OpenCurlyBracket) || Is(CT_OpenRoundBracket) || Is(CT_OpenSquareBracket) || ((eContext == CPPDC_ForInit) && Is(CT_Colon));
	if ((eContext == CPPDC_Condition) && !(Is(CT_Assignment) || Is(CT_OpenCurlyBracket)))
	{
		bFollows = false;
	}
	return EndTentative(&sTentative, pcDeclarator && pcDeclarator->GetName() && bFollows);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxStatement* CCPPParser::ParseDeclarationStatement(CCPPSyntax* pcParent, ECPPDeclarationContext eContext)
{
	CCPPSyntaxDeclarationStatement*	pcStatement;

	//[stmt.dcl]
	pcStatement = mpcMemory->Create<CCPPSyntaxDeclarationStatement>(pcParent);
	pcStatement->miFirstToken = CurrentToken();
	pcStatement->mpcDeclaration = ParseDeclaration(pcStatement, eContext);
	if (!pcStatement->mpcDeclaration)
	{
		return NULL;
	}
	pcStatement->miLastToken = PreviousToken();
	return pcStatement;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPParser::ParseCondition(CCPPSyntax* pcParent)
{
	//An expression, or a declaration with an initialiser, as in if (CFoo* p = Get()).  [stmt.select]
	if (IsDeclarationStatement(CPPDC_Condition))
	{
		return ParseSimpleDeclaration(pcParent, CPPDC_Condition);
	}
	return ParseExpression(pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxStatement* CCPPParser::ParseIfStatement(CCPPSyntax* pcParent)
{
	CCPPSyntaxIfStatement*	pcIf;

	//if ( condition ) statement else statement  [stmt.if]
	pcIf = mpcMemory->Create<CCPPSyntaxIfStatement>(pcParent);
	pcIf->miFirstToken = CurrentToken();
	Expect(CT_if);
	if (!Expect(CT_OpenRoundBracket))
	{
		return NULL;
	}
	pcIf->mpcScope = PushScope(CPPSC_Block, NULL);
	pcIf->mpcCondition = ParseCondition(pcIf);
	if (!pcIf->mpcCondition || !Expect(CT_CloseRoundBracket))
	{
		PopScope(pcIf->mpcScope);
		return NULL;
	}
	pcIf->mpcThen = ParseStatement(pcIf);
	if (pcIf->mpcThen && Accept(CT_else))
	{
		pcIf->mpcElse = ParseStatement(pcIf);
		if (!pcIf->mpcElse)
		{
			PopScope(pcIf->mpcScope);
			return NULL;
		}
	}
	PopScope(pcIf->mpcScope);
	if (!pcIf->mpcThen)
	{
		return NULL;
	}
	pcIf->miLastToken = PreviousToken();
	return pcIf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxStatement* CCPPParser::ParseDoStatement(CCPPSyntax* pcParent)
{
	CCPPSyntaxDoStatement*	pcDo;

	//do statement while ( expression ) ;  [stmt.do]
	pcDo = mpcMemory->Create<CCPPSyntaxDoStatement>(pcParent);
	pcDo->miFirstToken = CurrentToken();
	Expect(CT_do);
	pcDo->mpcBody = ParseStatement(pcDo);
	if (!pcDo->mpcBody || !Expect(CT_while) || !Expect(CT_OpenRoundBracket))
	{
		return NULL;
	}
	pcDo->mpcCondition = ParseExpression(pcDo);
	if (!pcDo->mpcCondition || !Expect(CT_CloseRoundBracket) || !Expect(CT_SemiColon))
	{
		return NULL;
	}
	pcDo->miLastToken = PreviousToken();
	return pcDo;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxStatement* CCPPParser::ParseForStatement(CCPPSyntax* pcParent)
{
	CCPPSyntaxForStatement*			pcFor;
	CCPPSyntaxRangeForStatement*	pcRange;
	CCPPSyntaxExpressionStatement*	pcInit;
	CCPPSyntaxDeclarationStatement*	pcDeclaration;
	CCPPScope*						pcScope;
	SCPPTentative					sTentative;
	int								iFirstToken;
	bool							bRange;

	//for ( init condition ; increment ) statement  [stmt.for]  for ( declaration : range ) statement  [stmt.ranged]
	iFirstToken = CurrentToken();
	Expect(CT_for);
	if (!Expect(CT_OpenRoundBracket))
	{
		return NULL;
	}
	pcScope = PushScope(CPPSC_Block, NULL);

	if (IsDeclarationStatement(CPPDC_ForInit))
	{
		BeginTentative(&sTentative);
		ParseDeclSpecifiers(NULL, CPPDC_ForInit);
		ParseDeclarator(NULL, CPPDK_Named, CPPDC_ForInit);
		bRange = EndTentative(&sTentative, Is(CT_Colon));

		if (bRange)
		{
			pcRange = mpcMemory->Create<CCPPSyntaxRangeForStatement>(pcParent);
			pcRange->miFirstToken = iFirstToken;
			pcRange->mpcScope = pcScope;
			pcRange->mpcSpecifiers = ParseDeclSpecifiers(pcRange, CPPDC_ForInit);
			if (!pcRange->mpcSpecifiers)
			{
				PopScope(pcScope);
				return NULL;
			}
			pcRange->mpcDeclarator = ParseDeclarator(pcRange, CPPDK_Named, CPPDC_ForInit);
			if (!pcRange->mpcDeclarator || !Expect(CT_Colon))
			{
				PopScope(pcScope);
				return NULL;
			}
			DeclareDeclarator(pcRange->mpcSpecifiers, pcRange->mpcDeclarator, pcRange, CPPDC_ForInit);
			pcRange->mpcRange = ParseInitializerClause(pcRange);
			if (!pcRange->mpcRange || !Expect(CT_CloseRoundBracket))
			{
				PopScope(pcScope);
				return NULL;
			}
			pcRange->mpcBody = ParseStatement(pcRange);
			PopScope(pcScope);
			if (!pcRange->mpcBody)
			{
				return NULL;
			}
			pcRange->miLastToken = PreviousToken();
			return pcRange;
		}
	}

	pcFor = mpcMemory->Create<CCPPSyntaxForStatement>(pcParent);
	pcFor->miFirstToken = iFirstToken;
	pcFor->mpcScope = pcScope;

	if (Is(CT_SemiColon))
	{
		Next();
	}
	else if (IsDeclarationStatement(CPPDC_ForInit))
	{
		pcDeclaration = mpcMemory->Create<CCPPSyntaxDeclarationStatement>(pcFor);
		pcDeclaration->miFirstToken = CurrentToken();
		pcDeclaration->mpcDeclaration = ParseSimpleDeclaration(pcDeclaration, CPPDC_ForInit);
		if (!pcDeclaration->mpcDeclaration)
		{
			PopScope(pcScope);
			return NULL;
		}
		pcDeclaration->miLastToken = PreviousToken();
		pcFor->mpcInitialiser = pcDeclaration;
	}
	else
	{
		pcInit = mpcMemory->Create<CCPPSyntaxExpressionStatement>(pcFor);
		pcInit->miFirstToken = CurrentToken();
		pcInit->mpcExpression = ParseExpression(pcInit);
		if (!pcInit->mpcExpression || !Expect(CT_SemiColon))
		{
			PopScope(pcScope);
			return NULL;
		}
		pcInit->miLastToken = PreviousToken();
		pcFor->mpcInitialiser = pcInit;
	}

	if (!Is(CT_SemiColon))
	{
		pcFor->mpcCondition = ParseCondition(pcFor);
		if (!pcFor->mpcCondition)
		{
			PopScope(pcScope);
			return NULL;
		}
	}
	if (!Expect(CT_SemiColon))
	{
		PopScope(pcScope);
		return NULL;
	}

	if (!Is(CT_CloseRoundBracket))
	{
		pcFor->mpcIncrement = ParseExpression(pcFor);
		if (!pcFor->mpcIncrement)
		{
			PopScope(pcScope);
			return NULL;
		}
	}
	if (!Expect(CT_CloseRoundBracket))
	{
		PopScope(pcScope);
		return NULL;
	}

	pcFor->mpcBody = ParseStatement(pcFor);
	PopScope(pcScope);
	if (!pcFor->mpcBody)
	{
		return NULL;
	}
	pcFor->miLastToken = PreviousToken();
	return pcFor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxStatement* CCPPParser::ParseReturnStatement(CCPPSyntax* pcParent)
{
	CCPPSyntaxReturnStatement*	pcReturn;

	//return expression ; or return braced-init-list ;  [stmt.return]
	pcReturn = mpcMemory->Create<CCPPSyntaxReturnStatement>(pcParent);
	pcReturn->miFirstToken = CurrentToken();
	Expect(CT_return);
	if (!Is(CT_SemiColon))
	{
		if (Is(CT_OpenCurlyBracket))
		{
			pcReturn->mpcValue = ParseBracedInitList(pcReturn);
		}
		else
		{
			pcReturn->mpcValue = ParseExpression(pcReturn);
		}
		if (!pcReturn->mpcValue)
		{
			return NULL;
		}
	}
	if (!Expect(CT_SemiColon))
	{
		return NULL;
	}
	pcReturn->miLastToken = PreviousToken();
	return pcReturn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxStatement* CCPPParser::ParseCaseStatement(CCPPSyntax* pcParent)
{
	CCPPSyntaxCaseStatement*	pcCase;

	//case constant-expression : statement and default : statement  [stmt.label]
	pcCase = mpcMemory->Create<CCPPSyntaxCaseStatement>(pcParent);
	pcCase->miFirstToken = CurrentToken();
	if (Accept(CT_case))
	{
		pcCase->mpcValue = ParseConstantExpression(pcCase);
		if (!pcCase->mpcValue)
		{
			return NULL;
		}
	}
	else
	{
		Expect(CT_default);
	}
	if (!Expect(CT_Colon))
	{
		return NULL;
	}

	//A label at the end of a block labels nothing; compilers allow it.
	if (!Is(CT_CloseCurlyBracket))
	{
		pcCase->mpcStatement = ParseStatement(pcCase);
		if (!pcCase->mpcStatement)
		{
			return NULL;
		}
	}
	pcCase->miLastToken = PreviousToken();
	return pcCase;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxStatement* CCPPParser::ParseTryStatement(CCPPSyntax* pcParent)
{
	CCPPSyntaxTryStatement*		pcTry;
	CCPPSyntaxCatchHandler*		pcHandler;

	//try compound-statement handlers  [except]
	pcTry = mpcMemory->Create<CCPPSyntaxTryStatement>(pcParent);
	pcTry->miFirstToken = CurrentToken();
	Expect(CT_try);
	pcTry->mpcBlock = ParseCompoundStatement(pcTry, true);
	if (!pcTry->mpcBlock)
	{
		return NULL;
	}

	while (Is(CT_catch))
	{
		pcHandler = mpcMemory->Create<CCPPSyntaxCatchHandler>(pcTry);
		pcHandler->miFirstToken = CurrentToken();
		Next();
		if (!Expect(CT_OpenRoundBracket))
		{
			return NULL;
		}
		pcHandler->mpcScope = PushScope(CPPSC_Block, NULL);
		if (!Accept(CT_Elipsis))
		{
			pcHandler->mpcParameter = ParseParameter(pcHandler);
			if (!pcHandler->mpcParameter)
			{
				PopScope(pcHandler->mpcScope);
				return NULL;
			}
		}
		if (!Expect(CT_CloseRoundBracket))
		{
			PopScope(pcHandler->mpcScope);
			return NULL;
		}
		pcHandler->mpcBlock = ParseCompoundStatement(pcHandler, false);
		PopScope(pcHandler->mpcScope);
		if (!pcHandler->mpcBlock)
		{
			return NULL;
		}
		pcHandler->miLastToken = PreviousToken();
		pcTry->macpHandlers.Add(pcHandler);
	}

	if (pcTry->macpHandlers.NumElements() == 0)
	{
		Error("Expected a catch handler.");
		return NULL;
	}
	pcTry->miLastToken = PreviousToken();
	return pcTry;
}

