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
#include "SourceFile.h"
#include "PreprocessedTokens.h"
#include "CppTokeniser.h"
#include "CPPSyntax.h"
#include "CPPContext.h"
#include "CPPCompileStatistics.h"
#include "CPPDiagnostics.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::Init(CCPPTokeniser* pcTokeniser, CCPPContext* pcContext, CCPPCompileStatistics* pcStatistics)
{
	mpcTokeniser = pcTokeniser;
	mpcContext = pcContext;
	mcErrors.Init();
	mcWarnings.Init();
	SetStatistics(pcStatistics);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::Kill(void)
{
	mcWarnings.Kill();
	mcErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What quiet trials, and errors taken away again, are counted in from now
// on.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::SetStatistics(CCPPCompileStatistics* pcStatistics)
{
	mpcStatistics = pcStatistics;
	mcErrors.CountTakenAway(pcStatistics);
}


//////////////////////////////////////////////////////////////////////////
//
// Errors are "File (line, column): message [token]", like the other stages'.
// In quiet mode (declarations, which only fail when what they declare is used) they
// are only noted.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::Error(CCPPSyntax* pcNode, const char* szMessage)
{
	if (mpcContext->IsQuiet())
	{
		mpcContext->SetFailed();
		return false;
	}
	return ErrorAt(pcNode ? pcNode->miFirstToken : -1, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// A warning at pcNode's first token, unless that token already has one (the
// same expression is often looked at more than once).
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::Warning(CCPPSyntax* pcNode, const char* szMessage)
{
	SPreprocessedToken*	psSource;
	CChars				szMessageText;
	CChars				szDetail;
	int					iToken;
	size				i;

	iToken = pcNode ? pcNode->miFirstToken : -1;
	if ((iToken < 0) || ((size)iToken >= mpcTokeniser->NumTokens()))
	{
		return;
	}
	for (i = 0; i < mcWarnings.NumErrors(); i++)
	{
		if (mcWarnings.GetToken(i) == iToken)
		{
			return;
		}
	}

	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(iToken));
	szMessageText.Init("Warning: ");
	szMessageText.Append(szMessage);
	AppendMacroUse(psSource, &szMessageText);
	szDetail.Init();
	psSource->Append(&szDetail);
	mpcTokeniser->mpcSource->AddError(&mcWarnings, psSource, iToken, szMessageText.Text(), szDetail.Text());
	szDetail.Kill();
	szMessageText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a warning containing szText is at one of pcNode's tokens.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::HasWarning(CCPPSyntax* pcNode, const char* szText)
{
	size	i;
	int		iToken;

	for (i = 0; i < mcWarnings.NumErrors(); i++)
	{
		iToken = mcWarnings.GetToken(i);
		if ((iToken >= pcNode->miFirstToken) && (iToken <= pcNode->miLastToken) && strstr(mcWarnings.Get(i), szText))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::ErrorAt(int iToken, const char* szMessage)
{
	SPreprocessedToken*	psSource;
	CChars				szDetail;
	CChars				szMessageText;

	if ((iToken < 0) || ((size)iToken >= mpcTokeniser->NumTokens()))
	{
		mcErrors.Add(NULL, -1, -1, -1, szMessage);
		return false;
	}

	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(iToken));
	szMessageText.Init(szMessage);
	AppendMacroUse(psSource, &szMessageText);
	szDetail.Init();
	psSource->Append(&szDetail);
	mpcTokeniser->mpcSource->AddError(&mcErrors, psSource, iToken, szMessageText.Text(), szDetail.Text());
	szDetail.Kill();
	szMessageText.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A token made by a macro is reported where its characters are (often in a header's #define), so where the outermost
// macro was used is added: " (in AssertInt at TestImage.cpp (12, 2))".
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::AppendMacroUse(SPreprocessedToken* psSource, CChars* pszMessage)
{
	SMacroExpansion*		psExpansion;
	CPreprocessedTokens*	pcSource;
	const char*				szFile;
	int						iLine;

	if (!psSource || (psSource->iExpansion < 0))
	{
		return;
	}
	pcSource = mpcTokeniser->mpcSource;
	psExpansion = pcSource->GetExpansion((size)psSource->iExpansion);
	while (psExpansion->iParent >= 0)
	{
		psExpansion = pcSource->GetExpansion((size)psExpansion->iParent);
	}
	if (!psExpansion->pcFile)
	{
		return;
	}
	pszMessage->Append(" (in ");
	pszMessage->Append(psExpansion->szName, (size)psExpansion->uiNameLength);
	pszMessage->Append(" at ");
	szFile = psExpansion->pcFile->ShortName();
	iLine = psExpansion->iLine;
	pcSource->ReportedPosition(psExpansion->pcFile, &szFile, &iLine);
	pszMessage->Append(szFile);
	pszMessage->Append(" (");
	pszMessage->Append(iLine + 1);
	pszMessage->Append(", ");
	pszMessage->Append(psExpansion->iColumn + 1);
	pszMessage->Append("))");
}


//////////////////////////////////////////////////////////////////////////
//
// An error in the C++ the IR front end found (break outside a loop), reported
// in quiet mode too.  Returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::ReportError(CCPPSyntax* pcNode, const char* szMessage)
{
	BeginLoud();
	Error(pcNode, szMessage);
	EndLoud();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// An error reported (as ReportError) unless the same one is already there
// for the place: for what is worked out once and asked about again, whose
// error must be there however often it is asked, and however often what
// was asked only quietly took its errors away.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::ReportOnce(CCPPSyntax* pcNode, const char* szMessage)
{
	size	i;

	for (i = 0; pcNode && (i < mcErrors.NumErrors()); i++)
	{
		if ((mcErrors.GetToken(i) == pcNode->miFirstToken) && strstr(mcErrors.Get(i), szMessage))
		{
			return false;
		}
	}
	return ReportError(pcNode, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// ReportError at a token.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::ReportErrorAt(int iToken, const char* szMessage)
{
	BeginLoud();
	ErrorAt(iToken, szMessage);
	EndLoud();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Declarations of things the compiler cannot do yet are only errors when they
// are used.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::BeginQuiet(void)
{
	if (mpcContext->BeginQuiet())
	{
		mpcStatistics->Count(CPPCC_QuietTrials);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if an error happened since BeginQuiet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::EndQuiet(void)
{
	bool	bPassed;

	bPassed = mpcContext->EndQuiet();
	if (!bPassed && !mpcContext->IsQuiet())
	{
		mpcStatistics->Count(CPPCC_QuietTrialsFailed);
	}
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
// Something is tried that may fail, and whether it does is its own: EndTrial says
// whether it made an error, and the quiet code around it (if any) hasn't failed
// because it did.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::BeginTrial(void)
{
	if (mpcContext->BeginTrial())
	{
		mpcStatistics->Count(CPPCC_QuietTrials);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::EndTrial(void)
{
	bool	bPassed;

	bPassed = mpcContext->EndTrial();
	if (!bPassed && !mpcContext->IsQuiet())
	{
		mpcStatistics->Count(CPPCC_QuietTrialsFailed);
	}
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
// What is an error where it is, in quiet mode too, is reported: until EndLoud
// nothing is quiet, and after it what was quiet is, having failed or not as it
// had.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::BeginLoud(void)
{
	mpcContext->BeginLoud();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::EndLoud(void)
{
	mpcContext->EndLoud();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPDiagnostics::GetErrors(void) { return &mcErrors; }
CCPPErrors* CCPPDiagnostics::GetWarnings(void) { return &mcWarnings; }
size CCPPDiagnostics::NumErrors(void) const { return mcErrors.NumErrors(); }
