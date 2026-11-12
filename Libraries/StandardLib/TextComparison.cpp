/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/FNVHash.h"
#include "TextComparison.h"


//What every line separator compares as.
const char*	gszTextDiffSeparator = "\n";


//////////////////////////////////////////////////////////////////////////
//
// Spaces and tabs.  Line separators are never inside a line.
//
//////////////////////////////////////////////////////////////////////////
bool IsTextDiffWhitespace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\f') || (c == '\v');
}


//////////////////////////////////////////////////////////////////////////
//
// Letters, digits, underscores and the bytes of UTF-8 sequences.
//
//////////////////////////////////////////////////////////////////////////
bool IsTextDiffWordChar(char c)
{
	return ((c >= 'a') && (c <= 'z')) ||
		   ((c >= 'A') && (c <= 'Z')) ||
		   ((c >= '0') && (c <= '9')) ||
		   (c == '_') ||
		   ((uint8)c >= 0x80);
}


//////////////////////////////////////////////////////////////////////////
//
// Hashes sz with FNV-1a, skipping whitespace if it is ignored.
//
//////////////////////////////////////////////////////////////////////////
void STextToken::Init(const char* sz, lsize uiLength, lsize uiStart, lsize uiEnd, bool bSkipWhitespace, bool bUnterminated)
{
	lsize	ui;

	this->sz = sz;
	this->uiLength = uiLength;
	this->uiStart = uiStart;
	this->uiEnd = uiEnd;
	this->bSkipWhitespace = bSkipWhitespace;
	this->bUnterminated = bUnterminated;

	uiHash = FNV_HASH32_START;
	for (ui = 0; ui < uiLength; ui++)
	{
		if (!bSkipWhitespace || !IsTextDiffWhitespace(sz[ui]))
		{
			uiHash = FNVHash32(&sz[ui], 1, uiHash);
		}
	}
	if (bUnterminated)
	{
		uiHash = ~uiHash;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Both tokens must skip whitespace, or not, alike.
//
//////////////////////////////////////////////////////////////////////////
bool STextToken::Equals(STextToken* psOther)
{
	lsize	ui1;
	lsize	ui2;

	if ((uiHash != psOther->uiHash) || (bUnterminated != psOther->bUnterminated))
	{
		return false;
	}

	if (!bSkipWhitespace)
	{
		return (uiLength == psOther->uiLength) && (memcmp(sz, psOther->sz, uiLength) == 0);
	}

	ui1 = 0;
	ui2 = 0;
	for (;;)
	{
		while ((ui1 < uiLength) && IsTextDiffWhitespace(sz[ui1]))
		{
			ui1++;
		}
		while ((ui2 < psOther->uiLength) && IsTextDiffWhitespace(psOther->sz[ui2]))
		{
			ui2++;
		}
		if ((ui1 == uiLength) || (ui2 == psOther->uiLength))
		{
			return (ui1 == uiLength) && (ui2 == psOther->uiLength);
		}
		if (sz[ui1] != psOther->sz[ui2])
		{
			return false;
		}
		ui1++;
		ui2++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::Init(ETextDiffWhitespace eWhitespace, ETextDiffHighlight eHighlight)
{
	meWhitespace = eWhitespace;
	meHighlight = eHighlight;
	mcMyers.Init();
	macTokens1.Init();
	macTokens2.Init();
	mauiIds1.Init();
	mauiIds2.Init();
	mauiTable.Init();
	macChanges.Init();
	macWordFragments.Init();
	macRefinedFragments.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::Kill(void)
{
	macRefinedFragments.Kill();
	macWordFragments.Kill();
	macChanges.Kill();
	mauiTable.Kill();
	mauiIds2.Kill();
	mauiIds1.Kill();
	macTokens2.Kill();
	macTokens1.Kill();
	mcMyers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the changed blocks, as line ranges, to pacBlocks.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::CompareLines(CTextContent* pcContent1, CTextContent* pcContent2, CArrayDiffRange* pacBlocks)
{
	macTokens1.ReInit();
	macTokens2.ReInit();
	AddLineTokens(pcContent1, &macTokens1);
	AddLineTokens(pcContent2, &macTokens2);

	Intern();
	mcMyers.Diff(mauiIds1.GetData(), mauiIds1.NumElements(), mauiIds2.GetData(), mauiIds2.NumElements(), pacBlocks);
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the fragments of a block, as text offsets, to pacFragments.
// Returns false if the block is too big to refine.
//
//////////////////////////////////////////////////////////////////////////
bool CTextComparison::CompareFragments(CTextContent* pcContent1, CTextContent* pcContent2, SDiffRange* psBlock, CArrayDiffRange* pacFragments)
{
	lsize	uiStart1;
	lsize	uiStart2;

	if (!IsRefinable(pcContent1, psBlock->uiStart1, psBlock->uiEnd1) || !IsRefinable(pcContent2, psBlock->uiStart2, psBlock->uiEnd2))
	{
		return false;
	}

	macTokens1.ReInit();
	macTokens2.ReInit();
	AddBlockTokens(pcContent1, psBlock->uiStart1, psBlock->uiEnd1, &macTokens1);
	AddBlockTokens(pcContent2, psBlock->uiStart2, psBlock->uiEnd2, &macTokens2);
	uiStart1 = pcContent1->GetLineStart(psBlock->uiStart1);
	uiStart2 = pcContent2->GetLineStart(psBlock->uiStart2);

	if (meHighlight == TDH_Words)
	{
		macWordFragments.ReInit();
		DiffTokens(uiStart1, uiStart2, &macWordFragments);
		RefineGaps(pcContent1, pcContent2, pacFragments);
	}
	else
	{
		DiffTokens(uiStart1, uiStart2, pacFragments);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextComparison::IsRefinable(CTextContent* pcContent, lsize uiStartLine, lsize uiEndLine)
{
	//An empty side (an insertion or deletion) has nothing to refine against.
	if (uiEndLine <= uiStartLine)
	{
		return false;
	}
	return pcContent->GetLineEnd(uiEndLine - 1) - pcContent->GetLineStart(uiStartLine) <= TEXT_DIFF_MAX_REFINE_LENGTH;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::AddLineTokens(CTextContent* pcContent, CArrayTextToken* pacTokens)
{
	lsize	uiLine;
	lsize	uiStart;
	lsize	uiEnd;
	lsize	uiTrimmedStart;
	lsize	uiTrimmedEnd;
	char*	szText;
	bool	bUnterminated;

	szText = pcContent->Text();
	for (uiLine = 0; uiLine < pcContent->NumLines(); uiLine++)
	{
		uiStart = pcContent->GetLineStart(uiLine);
		uiEnd = pcContent->GetLineEnd(uiLine);
		uiTrimmedStart = uiStart;
		uiTrimmedEnd = uiEnd;
		if (meWhitespace == TDW_Trim)
		{
			TrimLine(szText, &uiTrimmedStart, &uiTrimmedEnd);
		}

		bUnterminated = (meWhitespace == TDW_Default) && (pcContent->GetLineSeparator(uiLine) == TLS_None);
		pacTokens->Add()->Init(&szText[uiTrimmedStart], uiTrimmedEnd - uiTrimmedStart, uiStart, uiEnd, meWhitespace == TDW_Ignore, bUnterminated);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Under TDW_Trim a line's leading and trailing whitespace makes no tokens,
// and under TDW_Ignore no whitespace does.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::AddBlockTokens(CTextContent* pcContent, lsize uiStartLine, lsize uiEndLine, CArrayTextToken* pacTokens)
{
	lsize				uiLine;
	lsize				uiStart;
	lsize				uiEnd;
	char*				szText;
	ETextLineSeparator	eSeparator;

	szText = pcContent->Text();
	for (uiLine = uiStartLine; uiLine < uiEndLine; uiLine++)
	{
		uiStart = pcContent->GetLineStart(uiLine);
		uiEnd = pcContent->GetLineEnd(uiLine);
		if (meWhitespace == TDW_Trim)
		{
			TrimLine(szText, &uiStart, &uiEnd);
		}

		if (meHighlight == TDH_Chars)
		{
			AddCharTokens(szText, uiStart, uiEnd, pacTokens);
		}
		else
		{
			AddWordTokens(szText, uiStart, uiEnd, pacTokens);
		}

		eSeparator = pcContent->GetLineSeparator(uiLine);
		if ((meWhitespace == TDW_Default) && (eSeparator != TLS_None))
		{
			uiEnd = pcContent->GetLineEnd(uiLine);
			pacTokens->Add()->Init(gszTextDiffSeparator, 1, uiEnd, uiEnd + (eSeparator == TLS_CRLF ? 2 : 1), false, false);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::TrimLine(const char* szText, lsize* puiStart, lsize* puiEnd)
{
	while ((*puiStart < *puiEnd) && IsTextDiffWhitespace(szText[*puiStart]))
	{
		(*puiStart)++;
	}
	while ((*puiStart < *puiEnd) && IsTextDiffWhitespace(szText[*puiEnd - 1]))
	{
		(*puiEnd)--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Words, whitespace runs (unless whitespace is ignored) and single
// punctuation characters.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::AddWordTokens(const char* szText, lsize uiStart, lsize uiEnd, CArrayTextToken* pacTokens)
{
	lsize	ui;
	lsize	uiTokenStart;

	ui = uiStart;
	while (ui < uiEnd)
	{
		uiTokenStart = ui;
		if (IsTextDiffWordChar(szText[ui]))
		{
			while ((ui < uiEnd) && IsTextDiffWordChar(szText[ui]))
			{
				ui++;
			}
			AddTextToken(szText, uiTokenStart, ui, pacTokens);
		}
		else if (IsTextDiffWhitespace(szText[ui]))
		{
			while ((ui < uiEnd) && IsTextDiffWhitespace(szText[ui]))
			{
				ui++;
			}
			if (meWhitespace != TDW_Ignore)
			{
				AddTextToken(szText, uiTokenStart, ui, pacTokens);
			}
		}
		else
		{
			ui++;
			AddTextToken(szText, uiTokenStart, ui, pacTokens);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Single characters, except whitespace if it is ignored.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::AddCharTokens(const char* szText, lsize uiStart, lsize uiEnd, CArrayTextToken* pacTokens)
{
	lsize	ui;

	for (ui = uiStart; ui < uiEnd; ui++)
	{
		if ((meWhitespace != TDW_Ignore) || !IsTextDiffWhitespace(szText[ui]))
		{
			AddTextToken(szText, ui, ui + 1, pacTokens);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::AddTextToken(const char* szText, lsize uiStart, lsize uiEnd, CArrayTextToken* pacTokens)
{
	pacTokens->Add()->Init(&szText[uiStart], uiEnd - uiStart, uiStart, uiEnd, false, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Diffs macTokens1 against macTokens2 and appends the changes as text
// fragments.  uiStart1 and uiStart2 are where an empty side goes when its
// tokens are empty.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::DiffTokens(lsize uiStart1, lsize uiStart2, CArrayDiffRange* pacFragments)
{
	lsize		uiChange;
	SDiffRange*	psChange;
	SDiffRange*	psFragment;

	Intern();
	macChanges.ReInit();
	mcMyers.Diff(mauiIds1.GetData(), mauiIds1.NumElements(), mauiIds2.GetData(), mauiIds2.NumElements(), &macChanges);

	for (uiChange = 0; uiChange < macChanges.NumElements(); uiChange++)
	{
		psChange = macChanges.Get(uiChange);
		psFragment = pacFragments->Add();
		GetFragmentSide(&macTokens1, psChange->uiStart1, psChange->uiEnd1, uiStart1, &psFragment->uiStart1, &psFragment->uiEnd1);
		GetFragmentSide(&macTokens2, psChange->uiStart2, psChange->uiEnd2, uiStart2, &psFragment->uiStart2, &psFragment->uiEnd2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives each token in macTokens1 and macTokens2 an id in mauiIds1 and
// mauiIds2: the index of the first equal token, counting through
// macTokens1 and then macTokens2.  Uses an open addressed hash table.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::Intern(void)
{
	lsize		uiNumTokens1;
	lsize		uiNumTokens;
	lsize		uiTableSize;
	lsize		uiMask;
	lsize		ui;
	lsize		uiSlot;
	lsize		uiEntry;
	lsize*		pauiTable;
	lsize*		puiId;
	STextToken*	psToken;

	uiNumTokens1 = macTokens1.NumElements();
	uiNumTokens = uiNumTokens1 + macTokens2.NumElements();
	uiTableSize = 16;
	while (uiTableSize < uiNumTokens * 2)
	{
		uiTableSize *= 2;
	}
	uiMask = uiTableSize - 1;

	mauiTable.Resize(uiTableSize);
	pauiTable = mauiTable.GetData();
	memset(pauiTable, 0, uiTableSize * sizeof(lsize));
	mauiIds1.Resize(uiNumTokens1);
	mauiIds2.Resize(macTokens2.NumElements());

	for (ui = 0; ui < uiNumTokens; ui++)
	{
		psToken = GetInternToken(ui);
		puiId = (ui < uiNumTokens1) ? mauiIds1.Get(ui) : mauiIds2.Get(ui - uiNumTokens1);

		//Table entries are token indices plus one; zero is empty.
		uiSlot = psToken->uiHash & uiMask;
		for (;;)
		{
			uiEntry = pauiTable[uiSlot];
			if (uiEntry == 0)
			{
				pauiTable[uiSlot] = ui + 1;
				*puiId = ui;
				break;
			}
			if (GetInternToken(uiEntry - 1)->Equals(psToken))
			{
				*puiId = uiEntry - 1;
				break;
			}
			uiSlot = (uiSlot + 1) & uiMask;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STextToken* CTextComparison::GetInternToken(lsize uiIndex)
{
	if (uiIndex < macTokens1.NumElements())
	{
		return macTokens1.Get(uiIndex);
	}
	else
	{
		return macTokens2.Get(uiIndex - macTokens1.NumElements());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The text covered by tokens [uiStart, uiEnd).  With no tokens it is the
// empty range after the token before, or before the token after, or at
// uiEmptyStart if there are no tokens at all.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::GetFragmentSide(CArrayTextToken* pacTokens, lsize uiStart, lsize uiEnd, lsize uiEmptyStart, lsize* puiStart, lsize* puiEnd)
{
	if (uiStart < uiEnd)
	{
		*puiStart = pacTokens->Get(uiStart)->uiStart;
		*puiEnd = pacTokens->Get(uiEnd - 1)->uiEnd;
	}
	else
	{
		if (uiStart > 0)
		{
			*puiStart = pacTokens->Get(uiStart - 1)->uiEnd;
		}
		else if (uiStart < pacTokens->NumElements())
		{
			*puiStart = pacTokens->Get(uiStart)->uiStart;
		}
		else
		{
			*puiStart = uiEmptyStart;
		}
		*puiEnd = *puiStart;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Compares each word fragment of only whitespace and punctuation again by
// character, then squashes the result into pacFragments.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::RefineGaps(CTextContent* pcContent1, CTextContent* pcContent2, CArrayDiffRange* pacFragments)
{
	lsize		uiFragment;
	SDiffRange*	psFragment;
	char*		szText1;
	char*		szText2;

	szText1 = pcContent1->Text();
	szText2 = pcContent2->Text();
	macRefinedFragments.ReInit();
	for (uiFragment = 0; uiFragment < macWordFragments.NumElements(); uiFragment++)
	{
		psFragment = macWordFragments.Get(uiFragment);
		if (IsGap(szText1, psFragment->uiStart1, psFragment->uiEnd1) && IsGap(szText2, psFragment->uiStart2, psFragment->uiEnd2))
		{
			macTokens1.ReInit();
			macTokens2.ReInit();
			AddCharTokens(szText1, psFragment->uiStart1, psFragment->uiEnd1, &macTokens1);
			AddCharTokens(szText2, psFragment->uiStart2, psFragment->uiEnd2, &macTokens2);
			DiffTokens(psFragment->uiStart1, psFragment->uiStart2, &macRefinedFragments);
		}
		else
		{
			macRefinedFragments.Add(psFragment);
		}
	}

	Squash(pcContent1, pcContent2, pacFragments);
}


//////////////////////////////////////////////////////////////////////////
//
// Non-empty text without words or line breaks.
//
//////////////////////////////////////////////////////////////////////////
bool CTextComparison::IsGap(const char* szText, lsize uiStart, lsize uiEnd)
{
	lsize	ui;
	char	c;

	if (uiStart == uiEnd)
	{
		return false;
	}
	for (ui = uiStart; ui < uiEnd; ui++)
	{
		c = szText[ui];
		if (IsTextDiffWordChar(c) || (c == '\n') || (c == '\r'))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends macRefinedFragments to pacFragments, merging fragments that are
// separated only by whitespace on both sides.  Fragments on different
// lines stay apart.
//
//////////////////////////////////////////////////////////////////////////
void CTextComparison::Squash(CTextContent* pcContent1, CTextContent* pcContent2, CArrayDiffRange* pacFragments)
{
	lsize		uiFragment;
	lsize		uiFirst;
	SDiffRange*	psFragment;
	SDiffRange*	psTail;

	uiFirst = pacFragments->NumElements();
	for (uiFragment = 0; uiFragment < macRefinedFragments.NumElements(); uiFragment++)
	{
		psFragment = macRefinedFragments.Get(uiFragment);
		psTail = (pacFragments->NumElements() > uiFirst) ? pacFragments->Tail() : NULL;
		if ((psTail != NULL) &&
			IsWhitespace(pcContent1->Text(), psTail->uiEnd1, psFragment->uiStart1) &&
			IsWhitespace(pcContent2->Text(), psTail->uiEnd2, psFragment->uiStart2))
		{
			psTail->uiEnd1 = psFragment->uiEnd1;
			psTail->uiEnd2 = psFragment->uiEnd2;
		}
		else
		{
			pacFragments->Add(psFragment);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Only whitespace, or nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CTextComparison::IsWhitespace(const char* szText, lsize uiStart, lsize uiEnd)
{
	lsize	ui;

	for (ui = uiStart; ui < uiEnd; ui++)
	{
		if (!IsTextDiffWhitespace(szText[ui]))
		{
			return false;
		}
	}
	return true;
}
