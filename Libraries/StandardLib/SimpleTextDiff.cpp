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
#include "SimpleTextDiff.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ETextDiffBlockType STextDiffBlock::GetType(void)
{
	if (uiStartLine1 == uiEndLine1)
	{
		return TDBT_Inserted;
	}
	else if (uiStartLine2 == uiEndLine2)
	{
		return TDBT_Deleted;
	}
	else
	{
		return TDBT_Modified;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize STextDiffBlock::NumLines1(void)
{
	return uiEndLine1 - uiStartLine1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize STextDiffBlock::NumLines2(void)
{
	return uiEndLine2 - uiStartLine2;
}


//////////////////////////////////////////////////////////////////////////
//
// The contents start empty.  CTextDiffProducer (or GetContent1()->Set)
// sets them.
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextDiff::Init(const char* szTitle)
{
	mszTitle.Init(szTitle);
	mcContent1.Init();
	mcContent2.Init();
	macBlocks.Init();
	macFragments.Init();
	meWhitespace = TDW_Default;
	meHighlight = TDH_Lines;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextDiff::Kill(void)
{
	mcContent2.Kill();
	mcContent1.Kill();
	macFragments.Kill();
	macBlocks.Kill();
	mszTitle.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CSimpleTextDiff::GetTitle(void)
{
	return mszTitle.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextContent* CSimpleTextDiff::GetContent1(void)
{
	return &mcContent1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextContent* CSimpleTextDiff::GetContent2(void)
{
	return &mcContent2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ETextDiffWhitespace CSimpleTextDiff::GetWhitespace(void)
{
	return meWhitespace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ETextDiffHighlight CSimpleTextDiff::GetHighlight(void)
{
	return meHighlight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSimpleTextDiff::IsIdentical(void)
{
	return macBlocks.NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CSimpleTextDiff::NumBlocks(void)
{
	return macBlocks.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STextDiffBlock* CSimpleTextDiff::GetBlock(lsize uiBlock)
{
	return macBlocks.Get(uiBlock);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STextDiffFragment* CSimpleTextDiff::GetFragment(STextDiffBlock* psBlock, lsize uiFragment)
{
	return macFragments.Get(psBlock->uiFirstFragment + uiFragment);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CSimpleTextDiff::NumFragments(void)
{
	return macFragments.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextDiff::ClearChanges(ETextDiffWhitespace eWhitespace, ETextDiffHighlight eHighlight)
{
	macBlocks.ReInit();
	macFragments.ReInit();
	meWhitespace = eWhitespace;
	meHighlight = eHighlight;
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks must be added in order.
//
//////////////////////////////////////////////////////////////////////////
STextDiffBlock* CSimpleTextDiff::AddBlock(lsize uiStartLine1, lsize uiEndLine1, lsize uiStartLine2, lsize uiEndLine2)
{
	STextDiffBlock*	psBlock;

	psBlock = macBlocks.Add();
	psBlock->uiStartLine1 = uiStartLine1;
	psBlock->uiEndLine1 = uiEndLine1;
	psBlock->uiStartLine2 = uiStartLine2;
	psBlock->uiEndLine2 = uiEndLine2;
	psBlock->uiFirstFragment = macFragments.NumElements();
	psBlock->uiNumFragments = 0;
	return psBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// Fragments are added to the last block added, in order.
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextDiff::AddFragment(STextDiffBlock* psBlock, lsize uiStart1, lsize uiEnd1, lsize uiStart2, lsize uiEnd2)
{
	STextDiffFragment*	psFragment;

	psFragment = macFragments.Add();
	psFragment->uiStart1 = uiStart1;
	psFragment->uiEnd1 = uiEnd1;
	psFragment->uiStart2 = uiStart2;
	psFragment->uiEnd2 = uiEnd2;
	psBlock->uiNumFragments++;
}


//////////////////////////////////////////////////////////////////////////
//
// Prints the blocks as a unified diff with zero based line numbers and
// no context.  A last line without a separator is followed by git's
// "\ No newline at end of file".  Each fragment follows its block as
// ~[left]>[right].
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextDiff::Print(CChars* psz)
{
	lsize				uiBlock;
	lsize				uiFragment;
	STextDiffBlock*		psBlock;
	STextDiffFragment*	psFragment;

	psz->Append("--- ");
	psz->Append(mcContent1.GetTitle());
	psz->AppendNewLine();
	psz->Append("+++ ");
	psz->Append(mcContent2.GetTitle());
	psz->AppendNewLine();

	for (uiBlock = 0; uiBlock < macBlocks.NumElements(); uiBlock++)
	{
		psBlock = macBlocks.Get(uiBlock);
		psz->Append("@@ -");
		psz->Append((uint64)psBlock->uiStartLine1);
		psz->Append(',');
		psz->Append((uint64)psBlock->NumLines1());
		psz->Append(" +");
		psz->Append((uint64)psBlock->uiStartLine2);
		psz->Append(',');
		psz->Append((uint64)psBlock->NumLines2());
		psz->Append(" @@");
		psz->AppendNewLine();

		PrintLines(psz, &mcContent1, psBlock->uiStartLine1, psBlock->uiEndLine1, '-');
		PrintLines(psz, &mcContent2, psBlock->uiStartLine2, psBlock->uiEndLine2, '+');

		for (uiFragment = 0; uiFragment < psBlock->uiNumFragments; uiFragment++)
		{
			psFragment = GetFragment(psBlock, uiFragment);
			psz->Append("~[");
			PrintText(psz, &mcContent1, psFragment->uiStart1, psFragment->uiEnd1);
			psz->Append("]>[");
			PrintText(psz, &mcContent2, psFragment->uiStart2, psFragment->uiEnd2);
			psz->Append(']');
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextDiff::PrintLines(CChars* psz, CTextContent* pcContent, lsize uiStartLine, lsize uiEndLine, char cPrefix)
{
	lsize	uiLine;

	for (uiLine = uiStartLine; uiLine < uiEndLine; uiLine++)
	{
		psz->Append(cPrefix);
		psz->Append(pcContent->GetLineText(uiLine), (size)pcContent->GetLineLength(uiLine));
		psz->AppendNewLine();
		if (pcContent->GetLineSeparator(uiLine) == TLS_None)
		{
			psz->Append("\\ No newline at end of file");
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Line breaks inside fragments are printed as \n (and \r).
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextDiff::PrintText(CChars* psz, CTextContent* pcContent, lsize uiStart, lsize uiEnd)
{
	lsize	ui;
	char*	szText;

	szText = pcContent->Text();
	for (ui = uiStart; ui < uiEnd; ui++)
	{
		if (szText[ui] == '\n')
		{
			psz->Append("\\n");
		}
		else if (szText[ui] == '\r')
		{
			psz->Append("\\r");
		}
		else
		{
			psz->Append(szText[ui]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextDiff::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}
