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

** ------------------------------------------------------------------------ **/
#ifndef __TRUE_TYPE_FONT_H__
#define __TRUE_TYPE_FONT_H__
#include "BaseLib/Chars.h"
#include "TrueTypeOutline.h"


#define TRUE_TYPE_NAME_FAMILY		1
#define TRUE_TYPE_NAME_SUBFAMILY	2
#define TRUE_TYPE_NAME_FULL_NAME	4


//The horizontal metrics and bounding box of a glyph in font units, y up.
struct STrueTypeGlyphMetrics
{
	int		iAdvanceWidth;
	int		iLeftSideBearing;
	int		iXMin;
	int		iYMin;
	int		iXMax;
	int		iYMax;
};


//Reads glyph outlines and metrics from TrueType (glyf) fonts.  Simple and composite glyphs are read; hinting
//instructions are ignored.  Characters are mapped to glyphs with cmap format 4 or 12.  Variable fonts give their
//default instance.  The whole file is held in memory and every read is bounds checked.
class CTrueTypeFont
{
protected:
	uint8*		mpuiData;
	lsize		muiSize;

	lsize		muiHeadOffset;
	lsize		muiLocaOffset;
	lsize		muiLocaLength;
	lsize		muiGlyfOffset;
	lsize		muiGlyfLength;
	lsize		muiHmtxOffset;
	lsize		muiHmtxLength;
	lsize		muiNameOffset;
	lsize		muiNameLength;
	lsize		muiCmapOffset;  //The chosen subtable, not the table.
	lsize		muiCmapLength;
	int			miCmapFormat;

	int			miUnitsPerEm;
	int			miXMin;
	int			miYMin;
	int			miXMax;
	int			miYMax;
	bool		mbLongLoca;
	int			miNumGlyphs;
	int			miAscender;
	int			miDescender;
	int			miLineGap;
	int			miNumHMetrics;

	//From the optional post and OS/2 tables, in font units.  Positive is up from the baseline except the subscript
	//offset, which is down.
	bool		mbHasPost;
	int			miUnderlinePosition;	//The top of the underline.
	int			miUnderlineThickness;
	bool		mbHasOS2;
	int			miStrikeoutPosition;	//The top of the strikeout.
	int			miStrikeoutSize;
	int			miSubscriptYSize;
	int			miSubscriptYOffset;
	int			miSuperscriptYSize;
	int			miSuperscriptYOffset;

public:
	void		Init(void);
	void		Kill(void);

	bool		Load(char* szFilename);
	bool		Load(void* pvData, lsize uiSize);

	int			NumGlyphs(void);
	int			GetUnitsPerEm(void);
	int			GetAscender(void);
	int			GetDescender(void);
	int			GetLineGap(void);
	void		GetBounds(int* piXMin, int* piYMin, int* piXMax, int* piYMax);
	bool		GetName(int iNameID, CChars* psz);

	bool		HasPost(void);
	int			GetUnderlinePosition(void);
	int			GetUnderlineThickness(void);
	bool		HasOS2(void);
	int			GetStrikeoutPosition(void);
	int			GetStrikeoutSize(void);
	int			GetSubscriptYSize(void);
	int			GetSubscriptYOffset(void);
	int			GetSuperscriptYSize(void);
	int			GetSuperscriptYOffset(void);

	int			GetGlyphIndex(uint32 uiCodePoint);
	bool		GetGlyphMetrics(int iGlyph, STrueTypeGlyphMetrics* psMetrics);
	bool		GetGlyphOutline(int iGlyph, CTrueTypeOutline* pcOutline);

protected:
	bool		Parse(void);
	void		ParsePost(void);
	void		ParseOS2(void);
	bool		FindTable(const char* szTag, lsize* puiOffset, lsize* puiLength);
	bool		ChooseCmap(lsize uiCmapOffset, lsize uiCmapLength);
	int			GetGlyphIndexFormat4(uint32 uiCodePoint);
	int			GetGlyphIndexFormat12(uint32 uiCodePoint);
	bool		GetGlyphData(int iGlyph, lsize* puiOffset, lsize* puiLength);
	bool		ReadSimpleGlyph(lsize uiOffset, lsize uiLength, size iNumContours, CTrueTypeOutline* pcOutline);
	bool		ReadCompositeGlyph(lsize uiOffset, lsize uiLength, CTrueTypeOutline* pcOutline, int iDepth);
	bool		ReadGlyph(int iGlyph, CTrueTypeOutline* pcOutline, int iDepth);

	bool		ReadUint8(lsize uiOffset, uint8* pui);
	bool		ReadUint16(lsize uiOffset, uint16* pui);
	bool		ReadInt16(lsize uiOffset, int16* pi);
	bool		ReadUint32(lsize uiOffset, uint32* pui);
	bool		IsInFile(lsize uiOffset, lsize uiLength);
};


#endif // __TRUE_TYPE_FONT_H__
