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
#include "Preprocessor.h"


//The most headers kept for one file and context that are compared with the macros (a header included with ever
//different macros is mostly preprocessed).
#define PP_HEADER_CACHE_CANDIDATES	16


//////////////////////////////////////////////////////////////////////////
//
// Headers' output is kept in pcHeaderCache (shared with other units'
// preprocessors), and a header included with the macros it was kept with
// is replayed from it, not preprocessed.  The output is the same.  NULL
// preprocesses every header.
//
// A header is kept when its output is a run of whole tokens: it starts and
// ends outside a macro invocation (the arguments of a function-like macro
// don't run past its end), it makes no errors or warnings, and it doesn't
// use __COUNTER__, __DATE__, __TIME__ or __TIMESTAMP__.  What it depends on
// is what the macros it looked up (before it defined them) were, which of
// the #pragma once files it included had been, the include directories and
// the directories of the files including it (a quoted #include looks in
// them).  What it did besides its output (#define, #undef and #pragma
// once) is done again when it is replayed.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::SetHeaderCache(CPPHeaderCache* pcHeaderCache)
{
	mpcHeaderCache = pcHeaderCache;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a header included now can be replayed or kept: at a token
// boundary (not reading ahead past a macro name, nor in a macro's
// expansion).
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::CanUseHeaderCache(void)
{
	return mpcHeaderCache && !mbLogIncludes && !mbReplaying && !mbError && !mbInCondition && (miLookAhead == 0) &&
		(macPushedBack.NumElements() == 0) && (maiDisabledDefines.NumElements() == 0) && (muiPendingFlags == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// What finding a header's own #includes depends on, besides the macros: the
// include directories and the directories of the files including it.
//
//////////////////////////////////////////////////////////////////////////
uint64 CPreprocessor::HeaderContext(void)
{
	size		i;
	uint64		uiHash;
	SPPSource*	psSource;

	if (muiIncludeDirectories == 0)
	{
		uiHash = 0xCBF29CE484222325ULL;
		for (i = 0; i < mcHeaderNames.NumElements(); i++)
		{
			uiHash = (uiHash ^ (uint64)(size)*mcHeaderNames.Get(i)) * 0x100000001B3ULL;
		}
		muiIncludeDirectories = (uiHash == 0) ? 1 : uiHash;
	}
	psSource = GetCurrentFileSource();
	return (muiIncludeDirectories ^ (psSource ? psSource->uiDirectories : 0)) * 0x100000001B3ULL;
}


//////////////////////////////////////////////////////////////////////////
//
// pcFile's directory hashed after uiIncluding (the directories of the files
// including it).
//
//////////////////////////////////////////////////////////////////////////
uint64 CPreprocessor::DirectoriesHash(CSourceFile* pcFile, uint64 uiIncluding)
{
	CChars	szPath;
	uint64	uiHash;

	pcFile->Path(&szPath);
	szPath.Replace('\\', '/');
	uiHash = CNameSet::Hash(szPath.Text(), szPath.Length());
	szPath.Kill();
	return (uiIncluding ^ uiHash) * 0x100000001B3ULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Replays pcFile (found in pcHeaderNameMap) if another unit kept it with
// the macros it has now.  False if none did.
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ReplayCachedHeader(CHeaderFile* pcFile, CHeaderFiles* pcHeaderNameMap)
{
	uint64				uiContext;
	SPPCachedHeader*	psHeader;
	size				uiCandidates;
	SPPRecorder*		psRecorder;
	size				uiToken;
	size				uiExpansion;
	size				uiInactive;
	size				uiDirective;

	uiContext = HeaderContext();
	mpcHeaderCache->Lock();
	psHeader = mpcHeaderCache->Get(pcFile, pcHeaderNameMap, uiContext);
	mpcHeaderCache->Unlock();

	//Published headers don't change, so the list is followed unlocked.
	uiCandidates = 0;
	for (; psHeader && (uiCandidates < PP_HEADER_CACHE_CANDIDATES); psHeader = psHeader->psNext)
	{
		if ((psHeader->pcFile != pcFile) || (psHeader->pcHeaderNameMap != pcHeaderNameMap) || (psHeader->uiContext != uiContext))
		{
			continue;
		}
		uiCandidates++;
		if (CachedHeaderMatches(psHeader))
		{
			break;
		}
	}
	if (!psHeader || (uiCandidates == PP_HEADER_CACHE_CANDIDATES))
	{
		return false;
	}

	uiToken = mpcOutput->NumTokens();
	uiExpansion = mpcOutput->NumExpansions();
	uiInactive = mpcOutput->NumInactiveRegions();
	uiDirective = mpcOutput->NumDirectiveLines();

	mbReplaying = true;
	ReplayHeader(psHeader);
	mbReplaying = false;
	muiPendingFlags = psHeader->uiEndPendingFlags;
	muiHeaderHits++;
	muiReplayedTokens += psHeader->uiTotalTokens;

	//A header being kept includes it as a header kept on its own.
	psRecorder = masRecorders.Tail();
	if (psRecorder && psRecorder->bValid)
	{
		AddChildSegment(psRecorder, psHeader, uiToken, uiExpansion, uiInactive, uiDirective);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the macros and #pragma once files psHeader depends on are as they
// were when it was kept.
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::CachedHeaderMatches(SPPCachedHeader* psHeader)
{
	size					i;
	SPPHeaderQuery*			psQuery;
	SPPHeaderPragmaQuery*	psPragma;
	CExternalString			cName;
	SPPDefineIdentity		sIdentity;

	for (i = 0; i < psHeader->asQueries.NumElements(); i++)
	{
		psQuery = psHeader->asQueries.Get(i);
		cName.Init((char*)psQuery->szName, (int)psQuery->uiLength);
		DefineIdentity(mcDefines.Get(&cName), &sIdentity);
		if (!sIdentity.Equals(&psQuery->sIdentity))
		{
			return false;
		}
	}
	for (i = 0; i < psHeader->asPragmaQueries.NumElements(); i++)
	{
		psPragma = psHeader->asPragmaQueries.Get(i);
		if ((mcPragmaOnce.Get(psPragma->pcFile) != NULL) != psPragma->bOnce)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// psHeader's output is added to the unit's, and its #defines, #undefs and
// #pragma onces done.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ReplayHeader(SPPCachedHeader* psHeader)
{
	size				uiSegment;
	SPPHeaderSegment*	psSegment;
	size				ui;
	int					iFirstExpansion;
	SMacroExpansion*	psExpansion;
	SPreprocessedToken	sToken;
	SInactiveRegion*	psRegion;
	SDirectiveLine*		psLine;

	for (uiSegment = 0; uiSegment < psHeader->asSegments.NumElements(); uiSegment++)
	{
		psSegment = psHeader->asSegments.Get(uiSegment);
		if (psSegment->psChild)
		{
			ReplayHeader(psSegment->psChild);
			continue;
		}

		iFirstExpansion = (int)mpcOutput->NumExpansions();
		for (ui = 0; ui < psSegment->uiNumExpansions; ui++)
		{
			psExpansion = psHeader->asExpansions.Get(psSegment->uiFirstExpansion + ui);
			mpcOutput->AddExpansion(psExpansion->szName, psExpansion->uiNameLength, psExpansion->pcFile, psExpansion->iLine, psExpansion->iColumn, (psExpansion->iParent < 0) ? -1 : psExpansion->iParent + iFirstExpansion);
		}
		for (ui = 0; ui < psSegment->uiNumTokens; ui++)
		{
			sToken = *psHeader->asTokens.Get(psSegment->uiFirstToken + ui);
			if (sToken.iExpansion >= 0)
			{
				sToken.iExpansion += iFirstExpansion;
			}
			mpcOutput->Add(&sToken);
		}
		for (ui = 0; ui < psSegment->uiNumInactive; ui++)
		{
			psRegion = psHeader->asInactiveRegions.Get(psSegment->uiFirstInactive + ui);
			mpcOutput->AddInactiveLines(psRegion->pcFile, psRegion->iStartLine, psRegion->iEndLine);
		}
		for (ui = 0; ui < psSegment->uiNumDirectives; ui++)
		{
			psLine = psHeader->asDirectiveLines.Get(psSegment->uiFirstDirective + ui);
			mpcOutput->AddDirectiveLine(psLine->pcFile, psLine->iLine, psLine->bActive);
		}
		for (ui = 0; ui < psSegment->uiNumEvents; ui++)
		{
			ReplayEvent(psHeader->asEvents.Get(psSegment->uiFirstEvent + ui));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A #define or #undef is done again (from its directive, as it was in its
// file), a #pragma once is.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ReplayEvent(SPPHeaderEvent* psEvent)
{
	CPreprocessorTokenParser	cParser;
	CSourceFile*				pcCurrentFile;

	if (psEvent->eType == PPHE_PragmaOnce)
	{
		mcPragmaOnce.Put(psEvent->pcFile, psEvent->pcFile);
		return;
	}

	pcCurrentFile = mpcCurrentFile;
	mpcCurrentFile = psEvent->pcFile;
	cParser.Init(psEvent->pcDirective);
	if (psEvent->eType == PPHE_Define)
	{
		ProcessHashDefine(psEvent->pcDirective, &cParser);
	}
	else
	{
		ProcessHashUndef(psEvent->pcDirective, &cParser);
	}
	cParser.Kill();
	mpcCurrentFile = pcCurrentFile;
}


//////////////////////////////////////////////////////////////////////////
//
// pcFile (found in pcHeaderNameMap, just pushed) is kept as it is
// preprocessed, until it ends (EndRecording).
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::BeginRecording(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext)
{
	SPPRecorder*	psRecorder;

	macSources.Tail()->bRecorded = true;
	psRecorder = masRecorders.Push();
	psRecorder->psHeader = mpcHeaderCache->New(pcFile, pcHeaderNameMap, uiContext);
	psRecorder->cSeen.Init();
	psRecorder->apcPragmaSeen.Init();
	psRecorder->uiStartToken = mpcOutput->NumTokens();
	psRecorder->uiStartExpansion = mpcOutput->NumExpansions();
	psRecorder->uiStartInactive = mpcOutput->NumInactiveRegions();
	psRecorder->uiStartDirective = mpcOutput->NumDirectiveLines();
	psRecorder->uiErrors = mcErrors.NumErrors();
	psRecorder->uiWarnings = mcWarnings.NumErrors();
	psRecorder->bValid = true;
	StartSegment(psRecorder);
}


//////////////////////////////////////////////////////////////////////////
//
// The header being kept has ended (its file is being popped): it is
// published if it can be kept, and is part of the header including it.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::EndRecording(void)
{
	SPPRecorder*		psRecorder;
	SPPCachedHeader*	psHeader;
	size				uiToken;
	size				uiExpansion;
	size				uiInactive;
	size				uiDirective;

	psRecorder = masRecorders.Tail();
	if ((miLookAhead != 0) || (macPushedBack.NumElements() != 0) || (mcErrors.NumErrors() != psRecorder->uiErrors) || (mcWarnings.NumErrors() != psRecorder->uiWarnings))
	{
		InvalidateRecordings();
	}
	EndSegment(psRecorder, mpcOutput->NumTokens(), mpcOutput->NumExpansions(), mpcOutput->NumInactiveRegions(), mpcOutput->NumDirectiveLines());

	psHeader = NULL;
	if (psRecorder->bValid)
	{
		psHeader = psRecorder->psHeader;
		psHeader->uiEndPendingFlags = muiPendingFlags;
		psHeader->uiTotalTokens = mpcOutput->NumTokens() - psRecorder->uiStartToken;
		mpcHeaderCache->Publish(psHeader);
		muiHeaderMisses++;
	}
	else
	{
		//The headers including it can't be kept without it.
		InvalidateRecordings();
	}
	uiToken = psRecorder->uiStartToken;
	uiExpansion = psRecorder->uiStartExpansion;
	uiInactive = psRecorder->uiStartInactive;
	uiDirective = psRecorder->uiStartDirective;
	KillRecorder(psRecorder, psHeader != NULL);
	masRecorders.Pop();

	psRecorder = masRecorders.Tail();
	if (psHeader && psRecorder && psRecorder->bValid)
	{
		AddChildSegment(psRecorder, psHeader, uiToken, uiExpansion, uiInactive, uiDirective);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A run of the header's own output starts.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::StartSegment(SPPRecorder* psRecorder)
{
	SInactiveRegion*	psTail;

	psRecorder->uiSegmentToken = mpcOutput->NumTokens();
	psRecorder->uiSegmentExpansion = mpcOutput->NumExpansions();
	psRecorder->uiSegmentInactive = mpcOutput->NumInactiveRegions();
	psRecorder->uiSegmentDirective = mpcOutput->NumDirectiveLines();
	psRecorder->uiSegmentEvent = psRecorder->psHeader->asEvents.NumElements();
	psTail = (psRecorder->uiSegmentInactive > 0) ? mpcOutput->GetInactiveRegion(psRecorder->uiSegmentInactive - 1) : NULL;
	psRecorder->iTailEndLine = psTail ? psTail->iEndLine : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The run of the header's own output ends where the unit's output has
// uiToken tokens (and so on): it is copied into the header.  The tokens'
// text that isn't in a header file is copied too (a unit's file is
// unloaded, the unit's output freed), and macro expansions are numbered
// from the run's first.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::EndSegment(SPPRecorder* psRecorder, size uiToken, size uiExpansion, size uiInactive, size uiDirective)
{
	SPPCachedHeader*	psHeader;
	SPPHeaderSegment	sSegment;
	size				ui;
	SPreprocessedToken	sToken;
	SMacroExpansion		sExpansion;
	SInactiveRegion*	psTail;
	int					iFirst;
	int					iEnd;
	size				uiEvents;

	if (!psRecorder->bValid)
	{
		return;
	}

	//Skipped lines just before the run that its own joined (the same file) would be replayed differently.
	psTail = (psRecorder->uiSegmentInactive > 0) ? mpcOutput->GetInactiveRegion(psRecorder->uiSegmentInactive - 1) : NULL;
	if (psTail && (psTail->iEndLine != psRecorder->iTailEndLine))
	{
		InvalidateRecordings();
		return;
	}

	psHeader = psRecorder->psHeader;
	uiEvents = psHeader->asEvents.NumElements() - psRecorder->uiSegmentEvent;
	if ((uiToken == psRecorder->uiSegmentToken) && (uiExpansion == psRecorder->uiSegmentExpansion) && (uiInactive == psRecorder->uiSegmentInactive) &&
		(uiDirective == psRecorder->uiSegmentDirective) && (uiEvents == 0))
	{
		return;
	}

	iFirst = (int)psRecorder->uiSegmentExpansion;
	iEnd = (int)uiExpansion;
	memset(&sSegment, 0, sizeof(SPPHeaderSegment));
	sSegment.psChild = NULL;

	sSegment.uiFirstExpansion = (uint32)psHeader->asExpansions.NumElements();
	sSegment.uiNumExpansions = (uint32)(uiExpansion - psRecorder->uiSegmentExpansion);
	for (ui = psRecorder->uiSegmentExpansion; ui < uiExpansion; ui++)
	{
		sExpansion = *mpcOutput->GetExpansion(ui);
		if ((sExpansion.iParent >= 0) && ((sExpansion.iParent < iFirst) || (sExpansion.iParent >= iEnd)))
		{
			InvalidateRecordings();
			return;
		}
		sExpansion.szName = psHeader->AddString(sExpansion.szName, sExpansion.uiNameLength);
		if (sExpansion.iParent >= 0)
		{
			sExpansion.iParent -= iFirst;
		}
		psHeader->asExpansions.Add(&sExpansion);
	}

	sSegment.uiFirstToken = (uint32)psHeader->asTokens.NumElements();
	sSegment.uiNumTokens = (uint32)(uiToken - psRecorder->uiSegmentToken);
	for (ui = psRecorder->uiSegmentToken; ui < uiToken; ui++)
	{
		sToken = *mpcOutput->Get(ui);
		if (sToken.iExpansion >= 0)
		{
			if ((sToken.iExpansion < iFirst) || (sToken.iExpansion >= iEnd))
			{
				InvalidateRecordings();
				return;
			}
			sToken.iExpansion -= iFirst;
		}
		if ((sToken.uiFlags & PTF_OUTPUT_TEXT) || !sToken.pcFile || !sToken.pcFile->IsHeader())
		{
			sToken.szText = psHeader->AddString(sToken.szText, sToken.uiLength);
			sToken.uiFlags |= PTF_OUTPUT_TEXT;
		}
		psHeader->asTokens.Add(&sToken);
	}

	sSegment.uiFirstInactive = (uint32)psHeader->asInactiveRegions.NumElements();
	sSegment.uiNumInactive = (uint32)(uiInactive - psRecorder->uiSegmentInactive);
	for (ui = psRecorder->uiSegmentInactive; ui < uiInactive; ui++)
	{
		psHeader->asInactiveRegions.Add(mpcOutput->GetInactiveRegion(ui));
	}

	sSegment.uiFirstDirective = (uint32)psHeader->asDirectiveLines.NumElements();
	sSegment.uiNumDirectives = (uint32)(uiDirective - psRecorder->uiSegmentDirective);
	for (ui = psRecorder->uiSegmentDirective; ui < uiDirective; ui++)
	{
		psHeader->asDirectiveLines.Add(mpcOutput->GetDirectiveLine(ui));
	}

	sSegment.uiFirstEvent = (uint32)psRecorder->uiSegmentEvent;
	sSegment.uiNumEvents = (uint32)uiEvents;
	psHeader->asSegments.Add(&sSegment);
}


//////////////////////////////////////////////////////////////////////////
//
// The header being kept included psChild (kept, or replayed), which
// started where the unit's output had uiToken tokens (and so on): its own
// run ends there, psChild is a segment, and what psChild depends on (that
// the header didn't define first) the header does.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddChildSegment(SPPRecorder* psRecorder, SPPCachedHeader* psChild, size uiToken, size uiExpansion, size uiInactive, size uiDirective)
{
	SPPCachedHeader*		psHeader;
	SPPHeaderSegment		sSegment;
	size					ui;
	SPPHeaderQuery*			psQuery;
	SPPHeaderPragmaQuery*	psPragma;
	SPPHeaderName*			psName;
	CSourceFile*			pcFile;

	EndSegment(psRecorder, uiToken, uiExpansion, uiInactive, uiDirective);
	if (!psRecorder->bValid)
	{
		return;
	}

	psHeader = psRecorder->psHeader;
	memset(&sSegment, 0, sizeof(SPPHeaderSegment));
	sSegment.psChild = psChild;
	psHeader->asSegments.Add(&sSegment);

	//The child's names and files outlive the header (both are freed with the cache).
	for (ui = 0; ui < psChild->asQueries.NumElements(); ui++)
	{
		psQuery = psChild->asQueries.Get(ui);
		if (psRecorder->cSeen.Add(psQuery->szName, psQuery->uiLength))
		{
			psHeader->asQueries.Add(psQuery);
		}
	}
	for (ui = 0; ui < psChild->asPragmaQueries.NumElements(); ui++)
	{
		psPragma = psChild->asPragmaQueries.Get(ui);
		if (!psRecorder->apcPragmaSeen.Contains(&psPragma->pcFile))
		{
			psRecorder->apcPragmaSeen.Add(&psPragma->pcFile);
			psHeader->asPragmaQueries.Add(psPragma);
		}
	}
	for (ui = 0; ui < psChild->asDefined.NumElements(); ui++)
	{
		psName = psChild->asDefined.Get(ui);
		psRecorder->cSeen.Add(psName->szName, psName->uiLength);
		psHeader->asDefined.Add(psName);
	}
	for (ui = 0; ui < psChild->apcPragmaOnced.NumElements(); ui++)
	{
		pcFile = *psChild->apcPragmaOnced.Get(ui);
		if (!psRecorder->apcPragmaSeen.Contains(&pcFile))
		{
			psRecorder->apcPragmaSeen.Add(&pcFile);
		}
		psHeader->apcPragmaOnced.Add(&pcFile);
	}

	StartSegment(psRecorder);
}


//////////////////////////////////////////////////////////////////////////
//
// psRecorder is finished with; its header is freed unless bKeep (it was
// published).
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::KillRecorder(SPPRecorder* psRecorder, bool bKeep)
{
	psRecorder->apcPragmaSeen.Kill();
	psRecorder->cSeen.Kill();
	if (!bKeep)
	{
		mpcHeaderCache->Discard(psRecorder->psHeader);
	}
	psRecorder->psHeader = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// None of the headers being kept can be.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::InvalidateRecordings(void)
{
	size	i;

	for (i = 0; i < masRecorders.NumElements(); i++)
	{
		masRecorders.Get(i)->bValid = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The innermost header being kept, NULL if there is none (or it can't be,
// or a kept header is being replayed).
//
//////////////////////////////////////////////////////////////////////////
SPPRecorder* CPreprocessor::Recording(void)
{
	SPPRecorder*	psRecorder;

	if (mbReplaying)
	{
		return NULL;
	}
	psRecorder = masRecorders.Tail();
	return (psRecorder && psRecorder->bValid) ? psRecorder : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The macro szName was looked up and was pcDefine (or none).
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::NoteLookUp(const char* szName, size uiLength, CDefine* pcDefine)
{
	SPPRecorder*	psRecorder;
	SPPHeaderQuery	sQuery;

	psRecorder = Recording();
	if (!psRecorder || psRecorder->cSeen.Contains(szName, uiLength))
	{
		return;
	}
	sQuery.szName = psRecorder->psHeader->AddString(szName, uiLength);
	sQuery.uiLength = (uint32)uiLength;
	DefineIdentity(pcDefine, &sQuery.sIdentity);
	psRecorder->cSeen.Add(sQuery.szName, uiLength);
	psRecorder->psHeader->asQueries.Add(&sQuery);
}


//////////////////////////////////////////////////////////////////////////
//
// The macro szName was defined or undefined.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::NoteDefined(const char* szName, size uiLength)
{
	SPPRecorder*	psRecorder;
	SPPHeaderName	sName;

	psRecorder = Recording();
	if (!psRecorder)
	{
		return;
	}
	sName.szName = psRecorder->psHeader->AddString(szName, uiLength);
	sName.uiLength = (uint32)uiLength;
	psRecorder->cSeen.Add(sName.szName, uiLength);
	psRecorder->psHeader->asDefined.Add(&sName);
}


//////////////////////////////////////////////////////////////////////////
//
// pcFile was #included, and was skipped if bOnce (it had #pragma once).
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::NotePragmaQuery(CSourceFile* pcFile, bool bOnce)
{
	SPPRecorder*			psRecorder;
	SPPHeaderPragmaQuery	sQuery;

	psRecorder = Recording();
	if (!psRecorder || psRecorder->apcPragmaSeen.Contains(&pcFile))
	{
		return;
	}
	psRecorder->apcPragmaSeen.Add(&pcFile);
	sQuery.pcFile = pcFile;
	sQuery.bOnce = bOnce;
	psRecorder->psHeader->asPragmaQueries.Add(&sQuery);
}


//////////////////////////////////////////////////////////////////////////
//
// A #define, #undef or #pragma once, to be done again when the header is
// replayed.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::NoteEvent(EPPHeaderEvent eType, CSourceFile* pcFile, CPPDirective* pcDirective)
{
	SPPRecorder*	psRecorder;
	SPPHeaderEvent	sEvent;

	psRecorder = Recording();
	if (!psRecorder)
	{
		return;
	}
	sEvent.eType = (uint8)eType;
	sEvent.pcFile = pcFile;
	sEvent.pcDirective = pcDirective;
	psRecorder->psHeader->asEvents.Add(&sEvent);
	if (eType == PPHE_PragmaOnce)
	{
		if (!psRecorder->apcPragmaSeen.Contains(&pcFile))
		{
			psRecorder->apcPragmaSeen.Add(&pcFile);
		}
		psRecorder->psHeader->apcPragmaOnced.Add(&pcFile);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Which macro pcDefine is (SPPDefineIdentity), none if it is NULL.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::DefineIdentity(CDefine* pcDefine, SPPDefineIdentity* psIdentity)
{
	psIdentity->Init();
	if (!pcDefine)
	{
		return;
	}
	if (pcDefine->GetFile())
	{
		psIdentity->pcFile = pcDefine->GetFile();
		psIdentity->iLine = pcDefine->GetLine();
		psIdentity->iColumn = pcDefine->GetColumn();
	}
	else
	{
		psIdentity->uiHash = pcDefine->ContentHash();
	}
}
