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
#include "TextComparison.h"
#include "TextDiffProducer.h"


//////////////////////////////////////////////////////////////////////////
//
// IntelliJ's defaults: whitespace must match, changed words highlighted.
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffProducer::Init(void)
{
	Init(TDW_Default, TDH_Words);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffProducer::Init(ETextDiffWhitespace eWhitespace, ETextDiffHighlight eHighlight)
{
	meWhitespace = eWhitespace;
	meHighlight = eHighlight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffProducer::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffProducer::SetWhitespace(ETextDiffWhitespace eWhitespace)
{
	meWhitespace = eWhitespace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffProducer::SetHighlight(ETextDiffHighlight eHighlight)
{
	meHighlight = eHighlight;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the diff's blocks with those between its two contents.
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffProducer::Produce(CSimpleTextDiff* pcDiff)
{
	CTextComparison	cComparison;
	CArrayDiffRange	acBlocks;
	CArrayDiffRange	acFragments;
	lsize			uiBlock;
	lsize			uiFragment;
	SDiffRange*		psRange;
	SDiffRange*		psFragment;
	STextDiffBlock*	psBlock;
	CTextContent*	pcContent1;
	CTextContent*	pcContent2;

	pcDiff->ClearChanges(meWhitespace, meHighlight);
	pcContent1 = pcDiff->GetContent1();
	pcContent2 = pcDiff->GetContent2();

	cComparison.Init(meWhitespace, meHighlight);
	acBlocks.Init();
	acFragments.Init();

	cComparison.CompareLines(pcContent1, pcContent2, &acBlocks);
	for (uiBlock = 0; uiBlock < acBlocks.NumElements(); uiBlock++)
	{
		psRange = acBlocks.Get(uiBlock);
		psBlock = pcDiff->AddBlock(psRange->uiStart1, psRange->uiEnd1, psRange->uiStart2, psRange->uiEnd2);
		if ((meHighlight != TDH_Lines) && (psBlock->GetType() == TDBT_Modified))
		{
			acFragments.ReInit();
			if (cComparison.CompareFragments(pcContent1, pcContent2, psRange, &acFragments))
			{
				for (uiFragment = 0; uiFragment < acFragments.NumElements(); uiFragment++)
				{
					psFragment = acFragments.Get(uiFragment);
					pcDiff->AddFragment(psBlock, psFragment->uiStart1, psFragment->uiEnd1, psFragment->uiStart2, psFragment->uiEnd2);
				}
			}
		}
	}

	acFragments.Kill();
	acBlocks.Kill();
	cComparison.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDiffProducer::ProduceFromText(CSimpleTextDiff* pcDiff, const char* szTitle1, const char* szText1, const char* szTitle2, const char* szText2)
{
	pcDiff->GetContent1()->Set(szTitle1, szText1);
	pcDiff->GetContent2()->Set(szTitle2, szText2);
	Produce(pcDiff);
}


//////////////////////////////////////////////////////////////////////////
//
// The file names are the titles.  A file that can't be read is empty and
// false is returned, but the diff is still produced.
//
//////////////////////////////////////////////////////////////////////////
bool CTextDiffProducer::ProduceFromFiles(CSimpleTextDiff* pcDiff, const char* szFilename1, const char* szFilename2)
{
	bool	bResult1;
	bool	bResult2;

	bResult1 = pcDiff->GetContent1()->SetFromFile(szFilename1, szFilename1);
	bResult2 = pcDiff->GetContent2()->SetFromFile(szFilename2, szFilename2);
	Produce(pcDiff);
	return bResult1 && bResult2;
}
