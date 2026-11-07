/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "TextAreaBands.h"


#define TEXT_AREA_BAND_HASH_START	2166136261u	//FNV-1a's offset basis.
#define TEXT_AREA_BAND_HASH_PRIME	16777619u


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::Init(void)
{
	masBands.Init();
	mbComplete = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::Kill(void)
{
	masBands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// No bands: nothing is known of what was drawn.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::Clear(void)
{
	masBands.ReInit();
	mbComplete = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::Complete(void) { mbComplete = true; }  //The bands added are all of them.
bool CTextAreaBands::IsComplete(void) { return mbComplete; }
size CTextAreaBands::NumBands(void) { return masBands.NumElements(); }
STextAreaBand* CTextAreaBands::Get(size uiIndex) { return masBands.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// The next band, below those before it.  Its hash starts empty.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::Add(int32 iTop, int32 iBottom)
{
	STextAreaBand*	psBand;

	psBand = masBands.Add();
	psBand->iTop = iTop;
	psBand->iBottom = iBottom;
	psBand->uiHash = TEXT_AREA_BAND_HASH_START;
}


//////////////////////////////////////////////////////////////////////////
//
// The band y is in, or the first below it; NumBands if there is none.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaBands::FindBand(int32 y)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = masBands.NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (masBands.Get(uiMiddle)->iBottom <= y)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// Something drawn in band uiBand, described by pvData, goes into its
// hash.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::Mix(size uiBand, const void* pvData, size uiBytes)
{
	STextAreaBand*	psBand;
	const uint8*	puiData;
	size			i;
	uint32			uiHash;

	psBand = masBands.Get(uiBand);
	puiData = (const uint8*)pvData;
	uiHash = psBand->uiHash;
	for (i = 0; i < uiBytes; i++)
	{
		uiHash = (uiHash ^ puiData[i]) * TEXT_AREA_BAND_HASH_PRIME;
	}
	psBand->uiHash = uiHash;
}


//////////////////////////////////////////////////////////////////////////
//
// Something drawn from iTop to iBottom goes into the hash of every band
// it crosses.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::MixAll(int32 iTop, int32 iBottom, const void* pvData, size uiBytes)
{
	size	uiBand;

	for (uiBand = FindBand(iTop); uiBand < masBands.NumElements(); uiBand++)
	{
		if (masBands.Get(uiBand)->iTop >= iBottom)
		{
			break;
		}
		Mix(uiBand, pvData, uiBytes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The parts (x top, y bottom) of the bands that differ from those
// pcBefore was of: a band at the same place, the same size, with the
// same hash, is the same.  Any other, before or now, changed.  Parts next
// to each other are joined.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::Compare(CTextAreaBands* pcBefore, CArrayInt32Vec2* pasChanged)
{
	size			uiBefore;
	size			uiNow;
	STextAreaBand*	psBefore;
	STextAreaBand*	psNow;

	uiBefore = 0;
	uiNow = 0;
	while ((uiBefore < pcBefore->NumBands()) || (uiNow < NumBands()))
	{
		psBefore = (uiBefore < pcBefore->NumBands()) ? pcBefore->Get(uiBefore) : NULL;
		psNow = (uiNow < NumBands()) ? Get(uiNow) : NULL;
		if ((psNow == NULL) || ((psBefore != NULL) && (psBefore->iTop < psNow->iTop)))
		{
			AddChanged(pasChanged, psBefore->iTop, psBefore->iBottom);
			uiBefore++;
		}
		else if ((psBefore == NULL) || (psNow->iTop < psBefore->iTop))
		{
			AddChanged(pasChanged, psNow->iTop, psNow->iBottom);
			uiNow++;
		}
		else
		{
			if ((psBefore->iBottom != psNow->iBottom) || (psBefore->uiHash != psNow->uiHash))
			{
				AddChanged(pasChanged, psBefore->iTop, psBefore->iBottom);
				AddChanged(pasChanged, psNow->iTop, psNow->iBottom);
			}
			uiBefore++;
			uiNow++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Parts are added in order of their tops, so one that overlaps or
// touches the last is joined to it.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::AddChanged(CArrayInt32Vec2* pasChanged, int32 iTop, int32 iBottom)
{
	SInt32Vec2*	psLast;

	if (iBottom <= iTop)
	{
		return;
	}
	if (pasChanged->NumElements() > 0)
	{
		psLast = pasChanged->Tail();
		if (iTop <= psLast->y)
		{
			if (iBottom > psLast->y)
			{
				psLast->y = iBottom;
			}
			return;
		}
	}
	pasChanged->Add()->Init(iTop, iBottom);
}


//////////////////////////////////////////////////////////////////////////
//
// The bands become pcOther's, and pcOther's are cleared.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::Take(CTextAreaBands* pcOther)
{
	size	i;

	masBands.ReInit();
	for (i = 0; i < pcOther->NumBands(); i++)
	{
		*masBands.Add() = *pcOther->Get(i);
	}
	mbComplete = pcOther->mbComplete;
	pcOther->Clear();
}

