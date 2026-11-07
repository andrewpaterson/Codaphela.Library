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
#include "TextAreaTiles.h"


//////////////////////////////////////////////////////////////////////////
//
// The text area's text, its stop starts (one for each character but
// carriage returns, then one at the end), line starts, gaps and style,
// which the tiles are made from.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::Init(CTextDocument* pcDocument, CArrayTextAreaStopStart* pauiStopStarts, CTextLineGaps* pcGaps, STextStyle* psStyle)
{
	PreInit();

	mpcDocument = pcDocument;
	mpauiStopStarts = pauiStopStarts;
	mpcGaps = pcGaps;
	mpsStyle = psStyle;
	miTabWidth = 4;
	mpTileMap = OMalloc<CTileMap>();
	mpTileLayer = OMalloc<CTileLayerCel>(mpTileMap, "Text", SSizeVec2(0, 0), SSizeVec2(1, 1), 0, SInt32Vec2(0, 0));
	mpTileMap->AddLayer(mpTileLayer);
	mpTileWriter = OMalloc<CTileTextWriter>(mpTileLayer, Ptr<CFontColours>(), mpsStyle->sColour, (size)miTabWidth);
	masLines.Init();
	masStops.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::Class(void)
{
	U_Pointer(mpcDocument);
	U_Pointer(mpauiStopStarts);
	U_Pointer(mpcGaps);
	U_Pointer(mpsStyle);
	U_Int32(miTabWidth);
	M_Pointer(mpTileMap);
	M_Pointer(mpTileLayer);
	M_Pointer(mpTileWriter);
	U_Unknown(CArrayTextLayoutLine, masLines);
	U_Unknown(CArrayTextLayoutStop, masStops);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::Free(void)
{
	masStops.Kill();
	masLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaTiles::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaTiles::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The whole text laid out and written into the tiles, in the colours of
// psStyles (each byte's, NULL for all in the text area's style), tabs
// iTabWidth spaces apart, the cells the size of pColours' characters.
// psTextSize is set to the size of the text.  The stop starts must be
// those of the text.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::Read(Ptr<CFontColours> pColours, int32 iTabWidth, STextStyle* psStyles, SInt32Vec2* psTextSize)
{
	STextTileCursor			sCursor;
	size					uiTileRows;
	size					uiLine;
	CArrayTileTextSpan		asSpans;
	CArrayTileTextLineRows	auiLineRows;

	miTabWidth = iTabWidth;
	StartCursor(pColours->GetWhite(), &sCursor);
	masLines.ReInit();
	masStops.ReInit();
	AddLine(&sCursor, 0);
	uiTileRows = AddStops(&sCursor);
	psTextSize->Init((int32)(sCursor.uiColumns * sCursor.uiCellWidth), (int32)(uiTileRows * sCursor.uiCellHeight));

	mpTileLayer->SetCelSize(SSizeVec2(sCursor.uiCellWidth, sCursor.uiCellHeight));
	mpTileLayer->SetMapSize(SSizeVec2((sCursor.uiColumns > 0) ? sCursor.uiColumns : 1, uiTileRows));
	mpTileWriter->SetColours(pColours, mpsStyle->sColour);
	mpTileWriter->SetTabWidth((size)iTabWidth);

	auiLineRows.Init();
	for (uiLine = 0; uiLine < masLines.NumElements(); uiLine++)
	{
		*auiLineRows.Add() = (size)masLines.Get(uiLine)->iTop / sCursor.uiCellHeight;
	}
	asSpans.Init();
	MakeSpans(psStyles, 0, &asSpans);
	mpTileWriter->Write(mpcDocument->Text(), &asSpans, &auiLineRows);
	asSpans.Kill();
	auiLineRows.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The line text appended to the end goes on: the one the end is on.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaTiles::GetAppendLine(void)
{
	return masStops.Tail()->uiLine;
}


//////////////////////////////////////////////////////////////////////////
//
// The text appended after what was read: the old end stop is replaced by
// the new characters' and a new end stop, and the rows from the append
// line (GetAppendLine) on are written again in the colours of psStyles
// (each byte's from the start of that line).  psTextSize is the size of
// the text before, and is set to its size after.  The stop starts must be
// those of the text with what was appended.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::Append(Ptr<CFontColours> pColours, int32 iTabWidth, STextStyle* psStyles, SInt32Vec2* psTextSize)
{
	STextTileCursor		sCursor;
	STextLayoutStop*	psStop;
	size				uiFirstRow;
	size				uiTileRows;
	size				uiRow;
	size				uiLineStart;
	size				uiLineEnd;
	size				uiLength;
	char*				szText;
	CArrayTileTextSpan	asSpans;

	//The old end stop goes: the text goes on from where it was.
	miTabWidth = iTabWidth;
	StartCursor(pColours->GetWhite(), &sCursor);
	psStop = masStops.Tail();
	uiFirstRow = psStop->uiLine;
	sCursor.uiRow = uiFirstRow;
	sCursor.uiColumn = (sCursor.uiCellWidth > 0) ? (size)psStop->x / sCursor.uiCellWidth : 0;
	sCursor.uiColumns = (sCursor.uiCellWidth > 0) ? (size)psTextSize->x / sCursor.uiCellWidth : 0;
	sCursor.uiTileRow = (size)masLines.Get(uiFirstRow)->iTop / sCursor.uiCellHeight;
	masLines.Get(uiFirstRow)->uiNumStops--;
	masStops.RemoveTail();

	uiTileRows = AddStops(&sCursor);
	psTextSize->Init((int32)(sCursor.uiColumns * sCursor.uiCellWidth), (int32)(uiTileRows * sCursor.uiCellHeight));
	mpTileLayer->SetMapSize(SSizeVec2((sCursor.uiColumns > 0) ? sCursor.uiColumns : 1, uiTileRows));

	//Each line from the old last one written on its row, in the colours from its start.
	asSpans.Init();
	MakeSpans(psStyles, mpcDocument->GetLineStart(uiFirstRow), &asSpans);
	szText = mpcDocument->Text();
	uiLength = mpcDocument->Length();
	for (uiRow = uiFirstRow; uiRow < mpcDocument->NumLines(); uiRow++)
	{
		uiLineStart = mpcDocument->GetLineStart(uiRow);
		uiLineEnd = (uiRow + 1 < mpcDocument->NumLines()) ? mpcDocument->GetLineStart(uiRow + 1) - 1 : uiLength;
		mpTileWriter->WriteRow((size)masLines.Get(uiRow)->iTop / sCursor.uiCellHeight, &szText[uiLineStart], uiLineEnd - uiLineStart, uiLineStart, &asSpans);
	}
	asSpans.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The tiles written go, when the text is laid out as sprites instead.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::ClearTiles(void)
{
	if (mpTileLayer->NumStoredTiles() > 0)
	{
		mpTileLayer->Clear();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The map the tiles are drawn from.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileMap> CTextAreaTiles::GetMap(void)
{
	return mpTileMap;
}


size CTextAreaTiles::NumLines(void) { return masLines.NumElements(); }
size CTextAreaTiles::NumStops(void) { return masStops.NumElements(); }
STextLayoutLine* CTextAreaTiles::GetLine(size uiLine) { return masLines.Get(uiLine); }
STextLayoutStop* CTextAreaTiles::GetStop(size uiStop) { return masStops.Get(uiStop); }


//////////////////////////////////////////////////////////////////////////
//
// The bytes from uiFrom on coloured differently from the text area's
// style, as sorted spans that don't overlap.  psStyles are those of the
// bytes from uiFrom on (MakeStyles(uiFrom)); NULL makes none.  Only the
// colours of the styles are used: every tile is the size of the text
// area's font.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::MakeSpans(STextStyle* psStyles, size uiFrom, CArrayTileTextSpan* pasSpans)
{
	size			uiRange;
	size			uiPosition;
	size			uiStart;
	STileTextSpan*	psTileSpan;
	ARGB32			sColour;

	if (psStyles == NULL)
	{
		return;
	}

	uiRange = mpcDocument->Length() - uiFrom;
	uiPosition = 0;
	while (uiPosition < uiRange)
	{
		sColour = psStyles[uiPosition].sColour;
		if (sColour == mpsStyle->sColour)
		{
			uiPosition++;
			continue;
		}
		uiStart = uiPosition;
		while ((uiPosition < uiRange) && (psStyles[uiPosition].sColour == sColour))
		{
			uiPosition++;
		}
		psTileSpan = pasSpans->Add();
		psTileSpan->uiStart = uiFrom + uiStart;
		psTileSpan->uiLength = uiPosition - uiStart;
		psTileSpan->sColour = sColour;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The cursor at the start of the text, its cells the size of pWhite's
// characters.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::StartCursor(Ptr<CFont> pWhite, STextTileCursor* psCursor)
{
	psCursor->uiCellWidth = pWhite->GetAverageWidth();
	psCursor->uiCellHeight = (size)pWhite->GetLineHeight();
	psCursor->iAscent = pWhite->GetAscent();
	psCursor->uiColumn = 0;
	psCursor->uiColumns = 0;
	psCursor->uiRow = 0;
	psCursor->uiTileRow = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A row of tiles starting at the cursor's tile row, its first stop
// uiFirstStop.  Its baseline is below the top of the text, as
// CTextLayout's are.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::AddLine(STextTileCursor* psCursor, size uiFirstStop)
{
	STextLayoutLine*	psLine;

	psLine = masLines.Add();
	psLine->iTop = (int32)(psCursor->uiTileRow * psCursor->uiCellHeight);
	psLine->iHeight = (int32)psCursor->uiCellHeight;
	psLine->iBaseline = psLine->iTop + psCursor->iAscent;
	psLine->uiFirstStop = uiFirstStop;
	psLine->uiNumStops = 0;
	psLine->iWidth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A tile stop for each stop start after the tile stops there are, laid
// out from the cursor on: a line feed is the last stop of its row and the
// line's gap is rows of no text below it.  Returns the rows of tiles the
// text needs.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaTiles::AddStops(STextTileCursor* psCursor)
{
	size				ui;
	size				uiNumStops;
	size				uiStart;
	size				uiEnd;
	size				uiNextColumn;
	size				uiLength;
	char*				szText;
	STextLayoutStop*	psStop;

	szText = mpcDocument->Text();
	uiLength = mpcDocument->Length();
	uiNumStops = mpauiStopStarts->NumElements();
	for (ui = masStops.NumElements(); ui < uiNumStops; ui++)
	{
		uiStart = *mpauiStopStarts->Get(ui);
		uiEnd = (ui + 1 < uiNumStops) ? *mpauiStopStarts->Get(ui + 1) : uiLength;

		psStop = masStops.Add();
		psStop->x = (int32)(psCursor->uiColumn * psCursor->uiCellWidth);
		psStop->iWidth = 0;
		psStop->uiLine = psCursor->uiRow;
		masLines.Get(psCursor->uiRow)->uiNumStops++;

		if (uiStart < uiLength)
		{
			if (szText[uiStart] == '\n')
			{
				psCursor->uiTileRow += 1 + GetGapRows(psCursor->uiRow, psCursor->uiCellHeight);
				psCursor->uiRow++;
				psCursor->uiColumn = 0;
				AddLine(psCursor, ui + 1);
			}
			else
			{
				uiNextColumn = TileTextColumns(&szText[uiStart], uiEnd - uiStart, psCursor->uiColumn, (size)miTabWidth);
				psStop->iWidth = (int32)((uiNextColumn - psCursor->uiColumn) * psCursor->uiCellWidth);
				psCursor->uiColumn = uiNextColumn;
			}
		}
		if (psCursor->uiColumn > psCursor->uiColumns)
		{
			psCursor->uiColumns = psCursor->uiColumn;
		}
	}
	return psCursor->uiTileRow + 1 + GetGapRows(psCursor->uiRow, psCursor->uiCellHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// The fewest empty rows of tiles at least as high as line uiLine's gap.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaTiles::GetGapRows(size uiLine, size uiCellHeight)
{
	return ((size)mpcGaps->GetHeight(uiLine) + uiCellHeight - 1) / uiCellHeight;
}


//////////////////////////////////////////////////////////////////////////
//
// The underlines, overlines and strike throughs of the tiles, as
// CTextLayout draws them: each stretch of decorated characters on a row
// is one line, in the colours of its characters, and the spaces and tabs
// at the end of a row aren't decorated.  They are drawn over the tiles
// and aren't kept inside the cells.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::Decorate(Ptr<CMapFontDraw> pFontDraw, Ptr<CFontDescription> pDescription, STextStyle* psStyles)
{
	uint8				auiDecorations[3] = { TD_Underline, TD_Overline, TD_Strikethrough };
	size				uiRow;
	size				uiNumRows;
	STextLayoutLine*	psLine;
	size				uiFirst;
	size				uiEnd;
	size				uiStop;
	size				uiStretch;
	size				uiDecoration;
	uint8				uiFlag;
	char				c;
	bool				bAny;

	//Nothing to draw unless some text is decorated.
	bAny = (mpsStyle->GetDecorations() != 0);
	if (!bAny && psStyles)
	{
		for (uiStop = 0; (uiStop < mpcDocument->Length()) && !bAny; uiStop++)
		{
			bAny = (psStyles[uiStop].GetDecorations() != 0);
		}
	}
	if (!bAny)
	{
		return;
	}

	uiNumRows = masLines.NumElements();
	for (uiRow = 0; uiRow < uiNumRows; uiRow++)
	{
		psLine = masLines.Get(uiRow);
		uiFirst = psLine->uiFirstStop;
		uiEnd = psLine->uiFirstStop + psLine->uiNumStops;

		//The row's line feed or the end of the text, and the spaces and tabs before it, aren't decorated.
		while (uiEnd > uiFirst)
		{
			if (*mpauiStopStarts->Get(uiEnd - 1) >= mpcDocument->Length())
			{
				uiEnd--;
				continue;
			}
			c = mpcDocument->Text()[*mpauiStopStarts->Get(uiEnd - 1)];
			if ((c == '\n') || (c == ' ') || (c == '\t'))
			{
				uiEnd--;
				continue;
			}
			break;
		}

		for (uiDecoration = 0; uiDecoration < 3; uiDecoration++)
		{
			uiFlag = auiDecorations[uiDecoration];
			uiStop = uiFirst;
			while (uiStop < uiEnd)
			{
				if (GetStyle(psStyles, uiStop)->GetDecorations() & uiFlag)
				{
					uiStretch = uiStop;
					while ((uiStop < uiEnd) && (GetStyle(psStyles, uiStop)->GetDecorations() & uiFlag))
					{
						uiStop++;
					}
					DecorateStretch(pFontDraw, pDescription, uiFlag, psStyles, uiRow, uiStretch, uiStop);
				}
				else
				{
					uiStop++;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One decoration along stops uiFirstStop to uiEndStop (exclusive) of row
// uiRow, a rectangle for each run of the same colour.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaTiles::DecorateStretch(Ptr<CMapFontDraw> pFontDraw, Ptr<CFontDescription> pDescription, uint8 uiDecoration, STextStyle* psStyles, size uiRow, size uiFirstStop, size uiEndStop)
{
	STextLayoutLine*		psLine;
	STextLayoutStop*		psFirst;
	STextLayoutStop*		psLast;
	size					uiStop;
	size					uiColourStart;
	int32					y;
	int32					iThickness;
	ARGB32					sColour;

	psLine = masLines.Get(uiRow);
	if (uiDecoration == TD_Underline)
	{
		y = psLine->iBaseline + pDescription->GetUnderlineTop();
		iThickness = pDescription->GetUnderlineThickness();
	}
	else if (uiDecoration == TD_Overline)
	{
		y = psLine->iBaseline - pDescription->GetAscent();
		iThickness = pDescription->GetUnderlineThickness();
	}
	else
	{
		y = psLine->iBaseline + pDescription->GetStrikeoutTop();
		iThickness = pDescription->GetStrikeoutThickness();
	}

	uiStop = uiFirstStop;
	while (uiStop < uiEndStop)
	{
		uiColourStart = uiStop;
		sColour = GetStyle(psStyles, uiStop)->sColour;
		while ((uiStop < uiEndStop) && (GetStyle(psStyles, uiStop)->sColour == sColour))
		{
			uiStop++;
		}
		psFirst = masStops.Get(uiColourStart);
		psLast = masStops.Get(uiStop - 1);
		pFontDraw->Rectangle(psFirst->x, y, psLast->x + psLast->iWidth - psFirst->x, iThickness, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The style of the character at stop uiStop: the style of its first byte.
//
//////////////////////////////////////////////////////////////////////////
STextStyle* CTextAreaTiles::GetStyle(STextStyle* psStyles, size uiStop)
{
	if (psStyles == NULL)
	{
		return mpsStyle;
	}
	return &psStyles[*mpauiStopStarts->Get(uiStop)];
}
