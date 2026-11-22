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
	mauiSilent.Init();
	masKeeping.Init();
	mauiQuietKept.Init();
	mapcKept.Init();
	mcDeferred.Init();
	SetStatistics(pcStatistics);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::Kill(void)
{
	size	i;

	for (i = 0; i < mapcKept.NumElements(); i++)
	{
		mapcKept.GetPtr(i)->Kill();
		free(mapcKept.GetPtr(i));
	}
	mapcKept.Kill();
	mcDeferred.Kill();
	mauiQuietKept.Kill();
	masKeeping.Kill();
	mauiSilent.Kill();
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
	size	uiError;

	if (mpcContext->IsQuiet())
	{
		mpcContext->SetFailed();
		if (!mpcContext->IsKeeping())
		{
			return false;
		}
		uiError = mcErrors.NumErrors();
		mauiQuietKept.Add(&uiError);
	}
	return ErrorAt(pcNode ? pcNode->miFirstToken : -1, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// Error, unless the same error is said at the place already: what each of
// the two walkers asks of the same call is said once.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::ErrorOnce(CCPPSyntax* pcNode, const char* szMessage)
{
	if (!mpcContext->IsQuiet() && pcNode && mcErrors.HasAt(pcNode->miFirstToken, szMessage))
	{
		return false;
	}
	return Error(pcNode, szMessage);
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
	return AddError(&mcErrors, iToken, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// An error at a token of the unit, added to pcDest.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::AddError(CCPPErrors* pcDest, int iToken, const char* szMessage)
{
	SPreprocessedToken*	psSource;
	CChars				szDetail;
	CChars				szMessageText;

	if ((iToken < 0) || ((size)iToken >= mpcTokeniser->NumTokens()))
	{
		pcDest->Add(NULL, -1, -1, -1, szMessage);
		return false;
	}

	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(iToken));
	szMessageText.Init(szMessage);
	AppendMacroUse(psSource, &szMessageText);
	szDetail.Init();
	psSource->Append(&szDetail);
	mpcTokeniser->mpcSource->AddError(pcDest, psSource, iToken, szMessageText.Text(), szDetail.Text());
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
	if (pcNode && mcErrors.HasAt(pcNode->miFirstToken, szMessage))
	{
		return false;
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
// Quiet code of which nothing is kept: what it does is judged by what it
// returns, and whatever it says, loudly too, goes when it ends.  For asking
// whether something can be done, by trying it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::BeginSilent(void)
{
	size	uiErrors;

	uiErrors = mcErrors.NumErrors();
	mauiSilent.Add(&uiErrors);
	BeginQuiet();
}


//////////////////////////////////////////////////////////////////////////
//
// As EndQuiet: false if an error happened in it.  Its errors are taken
// away.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::EndSilent(void)
{
	bool	bPassed;

	bPassed = EndQuiet();
	mcErrors.Truncate(*mauiSilent.Tail());
	mauiSilent.RemoveTail();
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
// A trial of which nothing is kept.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::BeginSilentTrial(void)
{
	size	uiErrors;

	uiErrors = mcErrors.NumErrors();
	mauiSilent.Add(&uiErrors);
	BeginTrial();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::EndSilentTrial(void)
{
	bool	bPassed;

	bPassed = EndTrial();
	mcErrors.Truncate(*mauiSilent.Tail());
	mauiSilent.RemoveTail();
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
// What is worked out until EndKeeping is remembered by the caller, with the
// errors made working it out: they are recorded, in quiet code too, and
// given to the caller when it ends.  Without bLoudToo what is said loudly
// stays where it is, said: only what quiet code would not have said is
// kept.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::BeginKeeping(bool bLoudToo)
{
	SCPPKeeping*	psKeeping;

	psKeeping = masKeeping.Add();
	psKeeping->uiFirst = mcErrors.NumErrors();
	psKeeping->uiFirstQuiet = mauiQuietKept.NumElements();
	psKeeping->bLoudToo = bLoudToo;
	mpcContext->EnterKeeping();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an error is on the list at pcNode's first token: what is wrong
// there has been said, and what only follows from it need not be.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::HasErrorAt(CCPPSyntax* pcNode)
{
	size	i;

	for (i = mcErrors.NumErrors(); pcNode && (i > 0); i--)
	{
		if (mcErrors.GetToken(i - 1) == pcNode->miFirstToken)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether anything has been said since the last BeginKeeping.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::HasKept(void)
{
	return mcErrors.NumErrors() > masKeeping.Tail()->uiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// The errors made since BeginKeeping, taken off the list: NULL if there
// were none.  They are the diagnostics' to free, and the caller's to have
// said (Report) for as long as the unit lasts.
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPDiagnostics::EndKeeping(void)
{
	CCPPErrors*		pcKept;
	SCPPKeeping		sKeeping;
	size			i;
	size			uiError;

	mpcContext->LeaveKeeping();
	sKeeping = *masKeeping.Tail();
	masKeeping.RemoveTail();
	if ((mcErrors.NumErrors() <= sKeeping.uiFirst) || (!sKeeping.bLoudToo && (mauiQuietKept.NumElements() == sKeeping.uiFirstQuiet)))
	{
		mauiQuietKept.SetUsedElements(sKeeping.uiFirstQuiet);
		return NULL;
	}
	pcKept = (CCPPErrors*)malloc(sizeof(CCPPErrors));
	pcKept->Init();
	if (sKeeping.bLoudToo)
	{
		mcErrors.MoveTailTo(pcKept, sKeeping.uiFirst);
	}
	else
	{
		//Only the quiet ones: each is taken off the list, the last first, and they are kept in the order they were said.
		for (i = mauiQuietKept.NumElements(); i > sKeeping.uiFirstQuiet; i--)
		{
			uiError = *mauiQuietKept.Get(i - 1);
			if (uiError < mcErrors.NumErrors())
			{
				mcErrors.MoveTo(pcKept, uiError);
			}
		}
		pcKept->Reverse();
	}
	mauiQuietKept.SetUsedElements(sKeeping.uiFirstQuiet);
	mapcKept.Add(pcKept);
	return pcKept;
}


//////////////////////////////////////////////////////////////////////////
//
// Says errors that were kept as if they were said again here: in quiet
// code they only make it fail (as Error), unless what is around is being
// kept.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::Repeat(CCPPErrors* pcKept)
{
	if (!pcKept)
	{
		return;
	}
	if (mpcContext->IsQuiet())
	{
		mpcContext->SetFailed();
		if (!mpcContext->IsKeeping())
		{
			return;
		}
	}
	Report(pcKept);
}


//////////////////////////////////////////////////////////////////////////
//
// Says errors that were kept, in quiet code too (as ReportError): each
// unless the list has it already, so what is asked about again is said
// once.  NULL is no errors.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::Report(CCPPErrors* pcKept)
{
	CCPPErrors	cNew;
	size		i;

	if (!pcKept)
	{
		return;
	}
	cNew.Init();
	pcKept->AppendTo(&cNew);
	for (i = cNew.NumErrors(); i > 0; i--)
	{
		if (mcErrors.Has(cNew.Get(i - 1)))
		{
			cNew.Remove(i - 1);
		}
	}
	cNew.AppendTo(&mcErrors);
	cNew.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An error of what is worked out once where errors are often dropped (a
// class laid out quietly, or first by the IR front end): kept, and said
// when the unit ends (ReportDeferred), or sooner as the reason something
// failed.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::Defer(int iToken, const char* szMessage)
{
	return AddError(&mcDeferred, iToken, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// The errors deferred are said, and kept no longer.
//
//////////////////////////////////////////////////////////////////////////
void CCPPDiagnostics::ReportDeferred(void)
{
	mcDeferred.AppendTo(&mcErrors);
	mcDeferred.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an error is deferred and not yet said: what fails for want of
// what it is of has its reason there.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPDiagnostics::HasDeferred(void)
{
	return mcDeferred.NumErrors() > 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPDiagnostics::GetErrors(void) { return &mcErrors; }
CCPPErrors* CCPPDiagnostics::GetWarnings(void) { return &mcWarnings; }
size CCPPDiagnostics::NumErrors(void) const { return mcErrors.NumErrors(); }
