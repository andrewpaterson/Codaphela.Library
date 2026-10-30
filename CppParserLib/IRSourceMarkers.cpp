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
#include "IRBlock.h"
#include "IRSourceMarkers.h"


//The most differences Match looks through; past it the lines from before are only found one after another.
#define IR_SOURCE_MARKERS_MAX_DIFFERENCES	4000


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRSourceMarkers::Init(void)
{
	maszLines.Init();
	maiBefore.Init();
	maszMarkers.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRSourceMarkers::Kill(void)
{
	maszMarkers.Kill();
	maiBefore.Kill();
	maszLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the marker lines out of the code from uiStart, remembering the line
// each was before.
//
//////////////////////////////////////////////////////////////////////////
void CIRSourceMarkers::Take(CChars* pszCode, size uiStart)
{
	CArrayChars	aszLines;
	CChars*		pszLine;
	size		ui;
	int32		iBefore;

	maszLines.ReInit();
	maiBefore.ReInit();
	maszMarkers.ReInit();
	if (strstr(pszCode->Text() + uiStart, IR_SOURCE_MARKER) == NULL)
	{
		return;
	}

	//The code ends with a new line, so its last split line is empty.
	aszLines.Init();
	aszLines.Split(pszCode->Text() + uiStart, '\n');
	if ((aszLines.NumElements() > 0) && aszLines.Tail()->Empty())
	{
		aszLines.RemoveTail();
	}
	pszCode->SetLength(uiStart);
	for (ui = 0; ui < aszLines.NumElements(); ui++)
	{
		pszLine = aszLines.Get(ui);
		if (pszLine->StartsWith(IR_SOURCE_MARKER))
		{
			iBefore = (int32)maszLines.NumElements();
			maiBefore.Add(&iBefore);
			maszMarkers.Add(pszLine->Text());
			continue;
		}
		maszLines.Add(pszLine->Text());
		pszCode->Append(*pszLine);
		pszCode->AppendNewLine();
	}
	aszLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the markers Take took out back into the code from uiStart: the
// passes keep the order of the lines they leave, so the lines from before
// are found as the longest run of them in order (Match); between two
// found, the lines not found are paired in order with
// the lines now there (as a pass rewrites a line in place).  A marker goes
// before the first line it was before that has a place now.  Of markers
// that go before one line only the last stays (the code of the others
// went).
//
//////////////////////////////////////////////////////////////////////////
void CIRSourceMarkers::Put(CChars* pszCode, size uiStart)
{
	CArrayChars				aszLines;
	CArrayTemplate<int32>	aiFound;		//Per line from before: where it is now, or -1.
	CArrayTemplate<int32>	aiMarker;		//Per line now (and one past the end): the marker that goes before it, or -1.
	size					ui;
	int32					iFound;
	int32					iMarker;
	int32					iBefore;
	int32					iPrevious;
	int32					iNew;
	size					uiGap;
	size					uiGapStart;

	if (maszMarkers.NumElements() == 0)
	{
		return;
	}

	aszLines.Init();
	aszLines.Split(pszCode->Text() + uiStart, '\n');
	if ((aszLines.NumElements() > 0) && aszLines.Tail()->Empty())
	{
		aszLines.RemoveTail();
	}

	aiFound.Init();
	Match(&aszLines, &aiFound);

	//Between two lines found, the lines not found are taken to be the ones there now, in order (a pass rewrote them).
	iPrevious = -1;
	uiGapStart = 0;
	for (ui = 0; ui <= maszLines.NumElements(); ui++)
	{
		iFound = (ui < maszLines.NumElements()) ? *aiFound.Get(ui) : (int32)aszLines.NumElements();
		if (iFound < 0)
		{
			continue;
		}
		iNew = iPrevious + 1;
		for (uiGap = uiGapStart; (uiGap < ui) && (iNew < iFound); uiGap++, iNew++)
		{
			aiFound.Set(uiGap, &iNew);
		}
		iPrevious = iFound;
		uiGapStart = ui + 1;
	}

	aiMarker.Init();
	iMarker = -1;
	for (ui = 0; ui <= aszLines.NumElements(); ui++)
	{
		aiMarker.Add(&iMarker);
	}
	for (ui = 0; ui < maszMarkers.NumElements(); ui++)
	{
		iFound = (int32)aszLines.NumElements();
		for (iBefore = *maiBefore.Get(ui); iBefore < (int32)maszLines.NumElements(); iBefore++)
		{
			if (*aiFound.Get((size)iBefore) >= 0)
			{
				iFound = *aiFound.Get((size)iBefore);
				break;
			}
		}
		iMarker = (int32)ui;
		aiMarker.Set((size)iFound, &iMarker);
	}

	pszCode->SetLength(uiStart);
	for (ui = 0; ui <= aszLines.NumElements(); ui++)
	{
		iMarker = *aiMarker.Get(ui);
		if (iMarker >= 0)
		{
			pszCode->Append(*maszMarkers.Get((size)iMarker));
			pszCode->AppendNewLine();
		}
		if (ui < aszLines.NumElements())
		{
			pszCode->Append(*aszLines.Get(ui));
			pszCode->AppendNewLine();
		}
	}
	aiMarker.Kill();
	aiFound.Kill();
	aszLines.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
// Finds the lines from before (maszLines) in pasLines: the most of them in
// order (a diff, Myers' O(ND) one).  paiFound gets per line from before
// where it is in pasLines, or -1.  With too many differences the lines are
// found one after another from the start.
//
//////////////////////////////////////////////////////////////////////////
void CIRSourceMarkers::Match(CArrayChars* pasLines, CArrayTemplate<int32>* paiFound)
{
	CArrayTemplate<int32>	aiV;			//Per diagonal k (x - y), offset by iOffset: the furthest x reached.
	CArrayTemplate<int32>	aiTrace;		//aiV before each step d, diagonals -d - 1 to d + 1.
	int32					iN;
	int32					iM;
	int32					iOffset;
	int32					iD;
	int32					iK;
	int32					iX;
	int32					iY;
	int32					iPreviousK;
	int32					iPreviousX;
	int32					iPreviousY;
	int32					iTraceStart;
	int32					iNone;
	bool					bDone;
	int32					i;

	iN = (int32)maszLines.NumElements();
	iM = (int32)pasLines->NumElements();
	iNone = -1;
	for (i = 0; i < iN; i++)
	{
		paiFound->Add(&iNone);
	}

	iOffset = iN + iM + 2;
	aiV.Init();
	aiV.GrowToAtLeastNumElements((size)(2 * iOffset + 1), true, 0);
	aiTrace.Init();
	bDone = false;
	for (iD = 0; (iD <= iN + iM) && (iD <= IR_SOURCE_MARKERS_MAX_DIFFERENCES) && !bDone; iD++)
	{
		for (iK = -iD - 1; iK <= iD + 1; iK++)
		{
			aiTrace.Add(aiV.Get((size)(iK + iOffset)));
		}
		for (iK = -iD; iK <= iD; iK += 2)
		{
			if ((iK == -iD) || ((iK != iD) && (*aiV.Get((size)(iK - 1 + iOffset)) < *aiV.Get((size)(iK + 1 + iOffset)))))
			{
				iX = *aiV.Get((size)(iK + 1 + iOffset));
			}
			else
			{
				iX = *aiV.Get((size)(iK - 1 + iOffset)) + 1;
			}
			iY = iX - iK;
			while ((iX < iN) && (iY < iM) && maszLines.Get((size)iX)->Equals(pasLines->Get((size)iY)))
			{
				iX++;
				iY++;
			}
			aiV.Set((size)(iK + iOffset), &iX);
			if ((iX >= iN) && (iY >= iM))
			{
				bDone = true;
				break;
			}
		}
	}

	if (!bDone)
	{
		//Too different: each line from before is the next line equal to it.
		iY = 0;
		for (iX = 0; iX < iN; iX++)
		{
			for (i = iY; i < iM; i++)
			{
				if (maszLines.Get((size)iX)->Equals(pasLines->Get((size)i)))
				{
					paiFound->Set((size)iX, &i);
					iY = i + 1;
					break;
				}
			}
		}
		aiTrace.Kill();
		aiV.Kill();
		return;
	}

	//Back from the end through each step's aiV: the diagonal runs are the lines found.
	iX = iN;
	iY = iM;
	for (iD = iD - 1; iD >= 0; iD--)
	{
		iTraceStart = iD * (iD + 2);	//The steps before d stored 2d' + 3 diagonals each.
		iK = iX - iY;
		if ((iK == -iD) || ((iK != iD) && (*aiTrace.Get((size)(iTraceStart + iK - 1 + iD + 1)) < *aiTrace.Get((size)(iTraceStart + iK + 1 + iD + 1)))))
		{
			iPreviousK = iK + 1;
		}
		else
		{
			iPreviousK = iK - 1;
		}
		iPreviousX = *aiTrace.Get((size)(iTraceStart + iPreviousK + iD + 1));
		iPreviousY = iPreviousX - iPreviousK;
		while ((iX > iPreviousX) && (iY > iPreviousY))
		{
			iX--;
			iY--;
			paiFound->Set((size)iX, &iY);
		}
		if (iD > 0)
		{
			iX = iPreviousX;
			iY = iPreviousY;
		}
	}
	aiTrace.Kill();
	aiV.Kill();
}
