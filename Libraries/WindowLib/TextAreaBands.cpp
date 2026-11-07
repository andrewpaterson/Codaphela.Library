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
#include "TextAreaLayout.h"
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


//////////////////////////////////////////////////////////////////////////
//
// A band for each of pcLayout's rows that shows, iTop to iBottom: from
// its top to the next row's, the last to iEnd (the bottom of the text
// area).  Each has the row's height, its text and the styles its text
// changes to (pcDocument's) mixed in.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::AddRows(CTextAreaLayout* pcLayout, CTextDocument* pcDocument, int32 iTop, int32 iBottom, int32 iEnd)
{
	size				uiNumRows;
	size				uiFirstRow;
	size				uiRow;
	size				uiBand;
	size				uiFirstBand;
	size				uiFirstByte;
	size				uiLastByte;
	size				uiStart;
	size				uiEnd;
	size				uiPosition;
	STextLayoutLine*	psLine;
	STextLayoutLine*	psNext;
	STextStyle*			psStyles;
	STextStyle*			psStyle;
	STextStyle*			psLast;
	char*				szText;

	uiNumRows = pcLayout->NumLayoutLines();
	uiFirstRow = pcLayout->FindRowAt(iTop);
	uiFirstBand = NumBands();
	for (uiRow = uiFirstRow; uiRow < uiNumRows; uiRow++)
	{
		psLine = pcLayout->GetLayoutLine(uiRow);
		if (psLine->iTop >= iBottom)
		{
			break;
		}
		psNext = (uiRow + 1 < uiNumRows) ? pcLayout->GetLayoutLine(uiRow + 1) : NULL;
		Add(psLine->iTop, psNext ? psNext->iTop : iEnd);
	}

	//Each row's height, text and the styles its text changes to.
	uiFirstByte = pcLayout->GetRowStart(uiFirstRow);
	uiRow = uiFirstRow + (NumBands() - uiFirstBand);
	uiLastByte = (uiRow < uiNumRows) ? pcLayout->GetRowStart(uiRow) : pcDocument->Length();
	psStyles = pcLayout->MakeStyles(uiFirstByte, uiLastByte);
	szText = pcDocument->Text();
	uiRow = uiFirstRow;
	for (uiBand = uiFirstBand; uiBand < NumBands(); uiBand++)
	{
		psLine = pcLayout->GetLayoutLine(uiRow);
		Mix(uiBand, &psLine->iHeight, sizeof(int32));
		Mix(uiBand, &psLine->iBaseline, sizeof(int32));
		uiStart = pcLayout->GetRowStart(uiRow);
		uiEnd = (uiRow + 1 < uiNumRows) ? pcLayout->GetRowStart(uiRow + 1) : pcDocument->Length();
		Mix(uiBand, &szText[uiStart], uiEnd - uiStart);
		psLast = NULL;
		for (uiPosition = uiStart; uiPosition < uiEnd; uiPosition++)
		{
			psStyle = psStyles ? &psStyles[uiPosition - uiFirstByte] : pcLayout->GetStyle();
			if ((psLast == NULL) || !psStyle->Equals(psLast))
			{
				MixStyle(uiBand, uiPosition - uiStart, psStyle);
				psLast = psStyle;
			}
		}
		uiRow++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The style the row of band uiBand is in from uiOffset, as it is drawn.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::MixStyle(size uiBand, size uiOffset, STextStyle* psStyle)
{
	uint8	auiStyle[16];
	uint32	uiOffset32;

	uiOffset32 = (uint32)uiOffset;
	memcpy(&auiStyle[0], &uiOffset32, 4);
	memcpy(&auiStyle[4], &psStyle->fPixelSize, 4);
	memcpy(&auiStyle[8], &psStyle->sColour, 4);
	auiStyle[12] = (uint8)((psStyle->bBold ? 1 : 0) | (psStyle->bItalic ? 2 : 0));
	auiStyle[13] = psStyle->GetDecorations();
	auiStyle[14] = (uint8)psStyle->ePosition;
	auiStyle[15] = 0;
	Mix(uiBand, auiStyle, sizeof(auiStyle));
}


//////////////////////////////////////////////////////////////////////////
//
// Each of the map's shapes that shows, iTop to iBottom, into the bands it
// crosses: what it is, where and in what colour.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaBands::MixShapes(Ptr<CGeometryMap> pMap, int32 iTop, int32 iBottom)
{
	size				i;
	SGeometryCommand*	psCommand;
	int32				aiShape[8];
	float64				afShape[6];

	for (i = 0; i < pMap->NumCommands(); i++)
	{
		psCommand = pMap->GetCommand(i);
		if ((psCommand->cBounds.GetBottom() <= iTop) || (psCommand->cBounds.GetTop() >= iBottom))
		{
			continue;
		}
		aiShape[0] = (int32)psCommand->eType;
		aiShape[1] = psCommand->cBounds.GetLeft();
		aiShape[2] = psCommand->cBounds.GetTop();
		aiShape[3] = psCommand->cBounds.GetRight();
		aiShape[4] = psCommand->cBounds.GetBottom();
		aiShape[5] = (int32)psCommand->sColour;
		aiShape[6] = (int32)psCommand->uiNumPoints;
		aiShape[7] = (int32)psCommand->eCap;
		afShape[0] = psCommand->sStart.x;
		afShape[1] = psCommand->sStart.y;
		afShape[2] = psCommand->sEnd.x;
		afShape[3] = psCommand->sEnd.y;
		afShape[4] = psCommand->fRadius;
		afShape[5] = psCommand->fThickness;
		MixAll(psCommand->cBounds.GetTop(), psCommand->cBounds.GetBottom(), aiShape, sizeof(aiShape));
		MixAll(psCommand->cBounds.GetTop(), psCommand->cBounds.GetBottom(), afShape, sizeof(afShape));
		MixAll(psCommand->cBounds.GetTop(), psCommand->cBounds.GetBottom(), &psCommand->pcSprite, sizeof(CImageCel*));
	}
}

