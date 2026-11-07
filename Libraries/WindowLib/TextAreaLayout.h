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
#ifndef __TEXT_AREA_LAYOUT_H__
#define __TEXT_AREA_LAYOUT_H__
#include "BaseLib/TextDocument.h"
#include "StandardLib/Object.h"
#include "SupportLib/FontFamily.h"
#include "SupportLib/TextStyle.h"
#include "SupportLib/Text.h"
#include "SupportLib/SimpleTextReader.h"
#include "SupportLib/TextLayout.h"
#include "SupportLib/SpriteMap.h"
#include "SupportLib/GeometryMap.h"
#include "SupportLib/MapFontDraw.h"
#include "TextLineGaps.h"
#include "TextSpans.h"
#include "TextAreaTiles.h"


//A text area's text laid out in rows: read in the fonts of a font family, in its style with its spans over it, as
//sprites (a CTextLayout, wrapped to a width when word wrap is on) or, for a fixed width font, as tiles (CTextAreaTiles),
//with the gaps asked for below its lines.  Its sprites, tiles and the lines drawn along the text are in maps for the
//text area to draw.  Setting what it is laid out from only notes that it changed, so many changes are laid out once:
//ReadText and LayoutText do the work, UpdateText when something changed.  The rows are what a CTextEditing moves in:
//positions are bytes of the text, x and y pixels from its top left.
class CTextAreaLayout : public CObject
{
CONSTRUCTABLE(CTextAreaLayout);
DESTRUCTABLE(CTextAreaLayout);
protected:
	CTextDocument*			mpcDocument;  //Not held: the text area's.
	Ptr<CFontFamily>		mpFamily;
	STextStyle				msStyle;
	CTextSpans				mcSpans;
	CTextLineGaps			mcGaps;  //The empty gaps below lines (SetLineGap).
	bool					mbTextDirty;  //The spans, style or gaps changed since the text was last read.
	bool					mbSizeDirty;  //Reading the text again can change its size.
	bool					mbWordWrap;
	int32					miWidth;  //The text area's, which lines are wrapped to.
	size					muiTabWidth;  //In spaces.
	SInt32Vec2				msTextSize;
	CArrayTextAreaStopStart	mauiStopStarts;  //The byte each of the layout's stops is before.  Carriage returns have none.

	Ptr<CText>				mpText;
	bool					mbTextStale;  //Text was appended after mpText was read (AppendSprites): read it again before it is laid out whole.
	Ptr<CTextLayout>		mpTextLayout;
	Ptr<CTextLayout>		mpAppendLayout;  //Lays out text appended, from the start of its first line (AppendSprites).  NULL until then.
	Ptr<CSpriteMap>			mpSpriteMap;
	Ptr<CGeometryMap>		mpGeometryMap;  //The lines drawn along the text, over it.
	Ptr<CMapFontDraw>		mpFontDraw;

	//A fixed width font can be drawn as tiles instead of sprites: each character in a cell of a tile layer, the lines
	//and stops worked out from the columns instead of by mpTextLayout.
	bool					mbTiledText;  //Tiles were asked for; they are only used with a fixed width font and no word wrap.
	bool					mbTiled;  //The text was last read as tiles.
	Ptr<CTextAreaTiles>		mpTiles;

public:
	void					Init(CTextDocument* pcDocument, Ptr<CFontFamily> pFamily);
	void					Class(void) override;
	void					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	//What the text is laid out from.
	void					SetFontFamily(Ptr<CFontFamily> pFamily);
	Ptr<CFontFamily>		GetFontFamily(void);
	Ptr<CFontColours>		GetFontColours(void);
	Ptr<CFont>				GetFont(void);
	void					SetStyle(STextStyle* psStyle);
	STextStyle*				GetStyle(void);
	void					SetColour(ARGB32 sColour);
	bool					AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride);
	void					ClearSpans(void);
	size					NumSpans(void);
	void					GetStyleAt(size uiPosition, STextStyle* psStyle);
	CTextLineGaps*			GetGaps(void);
	void					GapsChanged(void);
	bool					SetWordWrap(bool bWordWrap);
	bool					IsWordWrap(void);
	bool					SetTiledText(bool bTiledText);
	bool					IsTiledText(void);
	bool					IsTiled(void);
	void					SetTabWidth(size uiTabWidth);
	int32					GetTabPixels(void);
	void					SetWidth(int32 iWidth);
	int32					GetWrapWidth(void);
	bool					IsTextDirty(void);
	bool					IsWrapWidthChanged(void);

	//Laying it out.
	void					TextSet(void);
	void					Append(const char* szText, ARGB32 sColour);
	void					ReadText(void);
	void					LayoutText(void);
	bool					UpdateText(void);
	bool					UpdateSize(void);
	SInt32Vec2				GetTextSize(void);
	Ptr<CSpriteMap>			GetSpriteMap(void);
	Ptr<CGeometryMap>		GetGeometryMap(void);
	Ptr<CTileMap>			GetTileMap(void);

	//The rows and stops the text is laid out in, by mpTextLayout or as tiles.
	bool					HasRows(void);
	size					NumLayoutLines(void);
	size					NumLayoutStops(void);
	STextLayoutLine*		GetLayoutLine(size uiLine);
	STextLayoutStop*		GetLayoutStop(size uiStop);
	size					FindStop(size uiPosition);
	bool					IsRowWrapped(size uiRow);
	size					FindRow(size uiPosition, bool bRowEnd);
	size					GetRowStart(size uiRow);
	size					GetRowEnd(size uiRow, bool* pbRowEnd);
	int32					GetRowRight(size uiRow);
	size					FindRowAt(int32 y);
	size					FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd);
	int32					GetPositionX(size uiPosition, bool bRowEnd);
	size					FindLastRow(size uiLine);
	bool					GetLineRows(size uiLine, int32* piTop, int32* piBottom);
	int32					GetNewLineWidth(void);

protected:
	STextStyle*				MakeStyles(size uiFrom = 0);
	void					MakeFontChanges(CArrayTextFontChange* pasChanges, Ptr<CFont> pDefaultFont, int16 iDefaultBaselineShift, size uiFrom = 0);
	bool					HasDecorations(void);
	void					FindStopStarts(size uiFrom);
	bool					UseTiles(void);
	void					ReadTiles(void);
	bool					CanAppendTiles(void);
	bool					CanAppendSprites(void);
	void					AppendSprites(size uiFrom);
	Ptr<CText>				ReadTextFrom(size uiFrom);
	void					AppendTiles(size uiFrom);
	void					LayoutTiles(void);
};


#endif // __TEXT_AREA_LAYOUT_H__

