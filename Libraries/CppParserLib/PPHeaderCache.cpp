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
#include "BaseLib/Numbers.h"
#include "PPHeaderCache.h"


#define PP_HEADER_CACHE_STRINGS		(4 KB)


//////////////////////////////////////////////////////////////////////////
//
// No macro.
//
//////////////////////////////////////////////////////////////////////////
void SPPDefineIdentity::Init(void)
{
	pcFile = NULL;
	iLine = -1;
	iColumn = -1;
	uiHash = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SPPDefineIdentity::Equals(SPPDefineIdentity* psOther)
{
	return (pcFile == psOther->pcFile) && (iLine == psOther->iLine) && (iColumn == psOther->iColumn) && (uiHash == psOther->uiHash);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SPPCachedHeader::Init(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext)
{
	this->pcFile = pcFile;
	this->pcHeaderNameMap = pcHeaderNameMap;
	this->uiContext = uiContext;
	asQueries.Init();
	asPragmaQueries.Init();
	asDefined.Init();
	apcPragmaOnced.Init();
	asSegments.Init();
	asTokens.Init();
	asExpansions.Init();
	asInactiveRegions.Init();
	asDirectiveLines.Init();
	asEvents.Init();
	uiEndPendingFlags = 0;
	uiTotalTokens = 0;
	mcStrings.Init(PP_HEADER_CACHE_STRINGS);
	psNext = NULL;
	apcFiles.Init();
	bForget = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SPPCachedHeader::Kill(void)
{
	mcStrings.Kill();
	apcFiles.Kill();
	asEvents.Kill();
	asDirectiveLines.Kill();
	asInactiveRegions.Kill();
	asExpansions.Kill();
	asTokens.Kill();
	asSegments.Kill();
	apcPragmaOnced.Kill();
	asDefined.Kill();
	asPragmaQueries.Kill();
	asQueries.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of szText (uiLength characters, NULL terminated) that lasts as
// long as the header.
//
//////////////////////////////////////////////////////////////////////////
char* SPPCachedHeader::AddString(const char* szText, size uiLength)
{
	char*	sz;

	sz = (char*)mcStrings.Add(uiLength + 1);
	memcpy(sz, szText, uiLength);
	sz[uiLength] = '\0';
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
// The files anything the header kept of its own (not its children's)
// came from or points into, noted once it is recorded: the header itself,
// its tokens (their text is the file's when it is a header), expansions,
// skipped lines and directives, the #defines and #undefs it replays, the
// macros it looked up and the #pragma once files.
//
//////////////////////////////////////////////////////////////////////////
void SPPCachedHeader::NoteFiles(void)
{
	size	i;

	apcFiles.ReInit();
	AddFile(pcFile);
	for (i = 0; i < asTokens.NumElements(); i++)
	{
		AddFile(asTokens.Get(i)->pcFile);
	}
	for (i = 0; i < asExpansions.NumElements(); i++)
	{
		AddFile(asExpansions.Get(i)->pcFile);
	}
	for (i = 0; i < asInactiveRegions.NumElements(); i++)
	{
		AddFile(asInactiveRegions.Get(i)->pcFile);
	}
	for (i = 0; i < asDirectiveLines.NumElements(); i++)
	{
		AddFile(asDirectiveLines.Get(i)->pcFile);
	}
	for (i = 0; i < asEvents.NumElements(); i++)
	{
		AddFile(asEvents.Get(i)->pcFile);
	}
	for (i = 0; i < asQueries.NumElements(); i++)
	{
		AddFile(asQueries.Get(i)->sIdentity.pcFile);
	}
	for (i = 0; i < asPragmaQueries.NumElements(); i++)
	{
		AddFile(asPragmaQueries.Get(i)->pcFile);
	}
	for (i = 0; i < apcPragmaOnced.NumElements(); i++)
	{
		AddFile(*apcPragmaOnced.Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcFile added to the files noted, unless it is NULL or noted already
// (they are few, and runs of records share a file).
//
//////////////////////////////////////////////////////////////////////////
void SPPCachedHeader::AddFile(CSourceFile* pcFile)
{
	size	i;

	if (pcFile == NULL)
	{
		return;
	}
	for (i = apcFiles.NumElements(); i > 0; i--)
	{
		if (*apcFiles.Get(i - 1) == pcFile)
		{
			return;
		}
	}
	apcFiles.Add(&pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether anything the header kept of its own came from or points into
// any of papcFiles (NoteFiles).
//
//////////////////////////////////////////////////////////////////////////
bool SPPCachedHeader::Refers(CArrayPPHeaderFiles* papcFiles)
{
	size	i;
	size	j;

	for (i = 0; i < apcFiles.NumElements(); i++)
	{
		for (j = 0; j < papcFiles->NumElements(); j++)
		{
			if (*apcFiles.Get(i) == *papcFiles->Get(j))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Init(void)
{
	mcHeaders.Init(true);
	mapsHeaders.Init();
	mcMutex.Init();
	muiHits = 0;
	muiMisses = 0;
	muiReplayedTokens = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Kill(void)
{
	size				i;
	SPPCachedHeader*	psHeader;

	for (i = 0; i < mapsHeaders.NumElements(); i++)
	{
		psHeader = mapsHeaders.GetPtr(i);
		psHeader->Kill();
		free(psHeader);
	}
	mapsHeaders.Kill();
	mcHeaders.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Lock(void)
{
	mcMutex.Lock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Unlock(void)
{
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CPPHeaderCache::Key(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext)
{
	uint64	uiKey;

	uiKey = uiContext;
	uiKey = (uiKey ^ (uint64)(size)pcFile) * 0x100000001B3ULL;
	uiKey = (uiKey ^ (uint64)(size)pcHeaderNameMap) * 0x100000001B3ULL;
	return (int64)uiKey;
}


//////////////////////////////////////////////////////////////////////////
//
// The first header kept for pcFile found in pcHeaderNameMap in the context
// (the rest follow psNext, and may be another file's with the same key:
// compare them).  Only while locked.
//
//////////////////////////////////////////////////////////////////////////
SPPCachedHeader* CPPHeaderCache::Get(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext)
{
	SPPCachedHeader**	ppsHeader;

	ppsHeader = mcHeaders.Get(Key(pcFile, pcHeaderNameMap, uiContext));
	return ppsHeader ? *ppsHeader : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A header to fill in, then Publish (or Discard).
//
//////////////////////////////////////////////////////////////////////////
SPPCachedHeader* CPPHeaderCache::New(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext)
{
	SPPCachedHeader*	psHeader;

	psHeader = (SPPCachedHeader*)malloc(sizeof(SPPCachedHeader));
	psHeader->Init(pcFile, pcHeaderNameMap, uiContext);
	return psHeader;
}


//////////////////////////////////////////////////////////////////////////
//
// psHeader (from New, filled in) is found first after this.
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Publish(SPPCachedHeader* psHeader)
{
	int64	lliKey;

	lliKey = Key(psHeader->pcFile, psHeader->pcHeaderNameMap, psHeader->uiContext);
	psHeader->NoteFiles();
	mcMutex.Lock();
	psHeader->psNext = Get(psHeader->pcFile, psHeader->pcHeaderNameMap, psHeader->uiContext);
	mcHeaders.Put(lliKey, &psHeader);
	mapsHeaders.Add(psHeader);
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// psHeader (from New) isn't to be kept.
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Discard(SPPCachedHeader* psHeader)
{
	psHeader->Kill();
	free(psHeader);
}


//////////////////////////////////////////////////////////////////////////
//
// A unit's preprocessor found uiHits headers kept and replayed
// uiReplayedTokens tokens of them, and kept uiMisses (it preprocessed them).
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Count(size uiHits, size uiMisses, size uiReplayedTokens)
{
	mcMutex.Lock();
	muiHits += uiHits;
	muiMisses += uiMisses;
	muiReplayedTokens += uiReplayedTokens;
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// Every header kept that depends on pcFile is forgotten, before pcFile's
// text changes (or it is unloaded).
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Forget(CSourceFile* pcFile)
{
	CArrayPPHeaderFiles	apcFiles;

	apcFiles.Init();
	apcFiles.Add(&pcFile);
	Forget(&apcFiles);
	apcFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every header kept that depends on any of papcFiles is forgotten, before
// their text changes (or they are unloaded): one that refers to one of
// them (Refers), and one that included a header forgotten.  The others
// are kept.  Not while units are being preprocessed with the cache.
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Forget(CArrayPPHeaderFiles* papcFiles)
{
	size								i;
	size								j;
	SPPCachedHeader*					psHeader;
	SPPHeaderSegment*					psSegment;
	bool								bChanged;
	CArrayTemplatePtr<SPPCachedHeader>	apsKept;

	mcMutex.Lock();
	for (i = 0; i < mapsHeaders.NumElements(); i++)
	{
		psHeader = mapsHeaders.GetPtr(i);
		psHeader->bForget = psHeader->Refers(papcFiles);
	}

	//A child is usually kept before its includers, but replaying needn't keep that order.
	do
	{
		bChanged = false;
		for (i = 0; i < mapsHeaders.NumElements(); i++)
		{
			psHeader = mapsHeaders.GetPtr(i);
			for (j = 0; !psHeader->bForget && (j < psHeader->asSegments.NumElements()); j++)
			{
				psSegment = psHeader->asSegments.Get(j);
				if (psSegment->psChild && psSegment->psChild->bForget)
				{
					psHeader->bForget = true;
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);

	apsKept.Init();
	for (i = 0; i < mapsHeaders.NumElements(); i++)
	{
		psHeader = mapsHeaders.GetPtr(i);
		if (psHeader->bForget)
		{
			Unlink(psHeader);
			Discard(psHeader);
		}
		else
		{
			apsKept.Add(psHeader);
		}
	}
	mapsHeaders.Kill();
	memcpy(&mapsHeaders, &apsKept, sizeof(mapsHeaders));
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// psHeader taken out of the headers kept with its key (newest first, by
// psNext).
//
//////////////////////////////////////////////////////////////////////////
void CPPHeaderCache::Unlink(SPPCachedHeader* psHeader)
{
	int64				lliKey;
	SPPCachedHeader**	ppsFirst;
	SPPCachedHeader*	psPrevious;

	lliKey = Key(psHeader->pcFile, psHeader->pcHeaderNameMap, psHeader->uiContext);
	ppsFirst = mcHeaders.Get(lliKey);
	if (ppsFirst == NULL)
	{
		return;
	}
	if (*ppsFirst == psHeader)
	{
		if (psHeader->psNext)
		{
			*ppsFirst = psHeader->psNext;
		}
		else
		{
			mcHeaders.Remove(lliKey);
		}
		return;
	}
	for (psPrevious = *ppsFirst; psPrevious->psNext; psPrevious = psPrevious->psNext)
	{
		if (psPrevious->psNext == psHeader)
		{
			psPrevious->psNext = psHeader->psNext;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPPHeaderCache::NumHeaders(void)
{
	return mapsHeaders.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPPHeaderCache::NumHits(void)
{
	return muiHits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPPHeaderCache::NumMisses(void)
{
	return muiMisses;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPPHeaderCache::NumReplayedTokens(void)
{
	return muiReplayedTokens;
}
