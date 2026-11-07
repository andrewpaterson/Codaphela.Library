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
#ifndef __TEXT_COMPARISON_H__
#define __TEXT_COMPARISON_H__
#include "MyersDiff.h"
#include "TextContent.h"
#include "TextDiffPolicies.h"


//A line, word, whitespace run, punctuation character or line separator being compared.  What is compared (sz) isn't
//always the text it covers ([uiStart, uiEnd) in its content): every separator compares as "\n", and a trimmed line
//covers its whole line.
struct STextToken
{
	const char*	sz;
	lsize		uiLength;
	lsize		uiStart;
	lsize		uiEnd;
	uint32		uiHash;
	bool		bSkipWhitespace;  //Whitespace in sz is ignored when comparing.
	bool		bUnterminated;  //The last line of a content that doesn't end with a separator.

	void	Init(const char* sz, lsize uiLength, lsize uiStart, lsize uiEnd, bool bSkipWhitespace, bool bUnterminated);
	bool	Equals(STextToken* psOther);
};


typedef CArrayLargeTemplate<STextToken>	CArrayTextToken;


//Compares two text contents (IntelliJ's ComparisonManager).  Lines are interned to ids under the whitespace policy and
//compared with Myers to find the changed blocks.  A modified block is then compared again as words or characters to
//find the fragments inside it.
//
//Under TDW_Default the line separators inside a block are tokens, so text moved across a line break shows, and a
//missing separator at the end of the text is a difference.  Separators compare equal whatever their kind.  Under
//TDW_Trim and TDW_Ignore separators are whitespace and ignored.
//
//Words are compared whole, as IntelliJ does.  A word fragment made only of whitespace and punctuation is compared
//again by character, and word fragments on a line separated only by whitespace are squashed into one.
class CTextComparison
{
protected:
	ETextDiffWhitespace			meWhitespace;
	ETextDiffHighlight			meHighlight;
	CMyersDiff					mcMyers;
	CArrayTextToken				macTokens1;
	CArrayTextToken				macTokens2;
	CArrayLargeTemplate<lsize>	mauiIds1;
	CArrayLargeTemplate<lsize>	mauiIds2;
	CArrayLargeTemplate<lsize>	mauiTable;
	CArrayDiffRange				macChanges;
	CArrayDiffRange				macWordFragments;
	CArrayDiffRange				macRefinedFragments;

public:
	void	Init(ETextDiffWhitespace eWhitespace, ETextDiffHighlight eHighlight);
	void	Kill(void);

	void	CompareLines(CTextContent* pcContent1, CTextContent* pcContent2, CArrayDiffRange* pacBlocks);
	bool	CompareFragments(CTextContent* pcContent1, CTextContent* pcContent2, SDiffRange* psBlock, CArrayDiffRange* pacFragments);

protected:
	void		AddLineTokens(CTextContent* pcContent, CArrayTextToken* pacTokens);
	void		AddBlockTokens(CTextContent* pcContent, lsize uiStartLine, lsize uiEndLine, CArrayTextToken* pacTokens);
	void		AddWordTokens(const char* szText, lsize uiStart, lsize uiEnd, CArrayTextToken* pacTokens);
	void		AddCharTokens(const char* szText, lsize uiStart, lsize uiEnd, CArrayTextToken* pacTokens);
	void		AddTextToken(const char* szText, lsize uiStart, lsize uiEnd, CArrayTextToken* pacTokens);
	void		TrimLine(const char* szText, lsize* puiStart, lsize* puiEnd);
	bool		IsRefinable(CTextContent* pcContent, lsize uiStartLine, lsize uiEndLine);

	void		DiffTokens(lsize uiStart1, lsize uiStart2, CArrayDiffRange* pacFragments);
	void		Intern(void);
	STextToken*	GetInternToken(lsize uiIndex);
	void		GetFragmentSide(CArrayTextToken* pacTokens, lsize uiStart, lsize uiEnd, lsize uiEmptyStart, lsize* puiStart, lsize* puiEnd);

	void		RefineGaps(CTextContent* pcContent1, CTextContent* pcContent2, CArrayDiffRange* pacFragments);
	bool		IsGap(const char* szText, lsize uiStart, lsize uiEnd);
	void		Squash(CTextContent* pcContent1, CTextContent* pcContent2, CArrayDiffRange* pacFragments);
	bool		IsWhitespace(const char* szText, lsize uiStart, lsize uiEnd);
};


bool	IsTextDiffWhitespace(char c);
bool	IsTextDiffWordChar(char c);


#endif // __TEXT_COMPARISON_H__
