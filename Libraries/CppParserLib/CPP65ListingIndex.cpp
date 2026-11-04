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
#include <string.h>
#include <stdlib.h>
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "CPP65ListingIndex.h"


#define CPP65_LISTING_LINE			".line "
#define CPP65_LISTING_SECTION		".section "
#define CPP65_LISTING_TEXT_SECTION	"text."
#define CPP65_LISTING_FINGERPRINT	".fingerprint "


//////////////////////////////////////////////////////////////////////////
//
// Ranges by file, line, listing, function and first line: a source
// line's places are together, each place's ranges in order.
//
//////////////////////////////////////////////////////////////////////////
static int CompareCPP65ListingRange(const void* pvLeft, const void* pvRight)
{
	SCPP65ListingRange*	psLeft;
	SCPP65ListingRange*	psRight;

	psLeft = (SCPP65ListingRange*)pvLeft;
	psRight = (SCPP65ListingRange*)pvRight;
	if (psLeft->uiFile != psRight->uiFile)
	{
		return (psLeft->uiFile < psRight->uiFile) ? -1 : 1;
	}
	if (psLeft->uiLine != psRight->uiLine)
	{
		return (psLeft->uiLine < psRight->uiLine) ? -1 : 1;
	}
	if (psLeft->uiListing != psRight->uiListing)
	{
		return (psLeft->uiListing < psRight->uiListing) ? -1 : 1;
	}
	if (psLeft->uiFunction != psRight->uiFunction)
	{
		return (psLeft->uiFunction < psRight->uiFunction) ? -1 : 1;
	}
	if (psLeft->uiFirstLine != psRight->uiFirstLine)
	{
		return (psLeft->uiFirstLine < psRight->uiFirstLine) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::Init(void)
{
	masListings.Init();
	maszFiles.Init();
	mcFileIndices.Init(false);
	maszFunctions.Init();
	mcFunctionIndices.Init();
	masRanges.Init();
	mbSorted = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::Kill(void)
{
	size			i;
	SCPP65Listing*	psListing;

	masRanges.Kill();
	mcFunctionIndices.Kill();
	maszFunctions.Kill();
	mcFileIndices.Kill();
	maszFiles.Kill();
	for (i = 0; i < masListings.NumElements(); i++)
	{
		psListing = masListings.Get(i);
		psListing->auiLineStarts.Kill();
		psListing->szText.Kill();
		psListing->szName.Kill();
	}
	masListings.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Reads and indexes the listing file szFileName, named by its name.
// False if it can't be read.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65ListingIndex::ReadListing(const char* szFileName)
{
	CTextFile	cFile;
	bool		bResult;

	cFile.Init();
	bResult = cFile.Read(szFileName);
	if (bResult)
	{
		AddListing(szFileName, cFile.Text() ? cFile.Text() : "");
	}
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Indexes szText, a listing, kept as szName.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::AddListing(const char* szName, const char* szText)
{
	SCPP65Listing*	psListing;
	size			uiPosition;
	char*			szCopy;

	psListing = masListings.Add();
	psListing->szName.Init(szName);
	psListing->szText.Init(szText);
	psListing->auiLineStarts.Init();
	szCopy = psListing->szText.Text();
	*psListing->auiLineStarts.Add() = 0;
	for (uiPosition = 0; szCopy[uiPosition] != '\0'; uiPosition++)
	{
		if ((szCopy[uiPosition] == '\n') && (szCopy[uiPosition + 1] != '\0'))
		{
			*psListing->auiLineStarts.Add() = uiPosition + 1;
		}
	}
	IndexListing(masListings.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// A source line's code runs from the line after its .line to the next
// .line, .section or .fingerprint (labels in it are kept), less blank
// lines at its end.  Its function is its section's name less "text.".
// Code before a section's first .line has no source line.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::IndexListing(size uiListing)
{
	SCPP65Listing*		psListing;
	size				uiLine;
	const char*			szText;
	const char*			szLine;
	const char*			szEnd;
	const char*			szTrimmed;
	const char*			szName;
	SCPP65ListingRange	sRange;
	bool				bInRange;
	CChars				szFile;

	psListing = masListings.Get(uiListing);
	szText = psListing->szText.Text();
	memset(&sRange, 0, sizeof(sRange));
	sRange.uiListing = uiListing;
	sRange.uiFunction = MAX_SIZE;
	bInRange = false;
	for (uiLine = 0; uiLine < psListing->auiLineStarts.NumElements(); uiLine++)
	{
		szLine = szText + *psListing->auiLineStarts.Get(uiLine);
		szEnd = strchr(szLine, '\n');
		if (!szEnd)
		{
			szEnd = szLine + strlen(szLine);
		}
		if ((szEnd > szLine) && (szEnd[-1] == '\r'))
		{
			szEnd--;
		}
		for (szTrimmed = szLine; (szTrimmed < szEnd) && ((*szTrimmed == ' ') || (*szTrimmed == '\t')); szTrimmed++)
		{
		}

		//Directives are indented; a line at the start is a label, kept in the range like an instruction.
		if ((szTrimmed == szLine) || !(StartsWith(szTrimmed, szEnd, CPP65_LISTING_LINE) || StartsWith(szTrimmed, szEnd, CPP65_LISTING_SECTION) || StartsWith(szTrimmed, szEnd, CPP65_LISTING_FINGERPRINT)))
		{
			//A line that isn't blank is the last of the range so far.
			if (bInRange && (szTrimmed < szEnd))
			{
				sRange.uiEndLine = uiLine + 1;
			}
			continue;
		}

		EndRange(&sRange, bInRange);
		bInRange = false;
		if (StartsWith(szTrimmed, szEnd, CPP65_LISTING_LINE))
		{
			szFile.Init();
			if (ReadLineDirective(szTrimmed + strlen(CPP65_LISTING_LINE), szEnd, &szFile, &sRange.uiLine))
			{
				sRange.uiFile = AddFile(szFile.Text());
				sRange.uiFirstLine = uiLine + 1;
				sRange.uiEndLine = uiLine + 1;
				bInRange = true;
			}
			szFile.Kill();
		}
		else if (StartsWith(szTrimmed, szEnd, CPP65_LISTING_SECTION))
		{
			szName = szTrimmed + strlen(CPP65_LISTING_SECTION);
			if (StartsWith(szName, szEnd, CPP65_LISTING_TEXT_SECTION))
			{
				szName += strlen(CPP65_LISTING_TEXT_SECTION);
			}
			sRange.uiFunction = AddFunction(szName, (size)(szEnd - szName));
		}
	}
	EndRange(&sRange, bInRange);
	mbSorted = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The range being read is kept if it has code and a function.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::EndRange(SCPP65ListingRange* psRange, bool bInRange)
{
	if (bInRange && (psRange->uiEndLine > psRange->uiFirstLine) && (psRange->uiFunction != MAX_SIZE))
	{
		*masRanges.Add() = *psRange;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the text from szText to szEnd starts with szPrefix.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65ListingIndex::StartsWith(const char* szText, const char* szEnd, const char* szPrefix)
{
	size	uiLength;

	uiLength = strlen(szPrefix);
	return ((size)(szEnd - szText) >= uiLength) && (strncmp(szText, szPrefix, uiLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// szText is the rest of a .line: "file", line.  The file's backslashes
// and quotes are escaped.  False if it isn't that.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65ListingIndex::ReadLineDirective(const char* szText, const char* szEnd, CChars* pszFile, size* puiLine)
{
	char*	szNumberEnd;
	long	lLine;

	if ((szText >= szEnd) || (*szText != '"'))
	{
		return false;
	}
	szText++;
	while ((szText < szEnd) && (*szText != '"'))
	{
		if ((*szText == '\\') && (szText + 1 < szEnd))
		{
			szText++;
		}
		pszFile->Append(*szText);
		szText++;
	}
	if (szText >= szEnd)
	{
		return false;
	}
	szText++;
	while ((szText < szEnd) && ((*szText == ',') || (*szText == ' ')))
	{
		szText++;
	}
	lLine = strtol(szText, &szNumberEnd, 10);
	if ((szNumberEnd == szText) || (lLine < 1))
	{
		return false;
	}

	//.line counts from 1; the index from 0.
	*puiLine = (size)(lLine - 1);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The places line uiLine (from 0) of szSourceFile is in, replacing
// pasDest's, in the order the listings were added.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::GetPlaces(const char* szSourceFile, size uiLine, CArrayCPP65ListingPlaces* pasDest)
{
	CChars				szFile;
	size*				puiFile;
	SCPP65ListingRange	sFind;
	size				uiRange;
	SCPP65ListingRange*	psRange;
	SCPP65ListingPlace*	psPlace;

	pasDest->ReInit();
	szFile.Init(szSourceFile);
	NormaliseFile(&szFile);
	puiFile = mcFileIndices.Get(szFile.Text());
	szFile.Kill();
	if (!puiFile)
	{
		return;
	}
	Sort();

	//The first range of the line: the first at or after the smallest key with its file and line.
	memset(&sFind, 0, sizeof(sFind));
	sFind.uiFile = *puiFile;
	sFind.uiLine = uiLine;
	masRanges.FindInSorted(&sFind, CompareCPP65ListingRange, &uiRange);

	psPlace = NULL;
	for (; uiRange < masRanges.NumElements(); uiRange++)
	{
		psRange = masRanges.Get(uiRange);
		if ((psRange->uiFile != *puiFile) || (psRange->uiLine != uiLine))
		{
			break;
		}
		if (!psPlace || (psPlace->uiListing != psRange->uiListing) || (psPlace->uiFunction != psRange->uiFunction))
		{
			psPlace = pasDest->Add();
			psPlace->uiListing = psRange->uiListing;
			psPlace->uiFunction = psRange->uiFunction;
			psPlace->uiFirstRange = uiRange;
			psPlace->uiNumRanges = 0;
		}
		psPlace->uiNumRanges++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Appends listing uiListing's lines uiFirstLine to uiEndLine (exclusive)
// to pszDest, each with its line feed.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::AppendLines(size uiListing, size uiFirstLine, size uiEndLine, CChars* pszDest)
{
	SCPP65Listing*	psListing;
	size			uiNumLines;
	size			uiStart;
	size			uiEnd;

	psListing = masListings.Get(uiListing);
	uiNumLines = psListing->auiLineStarts.NumElements();
	if (uiEndLine > uiNumLines)
	{
		uiEndLine = uiNumLines;
	}
	if (uiFirstLine >= uiEndLine)
	{
		return;
	}

	uiStart = *psListing->auiLineStarts.Get(uiFirstLine);
	uiEnd = (uiEndLine < uiNumLines) ? *psListing->auiLineStarts.Get(uiEndLine) : psListing->szText.Length();
	pszDest->AppendSubString(psListing->szText, uiStart, uiEnd);
	if ((uiEnd > uiStart) && (psListing->szText.GetChar(uiEnd - 1) != '\n'))
	{
		pszDest->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The index of szFile, added if it is new.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65ListingIndex::AddFile(const char* szFile)
{
	CChars	szKey;
	size*	puiIndex;
	size	uiIndex;

	szKey.Init(szFile);
	NormaliseFile(&szKey);
	puiIndex = mcFileIndices.Get(szKey.Text());
	if (puiIndex)
	{
		uiIndex = *puiIndex;
	}
	else
	{
		uiIndex = maszFiles.NumElements();
		maszFiles.Add(szFile);
		mcFileIndices.Put(szKey.Text(), &uiIndex);
	}
	szKey.Kill();
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the uiLength characters of szFunction, added if new.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65ListingIndex::AddFunction(const char* szFunction, size uiLength)
{
	CChars	szName;
	size*	puiIndex;
	size	uiIndex;

	szName.Init();
	szName.Append(szFunction, uiLength);
	puiIndex = mcFunctionIndices.Get(szName.Text());
	if (puiIndex)
	{
		uiIndex = *puiIndex;
	}
	else
	{
		uiIndex = maszFunctions.NumElements();
		maszFunctions.Add(szName.Text());
		mcFunctionIndices.Put(szName.Text(), &uiIndex);
	}
	szName.Kill();
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// The ranges are sorted once after listings are added, when they are
// next looked in.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::Sort(void)
{
	if (!mbSorted)
	{
		masRanges.QuickSort(CompareCPP65ListingRange);
		mbSorted = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A file name as it is looked up: either slash is the file separator.
// (The map ignores case.)
//
//////////////////////////////////////////////////////////////////////////
void CCPP65ListingIndex::NormaliseFile(CChars* pszFile)
{
	pszFile->Replace('/', FILE_SEPARATOR[0]);
	pszFile->Replace('\\', FILE_SEPARATOR[0]);
}


size CCPP65ListingIndex::NumListings(void) { return masListings.NumElements(); }
char* CCPP65ListingIndex::GetListingName(size uiListing) { return masListings.Get(uiListing)->szName.Text(); }
size CCPP65ListingIndex::NumListingLines(size uiListing) { return masListings.Get(uiListing)->auiLineStarts.NumElements(); }
size CCPP65ListingIndex::NumFiles(void) { return maszFiles.NumElements(); }
char* CCPP65ListingIndex::GetFile(size uiFile) { return maszFiles.GetText(uiFile); }
char* CCPP65ListingIndex::GetFunction(size uiFunction) { return maszFunctions.GetText(uiFunction); }
size CCPP65ListingIndex::NumRanges(void) { return masRanges.NumElements(); }
SCPP65ListingRange* CCPP65ListingIndex::GetRange(size uiRange) { Sort(); return masRanges.Get(uiRange); }

