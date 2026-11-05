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
#ifndef __SIMPLE_TEXT_DIFF_H__
#define __SIMPLE_TEXT_DIFF_H__
#include "TextContent.h"
#include "TextDiffPolicies.h"


enum ETextDiffBlockType
{
	TDBT_Inserted,  //Lines only on the right.
	TDBT_Deleted,  //Lines only on the left.
	TDBT_Modified
};


//Changed characters inside a modified block, as offsets into each side's whole text: [uiStart1, uiEnd1) on the left
//became [uiStart2, uiEnd2) on the right.  An empty side is the point the other side's characters were inserted at or
//deleted from.
struct STextDiffFragment
{
	lsize	uiStart1;
	lsize	uiEnd1;
	lsize	uiStart2;
	lsize	uiEnd2;
};


//Lines [uiStartLine1, uiEndLine1) on the left became lines [uiStartLine2, uiEndLine2) on the right.  Lines are counted
//from zero.  A modified block's fragments are uiNumFragments fragments from uiFirstFragment in the diff's fragments;
//inserted and deleted blocks, and blocks compared by line only, have none.
struct STextDiffBlock
{
	lsize	uiStartLine1;
	lsize	uiEndLine1;
	lsize	uiStartLine2;
	lsize	uiEndLine2;
	lsize	uiFirstFragment;
	lsize	uiNumFragments;

	ETextDiffBlockType	GetType(void);
	lsize				NumLines1(void);
	lsize				NumLines2(void);
};


typedef CArrayLargeTemplate<STextDiffBlock>		CArrayTextDiffBlock;
typedef CArrayLargeTemplate<STextDiffFragment>	CArrayTextDiffFragment;


//A two sided text diff, like IntelliJ's SimpleDiffRequest of two DocumentContents: a title, the left (old) and right
//(new) content and the changed blocks between them, in order.  Lines between blocks are unchanged (or differ only in
//ways the whitespace policy ignores).  CTextDiffProducer fills it; a view only reads it.
class CSimpleTextDiff
{
friend class CTextDiffProducer;
protected:
	CChars					mszTitle;
	CTextContent			mcContent1;
	CTextContent			mcContent2;
	CArrayTextDiffBlock		macBlocks;
	CArrayTextDiffFragment	macFragments;
	ETextDiffWhitespace		meWhitespace;
	ETextDiffHighlight		meHighlight;

public:
	void				Init(const char* szTitle);
	void				Kill(void);

	char*				GetTitle(void);
	CTextContent*		GetContent1(void);
	CTextContent*		GetContent2(void);
	ETextDiffWhitespace	GetWhitespace(void);
	ETextDiffHighlight	GetHighlight(void);

	bool				IsIdentical(void);
	lsize				NumBlocks(void);
	STextDiffBlock*		GetBlock(lsize uiBlock);
	STextDiffFragment*	GetFragment(STextDiffBlock* psBlock, lsize uiFragment);
	lsize				NumFragments(void);

	void				Print(CChars* psz);
	void				Dump(void);

protected:
	void				ClearChanges(ETextDiffWhitespace eWhitespace, ETextDiffHighlight eHighlight);
	STextDiffBlock*		AddBlock(lsize uiStartLine1, lsize uiEndLine1, lsize uiStartLine2, lsize uiEndLine2);
	void				AddFragment(STextDiffBlock* psBlock, lsize uiStart1, lsize uiEnd1, lsize uiStart2, lsize uiEnd2);
	void				PrintLines(CChars* psz, CTextContent* pcContent, lsize uiStartLine, lsize uiEndLine, char cPrefix);
	void				PrintText(CChars* psz, CTextContent* pcContent, lsize uiStart, lsize uiEnd);
};


#endif // __SIMPLE_TEXT_DIFF_H__
