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
// The line on the other side that fLine on side eFrom lines up with.
//
//////////////////////////////////////////////////////////////////////////
float64 CTextDiffLineMapper::TransferLine(ETextDiffSide eFrom, float64 fLine)
{
	lsize			uiBlock;
	STextDiffBlock*	psBlock;
	float64			fFromStart;
	float64			fFromEnd;
	float64			fToStart;
	float64			fToEnd;

	fFromEnd = 0;
	fToEnd = 0;
	for (uiBlock = 0; uiBlock < mpcDiff->NumBlocks(); uiBlock++)
	{
		psBlock = mpcDiff->GetBlock(uiBlock);
		if (eFrom == TDS_Left)
		{
			fFromStart = (float64)psBlock->uiStartLine1;
			fFromEnd = (float64)psBlock->uiEndLine1;
			fToStart = (float64)psBlock->uiStartLine2;
			fToEnd = (float64)psBlock->uiEndLine2;
		}
		else
		{
			fFromStart = (float64)psBlock->uiStartLine2;
			fFromEnd = (float64)psBlock->uiEndLine2;
			fToStart = (float64)psBlock->uiStartLine1;
			fToEnd = (float64)psBlock->uiEndLine1;
		}

		if (fLine < fFromStart)
		{
			//In the unchanged lines before the block.
			return fLine + (fToStart - fFromStart);
		}
		if (fLine < fFromEnd)
		{
			return fToStart + (fLine - fFromStart) * (fToEnd - fToStart) / (fFromEnd - fFromStart);
		}
	}

	//After the last block.
	return fLine + (fToEnd - fFromEnd);
}

