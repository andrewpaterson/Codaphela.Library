/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __TILE_TEXT_WRITER_H__
#define __TILE_TEXT_WRITER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/MapLongTemplate.h"
#include "StandardLib/Object.h"
#include "FontColours.h"
#include "TileLayerCel.h"


//A colour for the bytes uiStart to uiStart + uiLength of the text.  The spans given to CTileTextWriter are sorted by
//uiStart and don't overlap; bytes no span covers are in the writer's colour.
struct STileTextSpan
{
	size	uiStart;
	size	uiLength;
	ARGB32	sColour;
};


typedef CArrayTemplate<STileTextSpan>	CArrayTileTextSpan;
typedef CArrayTemplate<size>			CArrayTileTextLineRows;


#define TILE_TEXT_PAGE_SIZE			256
#define TILE_TEXT_NO_GLYPH			0xFFFFFFFF	//A character with nothing to show (no font, or no glyph and no '?').
#define TILE_TEXT_NO_CODE_POINT		0xFFFFFFFF


//The tile values (CTileLayerCel::AddCel's) of the characters of one page, the 256 code points from a multiple of 256,
//in one colour: 0 until the character is first written, TILE_TEXT_NO_GLYPH when it has nothing to show.
struct STileTextPage
{
	uint32	auiTiles[TILE_TEXT_PAGE_SIZE];
};


typedef CArrayTemplate<STileTextPage>	CArrayTileTextPage;
typedef CMapLongTemplate<size>			CMapTileTextPageIndex;


//Writes UTF-8 text into a tile layer with a fixed width font: each character is a tile (the glyph's cel, which is
//the cell's size), from the font of its colour.  Spaces are empty tiles, a tab moves to the next tab stop, a carriage
//return is ignored and a character the font hasn't got is a '?'.  What is past the layer's right or bottom is cut off.
//
//The tile value of each character in each colour is looked up (font, glyph, the layer's cel) the first time it is
//written and kept in pages by code point, so writing it again is reading it back.  They are forgotten when the font
//changes or the layer forgets its cels.  Rows are given to the layer whole (CTileLayerCel::SetRowTiles).
class CTileTextWriter : public CObject
{
CONSTRUCTABLE(CTileTextWriter);
DESTRUCTABLE(CTileTextWriter);
protected:
	Ptr<CTileLayerCel>		mpLayer;
	Ptr<CFontColours>		mpColours;
	ARGB32					msColour;		//Of the text no span covers.
	size					muiTabWidth;	//In columns.

	Ptr<CFont>				mpFont;			//The font of msFontColour, the last asked for.
	ARGB32					msFontColour;

	CArrayTileTextPage		masPages;		//The tile values of the characters written, by colour and page.
	CMapTileTextPageIndex	mcPageIndices;	//The index in masPages of a colour's page: the colour << 32 | the page.
	uint32*					mpuiPage;		//The tile values of the page last looked in, NULL for none.
	ARGB32					msPageColour;	//Its colour and page.
	uint32					muiPage;
	size					muiCelVersion;	//The layer's (CTileLayerCel::GetCelVersion) when the pages were made.
	bool					mbClearCels;	//The font changed: the next Write clears the layer's cels.

public:
	void		Init(Ptr<CTileLayerCel> pLayer, Ptr<CFontColours> pColours, ARGB32 sColour, size uiTabWidth = 4);
	void		Class(void) override;
	void 		Free(void) override;

	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	void		SetColours(Ptr<CFontColours> pColours, ARGB32 sColour);
	void		SetTabWidth(size uiTabWidth);
	bool		IsValid(void);
	bool		Write(const char* szText, CArrayTileTextSpan* pasSpans = NULL, CArrayTileTextLineRows* pauiLineRows = NULL);
	bool		WriteRow(size y, const char* szLine, size uiLength, size uiLineStart = 0, CArrayTileTextSpan* pasSpans = NULL);

	size		GetTabWidth(void);
	ARGB32		GetColour(void);

protected:
	bool		WriteRowTiles(const char* szLine, size uiLength, size uiLineStart, CArrayTileTextSpan* pasSpans, CArrayTileLayerTile* pauiTiles, size* puiColumns);
	uint32		GetTile(ARGB32 sColour, const char* szCharacter, size uiLength);
	void		FindPage(ARGB32 sColour, uint32 uiPage);
	uint32		LookUpTile(ARGB32 sColour, const char* szCharacter, size uiLength);
	void		ForgetTiles(void);
	void		CheckCelVersion(void);
	Ptr<CFont>	GetFont(ARGB32 sColour);
	Ptr<CGlyph>	GetGlyph(Ptr<CFont> pFont, const char* szCharacter, size uiLength);
	size		FindSpan(CArrayTileTextSpan* pasSpans, size uiPosition);
};


size	TileTextCharacterLength(const char* szText, size uiRemaining);
uint32	TileTextCodePoint(const char* szCharacter, size uiLength);
size	TileTextColumns(const char* szText, size uiLength, size uiColumn, size uiTabWidth);


#endif // __TILE_TEXT_WRITER_H__
