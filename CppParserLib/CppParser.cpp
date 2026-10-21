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
#include "BaseLib/Logger.h"
#include "SourceFile.h"
#include "CTIdentifier.h"
#include "CppParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::Init(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree)
{
	mpcTokeniser = pcTokeniser;
	mpcTree = pcTree;
	mpcMemory = pcTree->GetMemory();
	mpcSymbols = pcTree->GetSymbols();
	mpcScope = mpcSymbols->GetGlobalScope();
	miNumTokens = (int)pcTokeniser->NumTokens();

	miToken = 0;
	mbHalf = false;
	mcGreaterThan.Init(CT_GreaterThan);
	mcGreaterThan.SetSource(-1);

	miTentative = 0;
	mbFailed = false;
	mbNoGreater = false;
	mbMemberName = false;
	miClassDepth = 0;
	mbDeclaringTemplate = false;
	macDeferred.Init();
	miErrors = 0;
	miSkimmedErrors = 0;
	mbReportSkimmed = false;
	mlliFunction = pcTokeniser->GetIdentifierID("__FUNCTION__");
	mlliFuncsig = pcTokeniser->GetIdentifierID("__FUNCSIG__");
	mlliFuncdname = pcTokeniser->GetIdentifierID("__FUNCDNAME__");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::Kill(void)
{
	macDeferred.Kill();
	mpcScope = NULL;
	mpcSymbols = NULL;
	mpcMemory = NULL;
	mpcTree = NULL;
	mpcTokeniser = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::Parse(void)
{
	CCPPSyntaxTranslationUnit*	pcRoot;

	pcRoot = mpcMemory->CreateTranslationUnit(NULL);
	mpcTree->SetRoot(pcRoot);
	DeclareBuiltins();

	ParseDeclarations(&pcRoot->macpDeclarations, pcRoot, false);
	pcRoot->SetTokens(0, miNumTokens - 1);
	return miErrors == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPParser::NumErrors(void)
{
	return miErrors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPParser::NumSkimmedErrors(void)
{
	return miSkimmedErrors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::ReportSkimmedErrors(bool bReport)
{
	//Errors in system headers are normally only counted.
	mbReportSkimmed = bReport;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPToken* CCPPParser::Peek(int iAhead)
{
	int		iIndex;

	if (mbHalf)
	{
		if (iAhead == 0)
		{
			return &mcGreaterThan;
		}
	}

	iIndex = miToken + iAhead;
	if (iIndex >= miNumTokens)
	{
		return NULL;
	}
	return mpcTokeniser->Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::Is(ECppTokens eToken)
{
	CCPPToken*	pcToken;

	pcToken = Peek(0);
	return pcToken && pcToken->Is(eToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsAhead(int iAhead, ECppTokens eToken)
{
	CCPPToken*	pcToken;

	pcToken = Peek(iAhead);
	return pcToken && pcToken->Is(eToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsIdentifier(void)
{
	return IsIdentifierAhead(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsIdentifierAhead(int iAhead)
{
	CCPPToken*	pcToken;

	pcToken = Peek(iAhead);
	return pcToken && pcToken->IsIdentifier();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPParser::GetIdentifier(int iAhead)
{
	CCPPToken*	pcToken;

	pcToken = Peek(iAhead);
	if (pcToken && pcToken->IsIdentifier())
	{
		return ((CCTIdentifier*)pcToken)->mlliIdentifier;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsLiteral(void)
{
	CCPPToken*	pcToken;

	pcToken = Peek(0);
	return pcToken && (pcToken->IsNumber() || pcToken->IsChar() || pcToken->IsBool() || pcToken->IsString());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsEnd(void)
{
	return !mbHalf && (miToken >= miNumTokens);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::Next(void)
{
	mbHalf = false;
	if (miToken < miNumTokens)
	{
		miToken++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::Accept(ECppTokens eToken)
{
	if (Is(eToken))
	{
		Next();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::Expect(ECppTokens eToken)
{
	CChars	sz;

	if (Accept(eToken))
	{
		return true;
	}

	sz.Init("Expected '");
	sz.Append(mpcTokeniser->GetReservedWord(eToken));
	sz.Append("'.");
	Error(sz.Text());
	sz.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsCloseAngle(void)
{
	return Is(CT_GreaterThan) || (!mbHalf && Is(CT_RightShift));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::AcceptCloseAngle(void)
{
	//The first '>' of '>>' closes a template argument list; the second is left.  [temp.names] 3
	if (Is(CT_GreaterThan))
	{
		Next();
		return true;
	}
	if (!mbHalf && Is(CT_RightShift))
	{
		mbHalf = true;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPParserPosition CCPPParser::Mark(void)
{
	SCPPParserPosition	sPosition;

	sPosition.iToken = miToken;
	sPosition.bHalf = mbHalf;
	return sPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::Rewind(SCPPParserPosition sPosition)
{
	miToken = sPosition.iToken;
	mbHalf = sPosition.bHalf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPParser::CurrentToken(void)
{
	return miToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPParser::PreviousToken(void)
{
	if (mbHalf)
	{
		return miToken;
	}
	return miToken - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsSkimming(void)
{
	return IsSkimming(miToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::IsSkimming(int iToken)
{
	CCPPToken*			pcToken;
	SPreprocessedToken*	psSource;

	//Code in system headers is skimmed.
	pcToken = mpcTokeniser->Get(iToken);
	if (!pcToken)
	{
		return false;
	}
	psSource = mpcTokeniser->GetSource(pcToken);
	return psSource && psSource->pcFile && psSource->pcFile->IsSystemFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::Error(char* szError)
{
	return ErrorAt(miToken < miNumTokens ? miToken : miNumTokens - 1, szError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::ErrorAt(int iToken, char* szError)
{
	CChars				sz;
	CCPPToken*			pcToken;
	SPreprocessedToken*	psSource;

	if (miTentative > 0)
	{
		mbFailed = true;
		return false;
	}

	sz.Init();
	if (IsSkimming(iToken))
	{
		miSkimmedErrors++;
		if (!mbReportSkimmed)
		{
			sz.Kill();
			return false;
		}
		sz.Append("(skimmed) ");
	}

	pcToken = mpcTokeniser->Get(iToken);
	psSource = pcToken ? mpcTokeniser->GetSource(pcToken) : NULL;
	if (psSource && psSource->pcFile)
	{
		sz.Append(psSource->pcFile->ShortName());
		sz.Append(" (");
		sz.Append(psSource->iLine + 1);
		sz.Append(", ");
		sz.Append(psSource->iColumn + 1);
		sz.Append("): ");
	}
	sz.Append(szError);
	if (pcToken)
	{
		sz.Append("  [");
		pcToken->Append(&sz, mpcTokeniser);
		sz.Append("]");
	}
	mpcTree->AddError(iToken, sz.Text());
	sz.Kill();
	if (!IsSkimming(iToken))
	{
		miErrors++;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::BeginTentative(int* piSavedFailed)
{
	//Nothing parsed tentatively is reported, declared or resolved.
	*piSavedFailed = mbFailed;
	mbFailed = false;
	miTentative++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::EndTentative(int iSavedFailed)
{
	bool	bSucceeded;

	bSucceeded = !mbFailed;
	miTentative--;
	mbFailed = (iSavedFailed != 0);
	return bSucceeded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::Resolve(int iToken, CCPPSymbol* pcSymbol, ECPPTokenRole eRole)
{
	if (miTentative == 0)
	{
		mpcTree->SetTokenSymbol(iToken, pcSymbol, eRole);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPParser::SkipBalanced(void)
{
	int		iDepth;

	//The current token is (, [ or {.  Moves past the token that closes it.
	iDepth = 0;
	while (!IsEnd())
	{
		if (Is(CT_OpenRoundBracket) || Is(CT_OpenSquareBracket) || Is(CT_OpenCurlyBracket))
		{
			iDepth++;
		}
		else if (Is(CT_CloseRoundBracket) || Is(CT_CloseSquareBracket) || Is(CT_CloseCurlyBracket))
		{
			iDepth--;
			if (iDepth <= 0)
			{
				Next();
				return true;
			}
		}
		Next();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::SkipDeclaration(void)
{
	//Recovers from an error by moving past the end of the declaration: a ';' or a body.  A '}' that closes the
	//enclosing scope is left.
	while (!IsEnd())
	{
		if (Is(CT_SemiColon))
		{
			Next();
			return;
		}
		if (Is(CT_CloseCurlyBracket) || Is(CT_CloseRoundBracket) || Is(CT_CloseSquareBracket))
		{
			if (Is(CT_CloseCurlyBracket))
			{
				return;
			}
			Next();
			continue;
		}
		if (Is(CT_OpenCurlyBracket))
		{
			SkipBalanced();
			if (!Is(CT_SemiColon) && !IsIdentifier() && !Is(CT_Comma) && !Is(CT_Multiply))
			{
				return;
			}
			continue;
		}
		if (Is(CT_OpenRoundBracket) || Is(CT_OpenSquareBracket))
		{
			SkipBalanced();
			continue;
		}
		Next();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::SkipStatement(void)
{
	//Recovers from an error in a statement: moves past the ';' or the block.
	while (!IsEnd())
	{
		if (Is(CT_SemiColon))
		{
			Next();
			return;
		}
		if (Is(CT_CloseCurlyBracket))
		{
			return;
		}
		if (Is(CT_OpenCurlyBracket))
		{
			SkipBalanced();
			return;
		}
		if (Is(CT_OpenRoundBracket) || Is(CT_OpenSquareBracket))
		{
			SkipBalanced();
			continue;
		}
		Next();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPParser::PushScope(ECPPScopeKind eKind, CCPPSymbol* pcOwner)
{
	CCPPScope*	pcScope;

	pcScope = mpcSymbols->CreateScope(eKind, mpcScope, pcOwner);
	mpcScope = pcScope;
	return pcScope;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::PopScope(CCPPScope* pcScope)
{
	if (pcScope)
	{
		mpcScope = pcScope->mpcParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPParser::Declare(int64 lliName, int iToken, ECPPSymbolKind eKind, CCPPSyntax* pcDeclaration, CCPPScope* pcScope)
{
	CCPPSymbol*	pcSymbol;

	if ((miTentative > 0) || (lliName == -1) || !pcScope)
	{
		return NULL;
	}

	pcSymbol = mpcSymbols->CreateSymbol(lliName, eKind, pcDeclaration, pcScope);
	pcScope->Add(pcSymbol);
	if (iToken != -1)
	{
		Resolve(iToken, pcSymbol, CPPTR_Declaration);
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPParser::DeclareBuiltins(void)
{
	CCPPScope*	pcGlobal;

	//g++ and clang types with identifier names, for the system headers.
	pcGlobal = mpcSymbols->GetGlobalScope();
	Declare(mpcTokeniser->GetIdentifierID("__builtin_va_list"), -1, CPPSK_BuiltinType, NULL, pcGlobal);
	Declare(mpcTokeniser->GetIdentifierID("__int128"), -1, CPPSK_BuiltinType, NULL, pcGlobal);
	Declare(mpcTokeniser->GetIdentifierID("__float128"), -1, CPPSK_BuiltinType, NULL, pcGlobal);

	//The names of the enclosing function.  [dcl.fct.def.general] 8
	Declare(mpcTokeniser->GetIdentifierID("__func__"), -1, CPPSK_Variable, NULL, pcGlobal);
	Declare(mlliFunction, -1, CPPSK_Variable, NULL, pcGlobal);
	Declare(mlliFuncsig, -1, CPPSK_Variable, NULL, pcGlobal);
	Declare(mlliFuncdname, -1, CPPSK_Variable, NULL, pcGlobal);
	Declare(mpcTokeniser->GetIdentifierID("__PRETTY_FUNCTION__"), -1, CPPSK_Variable, NULL, pcGlobal);
}

