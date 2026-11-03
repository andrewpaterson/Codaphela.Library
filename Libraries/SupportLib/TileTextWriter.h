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


//Writes UTF-8 text into a tile layer with a fixed width font: each character is a tile (the glyph's cel, which is
//the cell's size), from the font of its colour.  Spaces are empty tiles, a tab moves to the next tab stop, a carriage
//return is ignored and a character the font hasn't got is a '?'.  What is past the layer's right or bottom is cut off.
class CTileTextWriter : public CObject
{
CONSTRUCTABLE(CTileTextWriter);
DESTRUCTABLE(CTileTextWriter);
protected:
	Ptr<CTileLayerCel>	mpLayer;
	Ptr<CFontColours>	mpColours;
	ARGB32				msColour;		//Of the text no span covers.
	size				muiTabWidth;	//In columns.

	Ptr<CFont>			mpFont;			//The font of msFontColour, the last asked for.
	ARGB32				msFontColour;

public:
	void		Init(Ptr<CTileLayerCel> pLayer, Ptr<CFontColours> pColours, ARGB32 sColour, size uiTabWidth = 4);
	void		Class(void) override;
	void 		Free(void) override;

	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	void		SetColours(Ptr<CFontColours> pColours, ARGB32 sColour);
	void		SetTabWidth(size uiTabWidth);
	bool		IsValid(void);
	bool		Write(const char* szText, CArrayTileTextSpan* pasSpans = NULL);
	bool		WriteRow(size y, const char* szLine, size uiLength, size uiLineStart = 0, CArrayTileTextSpan* pasSpans = NULL);

	size		GetTabWidth(void);
	ARGB32		GetColour(void);

protected:
	Ptr<CFont>	GetFont(ARGB32 sColour);
	Ptr<CGlyph>	GetGlyph(Ptr<CFont> pFont, const char* szCharacter, size uiLength);
	size		FindSpan(CArrayTileTextSpan* pasSpans, size uiPosition);
};


size TileTextCharacterLength(const char* szText, size uiRemaining);
size TileTextColumns(const char* szText, size uiLength, size uiColumn, size uiTabWidth);


#endif // __TILE_TEXT_WRITER_H__
