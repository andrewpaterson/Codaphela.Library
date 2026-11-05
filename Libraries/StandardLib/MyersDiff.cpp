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
#include "MyersDiff.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDiffRange::Init(lsize uiStart1, lsize uiEnd1, lsize uiStart2, lsize uiEnd2)
{
	this->uiStart1 = uiStart1;
	this->uiEnd1 = uiEnd1;
	this->uiStart2 = uiStart2;
	this->uiEnd2 = uiEnd2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize SDiffRange::Length1(void)
{
	return uiEnd1 - uiStart1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize SDiffRange::Length2(void)
{
	return uiEnd2 - uiStart2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMyersDiff::Init(void)
{
	mpaui1 = NULL;
	mpaui2 = NULL;
	mpacChanges = NULL;
	muiFirstChange = 0;
	macPending.Init();
	maiForward.Init();
	maiBackward.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMyersDiff::Kill(void)
{
	maiBackward.Kill();
	maiForward.Kill();
	macPending.Kill();
	mpacChanges = NULL;
	mpaui2 = NULL;
	mpaui1 = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the changes that turn paui1 into paui2 to pacChanges.
//
//////////////////////////////////////////////////////////////////////////
void CMyersDiff::Diff(lsize* paui1, lsize uiLength1, lsize* paui2, lsize uiLength2, CArrayDiffRange* pacChanges)
{
	SDiffRange	sRange;

	mpaui1 = paui1;
	mpaui2 = paui2;
	mpacChanges = pacChanges;
	muiFirstChange = pacChanges->NumElements();

	macPending.ReInit();
	sRange.Init(0, uiLength1, 0, uiLength2);
	macPending.Push(&sRange);

	//The second half of a split is pushed before the first so the first is compared first and the changes come out in
	//order.
	while (macPending.Pop(&sRange))
	{
		DiffRange(&sRange);
	}

	mpacChanges = NULL;
	mpaui2 = NULL;
	mpaui1 = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMyersDiff::DiffRange(SDiffRange* psRange)
{
	lsize		uiStart1;
	lsize		uiEnd1;
	lsize		uiStart2;
	lsize		uiEnd2;
	lsize		uiSplit1;
	lsize		uiSplit2;
	SDiffRange	sHalf;

	uiStart1 = psRange->uiStart1;
	uiEnd1 = psRange->uiEnd1;
	uiStart2 = psRange->uiStart2;
	uiEnd2 = psRange->uiEnd2;

	while ((uiStart1 < uiEnd1) && (uiStart2 < uiEnd2) && (mpaui1[uiStart1] == mpaui2[uiStart2]))
	{
		uiStart1++;
		uiStart2++;
	}
	while ((uiStart1 < uiEnd1) && (uiStart2 < uiEnd2) && (mpaui1[uiEnd1 - 1] == mpaui2[uiEnd2 - 1]))
	{
		uiEnd1--;
		uiEnd2--;
	}

	if ((uiStart1 == uiEnd1) && (uiStart2 == uiEnd2))
	{
		return;
	}
	if ((uiStart1 == uiEnd1) || (uiStart2 == uiEnd2))
	{
		AddChange(uiStart1, uiEnd1, uiStart2, uiEnd2);
		return;
	}

	sHalf.Init(uiStart1, uiEnd1, uiStart2, uiEnd2);
	if (Bisect(&sHalf, &uiSplit1, &uiSplit2))
	{
		sHalf.Init(uiSplit1, uiEnd1, uiSplit2, uiEnd2);
		macPending.Push(&sHalf);
		sHalf.Init(uiStart1, uiSplit1, uiStart2, uiSplit2);
		macPending.Push(&sHalf);
	}
	else
	{
		AddChange(uiStart1, uiEnd1, uiStart2, uiEnd2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the middle snake of the range by searching forward from its start
// and backward from its end until the paths overlap.  The split is where
// the forward path meets the backward one, as absolute indices.  Both sides
// of the range must be non-empty and have different first and last
// elements (DiffRange trims them).
//
//////////////////////////////////////////////////////////////////////////
bool CMyersDiff::Bisect(SDiffRange* psRange, lsize* puiSplit1, lsize* puiSplit2)
{
	lsize*	paui1;
	lsize*	paui2;
	int64	iLength1;
	int64	iLength2;
	int64	iMaxD;
	int64	iOffset;
	int64	iVLength;
	int64	iDelta;
	bool	bFront;
	int64	iK1Start;
	int64	iK1End;
	int64	iK2Start;
	int64	iK2End;
	int64	iD;
	int64	iK1;
	int64	iK2;
	int64	iK1Offset;
	int64	iK2Offset;
	int64	iX1;
	int64	iY1;
	int64	iX2;
	int64	iY2;
	int64*	paiForward;
	int64*	paiBackward;
	int64	i;

	paui1 = &mpaui1[psRange->uiStart1];
	paui2 = &mpaui2[psRange->uiStart2];
	iLength1 = (int64)psRange->Length1();
	iLength2 = (int64)psRange->Length2();

	iMaxD = (iLength1 + iLength2 + 1) / 2;
	iOffset = iMaxD;
	iVLength = 2 * iMaxD + 2;

	maiForward.Resize((lsize)iVLength);
	maiBackward.Resize((lsize)iVLength);
	paiForward = maiForward.GetData();
	paiBackward = maiBackward.GetData();
	for (i = 0; i < iVLength; i++)
	{
		paiForward[i] = -1;
		paiBackward[i] = -1;
	}
	paiForward[iOffset + 1] = 0;
	paiBackward[iOffset + 1] = 0;

	//If the total length is odd the forward path finds the overlap, otherwise the backward one does.
	iDelta = iLength1 - iLength2;
	bFront = (iDelta % 2) != 0;

	//Diagonals that have run off the edge of the grid are skipped.
	iK1Start = 0;
	iK1End = 0;
	iK2Start = 0;
	iK2End = 0;
	for (iD = 0; iD < iMaxD; iD++)
	{
		for (iK1 = -iD + iK1Start; iK1 <= iD - iK1End; iK1 += 2)
		{
			iK1Offset = iOffset + iK1;
			if ((iK1 == -iD) || ((iK1 != iD) && (paiForward[iK1Offset - 1] < paiForward[iK1Offset + 1])))
			{
				iX1 = paiForward[iK1Offset + 1];
			}
			else
			{
				iX1 = paiForward[iK1Offset - 1] + 1;
			}
			iY1 = iX1 - iK1;
			while ((iX1 < iLength1) && (iY1 < iLength2) && (paui1[iX1] == paui2[iY1]))
			{
				iX1++;
				iY1++;
			}
			paiForward[iK1Offset] = iX1;

			if (iX1 > iLength1)
			{
				iK1End += 2;
			}
			else if (iY1 > iLength2)
			{
				iK1Start += 2;
			}
			else if (bFront)
			{
				iK2Offset = iOffset + iDelta - iK1;
				if ((iK2Offset >= 0) && (iK2Offset < iVLength) && (paiBackward[iK2Offset] != -1))
				{
					iX2 = iLength1 - paiBackward[iK2Offset];
					if (iX1 >= iX2)
					{
						*puiSplit1 = psRange->uiStart1 + (lsize)iX1;
						*puiSplit2 = psRange->uiStart2 + (lsize)iY1;
						return true;
					}
				}
			}
		}

		for (iK2 = -iD + iK2Start; iK2 <= iD - iK2End; iK2 += 2)
		{
			iK2Offset = iOffset + iK2;
			if ((iK2 == -iD) || ((iK2 != iD) && (paiBackward[iK2Offset - 1] < paiBackward[iK2Offset + 1])))
			{
				iX2 = paiBackward[iK2Offset + 1];
			}
			else
			{
				iX2 = paiBackward[iK2Offset - 1] + 1;
			}
			iY2 = iX2 - iK2;
			while ((iX2 < iLength1) && (iY2 < iLength2) && (paui1[iLength1 - iX2 - 1] == paui2[iLength2 - iY2 - 1]))
			{
				iX2++;
				iY2++;
			}
			paiBackward[iK2Offset] = iX2;

			if (iX2 > iLength1)
			{
				iK2End += 2;
			}
			else if (iY2 > iLength2)
			{
				iK2Start += 2;
			}
			else if (!bFront)
			{
				iK1Offset = iOffset + iDelta - iK2;
				if ((iK1Offset >= 0) && (iK1Offset < iVLength) && (paiForward[iK1Offset] != -1))
				{
					iX1 = paiForward[iK1Offset];
					iY1 = iOffset + iX1 - iK1Offset;
					iX2 = iLength1 - iX2;
					if (iX1 >= iX2)
					{
						*puiSplit1 = psRange->uiStart1 + (lsize)iX1;
						*puiSplit2 = psRange->uiStart2 + (lsize)iY1;
						return true;
					}
				}
			}
		}
	}

	//No common elements at all.
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMyersDiff::AddChange(lsize uiStart1, lsize uiEnd1, lsize uiStart2, lsize uiEnd2)
{
	SDiffRange*	psTail;
	SDiffRange*	psChange;

	psTail = NULL;
	if (mpacChanges->NumElements() > muiFirstChange)
	{
		psTail = mpacChanges->Tail();
	}
	if ((psTail != NULL) && (psTail->uiEnd1 == uiStart1) && (psTail->uiEnd2 == uiStart2))
	{
		psTail->uiEnd1 = uiEnd1;
		psTail->uiEnd2 = uiEnd2;
	}
	else
	{
		psChange = mpacChanges->Add();
		psChange->Init(uiStart1, uiEnd1, uiStart2, uiEnd2);
	}
}
