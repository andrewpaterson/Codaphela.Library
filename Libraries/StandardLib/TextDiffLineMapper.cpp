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
#include "TextDiffLineMapper.h"


//////////////////////////////////////////////////////////////////////////
//
// pcDiff is only read, and must outlive the mapper.
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffLineMapper::Init(CSimpleTextDiff* pcDiff)
{
	mpcDiff = pcDiff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffLineMapper::Kill(void)
{
	mpcDiff = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A block's lines on the side mapped from and the side mapped to.
//
//////////////////////////////////////////////////////////////////////////
static void TextDiffBlockSides(STextDiffBlock* psBlock, ETextDiffSide eFrom, lsize* puiFromStart, lsize* puiFromEnd, lsize* puiToStart, lsize* puiToEnd)
{
	if (eFrom == TDS_Left)
	{
		*puiFromStart = psBlock->uiStartLine1;
		*puiFromEnd = psBlock->uiEndLine1;
		*puiToStart = psBlock->uiStartLine2;
		*puiToEnd = psBlock->uiEndLine2;
	}
	else
	{
		*puiFromStart = psBlock->uiStartLine2;
		*puiFromEnd = psBlock->uiEndLine2;
		*puiToStart = psBlock->uiStartLine1;
		*puiToEnd = psBlock->uiEndLine1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first block that ends, on side eFrom, after fLine: fLine is before
// it or in it.  NumBlocks if none does.  The blocks are in order on both
// sides, so it is a binary search, of whole lines: a block's end is past
// fLine exactly when it is past fLine's whole line.
//
//////////////////////////////////////////////////////////////////////////
lsize CTextDiffLineMapper::FindBlockEndingAfter(ETextDiffSide eFrom, float64 fLine)
{
	lsize			uiLine;
	lsize			uiLow;
	lsize			uiHigh;
	lsize			uiMiddle;
	STextDiffBlock*	psBlock;
	lsize			uiEnd;

	if (fLine < 0)
	{
		//Every block ends at or after line 0.
		return 0;
	}

	uiLine = (lsize)fLine;
	uiLow = 0;
	uiHigh = mpcDiff->NumBlocks();
	while (uiLow < uiHigh)
	{
		uiMiddle = uiLow + (uiHigh - uiLow) / 2;
		psBlock = mpcDiff->GetBlock(uiMiddle);
		uiEnd = (eFrom == TDS_Left) ? psBlock->uiEndLine1 : psBlock->uiEndLine2;
		if (uiEnd > uiLine)
		{
			uiHigh = uiMiddle;
		}
		else
		{
			uiLow = uiMiddle + 1;
		}
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// The line on the other side that fLine on side eFrom lines up with.
//
//////////////////////////////////////////////////////////////////////////
float64 CTextDiffLineMapper::TransferLine(ETextDiffSide eFrom, float64 fLine)
{
	lsize			uiNumBlocks;
	lsize			uiBlock;
	lsize			uiFromStart;
	lsize			uiFromEnd;
	lsize			uiToStart;
	lsize			uiToEnd;

	uiNumBlocks = mpcDiff->NumBlocks();
	if (uiNumBlocks == 0)
	{
		return fLine;
	}

	uiBlock = FindBlockEndingAfter(eFrom, fLine);
	if (uiBlock == uiNumBlocks)
	{
		//After the last block.
		TextDiffBlockSides(mpcDiff->GetBlock(uiNumBlocks - 1), eFrom, &uiFromStart, &uiFromEnd, &uiToStart, &uiToEnd);
		return fLine + ((float64)uiToEnd - (float64)uiFromEnd);
	}

	TextDiffBlockSides(mpcDiff->GetBlock(uiBlock), eFrom, &uiFromStart, &uiFromEnd, &uiToStart, &uiToEnd);
	if (fLine < (float64)uiFromStart)
	{
		//In the unchanged lines before the block.
		return fLine + ((float64)uiToStart - (float64)uiFromStart);
	}
	return (float64)uiToStart + (fLine - (float64)uiFromStart) * (float64)(uiToEnd - uiToStart) / (float64)(uiFromEnd - uiFromStart);
}

