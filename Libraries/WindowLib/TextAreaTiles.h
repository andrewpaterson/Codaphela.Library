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
#ifndef __TEXT_AREA_TILES_H__
#define __TEXT_AREA_TILES_H__
#include "StandardLib/Object.h"
#include "SupportLib/FontColours.h"
#include "SupportLib/TextStyle.h"
#include "SupportLib/TextLayout.h"
#include "SupportLib/MapFontDraw.h"
#include "SupportLib/TileMap.h"
#include "SupportLib/TileLayerCel.h"
#include "SupportLib/TileTextWriter.h"
#include "TextLineGaps.h"


typedef CArrayTemplate<size>	CArrayTextAreaStopStart;
typedef CArrayTemplate<size>	CArrayTextAreaLineStart;


//Where the tiles laid out so far end (CTextAreaTiles::AddStops).
struct STextTileCursor
{
	size	uiCellWidth;
	size	uiCellHeight;
	int32	iAscent;
	size	uiColumn;  //Of the next stop, on its row.
	size	uiColumns;  //The widest row so far.
	size	uiRow;  //The row of the next stop: the line it is on, as tiles aren't wrapped.
	size	uiTileRow;  //The row of tiles the row is on: lines' gaps are whole rows of tiles.
};


//A text area's text laid out in a fixed width font as tiles, as CTextLayout lays text out as sprites: each character
//written into a cell of a tile layer, in the colours of its styles, and the rows and stops worked out from the columns
//of the characters (a tab to the next tab stop, every other character a column).  A line's gap is the fewest empty rows
//of tiles at least as high as it.  It reads the text area's text, stop starts, line starts, gaps and style, which it
//doesn't hold.
class CTextAreaTiles : public CObject
{
CONSTRUCTABLE(CTextAreaTiles);
DESTRUCTABLE(CTextAreaTiles);
protected:
	CChars*						mpszText;
	CArrayTextAreaStopStart*	mpauiStopStarts;  //The byte each stop is before, and the end.
	CArrayTextAreaLineStart*	mpauiLineStarts;
	CTextLineGaps*				mpcGaps;
	STextStyle*					mpsStyle;  //The text area's: text in it is in the font's own colour.
	int32						miTabWidth;  //In spaces.
	Ptr<CTileMap>				mpTileMap;
	Ptr<CTileLayerCel>			mpTileLayer;
	Ptr<CTileTextWriter>		mpTileWriter;
	CArrayTextLayoutLine		masLines;
	CArrayTextLayoutStop		masStops;

public:
	void				Init(CChars* pszText, CArrayTextAreaStopStart* pauiStopStarts, CArrayTextAreaLineStart* pauiLineStarts, CTextLineGaps* pcGaps, STextStyle* psStyle);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				Read(Ptr<CFontColours> pColours, int32 iTabWidth, STextStyle* psStyles, SInt32Vec2* psTextSize);
	size				GetAppendLine(void);
	void				Append(Ptr<CFontColours> pColours, int32 iTabWidth, STextStyle* psStyles, SInt32Vec2* psTextSize);
	void				Decorate(Ptr<CMapFontDraw> pFontDraw, Ptr<CFontDescription> pDescription, STextStyle* psStyles);
	void				ClearTiles(void);

	Ptr<CTileMap>		GetMap(void);
	size				NumLines(void);
	size				NumStops(void);
	STextLayoutLine*	GetLine(size uiLine);
	STextLayoutStop*	GetStop(size uiStop);

protected:
	void				MakeSpans(STextStyle* psStyles, size uiFrom, CArrayTileTextSpan* pasSpans);
	void				StartCursor(Ptr<CFont> pWhite, STextTileCursor* psCursor);
	void				AddLine(STextTileCursor* psCursor, size uiFirstStop);
	size				AddStops(STextTileCursor* psCursor);
	size				GetGapRows(size uiLine, size uiCellHeight);
	void				DecorateStretch(Ptr<CMapFontDraw> pFontDraw, Ptr<CFontDescription> pDescription, uint8 uiDecoration, STextStyle* psStyles, size uiRow, size uiFirstStop, size uiEndStop);
	STextStyle*			GetStyle(STextStyle* psStyles, size uiStop);
};


#endif // __TEXT_AREA_TILES_H__

