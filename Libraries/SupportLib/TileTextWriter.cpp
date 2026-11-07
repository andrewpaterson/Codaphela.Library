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
#include "BaseLib/UTF8.h"
#include "TileTextWriter.h"


//////////////////////////////////////////////////////////////////////////
//
// The bytes of the UTF-8 character at szText (1 for a byte that doesn't
// start one, or one cut short).
//
//////////////////////////////////////////////////////////////////////////
size TileTextCharacterLength(const char* szText, size uiRemaining)
{
	uint8	uiLead;
	size	uiLength;
	size	i;

	uiLead = (uint8)szText[0];
	if (uiLead < 0x80)
	{
		return 1;
	}
	else if ((uiLead & 0xE0) == 0xC0)
	{
		uiLength = 2;
	}
	else if ((uiLead & 0xF0) == 0xE0)
	{
		uiLength = 3;
	}
	else if ((uiLead & 0xF8) == 0xF0)
	{
		uiLength = 4;
	}
	else
	{
		return 1;
	}

	if (uiLength > uiRemaining)
	{
		return 1;
	}
	for (i = 1; i < uiLength; i++)
	{
		if (((uint8)szText[i] & 0xC0) != 0x80)
		{
			return 1;
		}
	}
	return uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
// The code point of the uiLength byte UTF-8 character at szCharacter (as
// TileTextCharacterLength measures it), or TILE_TEXT_NO_CODE_POINT if it
// isn't one: a byte that doesn't start a character, a character in more
// bytes than it needs, a surrogate or one past U+10FFFF.
//
//////////////////////////////////////////////////////////////////////////
uint32 TileTextCodePoint(const char* szCharacter, size uiLength)
{
	uint32	uiLead;
	uint32	uiCodePoint;

	uiLead = (uint8)szCharacter[0];
	if (uiLength == 1)
	{
		return (uiLead < 0x80) ? uiLead : TILE_TEXT_NO_CODE_POINT;
	}
	else if (uiLength == 2)
	{
		uiCodePoint = ((uiLead & 0x1F) << 6) | ((uint32)(uint8)szCharacter[1] & 0x3F);
		return (uiCodePoint >= 0x80) ? uiCodePoint : TILE_TEXT_NO_CODE_POINT;
	}
	else if (uiLength == 3)
	{
		uiCodePoint = ((uiLead & 0x0F) << 12) | (((uint32)(uint8)szCharacter[1] & 0x3F) << 6) | ((uint32)(uint8)szCharacter[2] & 0x3F);
		if ((uiCodePoint < 0x800) || ((uiCodePoint >= 0xD800) && (uiCodePoint <= 0xDFFF)))
		{
			return TILE_TEXT_NO_CODE_POINT;
		}
		return uiCodePoint;
	}
	else if (uiLength == 4)
	{
		uiCodePoint = ((uiLead & 0x07) << 18) | (((uint32)(uint8)szCharacter[1] & 0x3F) << 12) | (((uint32)(uint8)szCharacter[2] & 0x3F) << 6) | ((uint32)(uint8)szCharacter[3] & 0x3F);
		return ((uiCodePoint >= 0x10000) && (uiCodePoint <= 0x10FFFF)) ? uiCodePoint : TILE_TEXT_NO_CODE_POINT;
	}
	return TILE_TEXT_NO_CODE_POINT;
}


//////////////////////////////////////////////////////////////////////////
//
// The column after the uiLength bytes of szText, starting at uiColumn: a
// tab goes to the next tab stop, carriage returns take no columns and
// every other character takes one.  It stops at a new line or a
// terminator.
//
//////////////////////////////////////////////////////////////////////////
size TileTextColumns(const char* szText, size uiLength, size uiColumn, size uiTabWidth)
{
	size	i;
	char	c;

	if (uiTabWidth == 0)
	{
		uiTabWidth = 1;
	}

	i = 0;
	while (i < uiLength)
	{
		c = szText[i];
		if ((c == '\n') || (c == '\0'))
		{
			break;
		}
		else if (c == '\t')
		{
			uiColumn = (uiColumn / uiTabWidth + 1) * uiTabWidth;
			i++;
		}
		else if (c == '\r')
		{
			i++;
		}
		else
		{
			uiColumn++;
			i += TileTextCharacterLength(&szText[i], uiLength - i);
		}
	}
	return uiColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::Init(Ptr<CTileLayerCel> pLayer, Ptr<CFontColours> pColours, ARGB32 sColour, size uiTabWidth)
{
	PreInit();

	mpLayer = pLayer;
	mpColours = pColours;
	msColour = sColour;
	muiTabWidth = (uiTabWidth > 0) ? uiTabWidth : 1;
	mpFont = NULL;
	msFontColour = 0;
	masPages.Init();
	mcPageIndices.Init(true);
	mpuiPage = NULL;
	msPageColour = 0;
	muiPage = 0;
	muiCelVersion = mpLayer.IsNotNull() ? mpLayer->GetCelVersion() : 0;
	mbClearCels = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::Class(void)
{
	M_Pointer(mpLayer);
	M_Pointer(mpColours);
	U_UInt32(msColour);
	U_Size(muiTabWidth);
	M_Pointer(mpFont);
	U_UInt32(msFontColour);
	U_Unknown(CArrayTileTextPage, masPages);
	U_Unknown(CMapTileTextPageIndex, mcPageIndices);
	U_Pointer(mpuiPage);
	U_UInt32(msPageColour);
	U_UInt32(muiPage);
	U_Size(muiCelVersion);
	U_Bool(mbClearCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::Free(void)
{
	mcPageIndices.Kill();
	masPages.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileTextWriter::Save(CObjectWriter* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileTextWriter::Load(CObjectReader* pcFile)
{
	//The tile values aren't saved: they are looked up again as the text is written.
	masPages.Init();
	mcPageIndices.Init(true);
	mpuiPage = NULL;
	muiCelVersion = 0;
	mbClearCels = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The font (and the colour of text no span covers) to write in.  A new
// font forgets the tile values looked up in the old, and the next Write
// clears the layer's cels: they are the old font's glyphs.
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::SetColours(Ptr<CFontColours> pColours, ARGB32 sColour)
{
	if (mpColours != pColours)
	{
		mpColours = pColours;
		mpFont = NULL;
		ForgetTiles();
		mbClearCels = true;
	}
	msColour = sColour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::SetTabWidth(size uiTabWidth)
{
	muiTabWidth = (uiTabWidth > 0) ? uiTabWidth : 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The font is fixed width and the layer's cells are its glyphs' size: its
// advance wide and its line high.
//
//////////////////////////////////////////////////////////////////////////
bool CTileTextWriter::IsValid(void)
{
	Ptr<CFont>	pWhite;
	SSizeVec2	sCelSize;

	if (mpLayer.IsNull() || mpColours.IsNull())
	{
		return false;
	}

	pWhite = mpColours->GetWhite();
	if (pWhite.IsNull() || !pWhite->IsFixedWidth())
	{
		return false;
	}

	sCelSize = mpLayer->GetCelSize();
	return (sCelSize.x == pWhite->GetAverageWidth()) && (sCelSize.y == (size)pWhite->GetLineHeight());
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the layer's tiles with szText, its first line in the top row
// and each line in the row after, or with pauiLineRows in the row it gives
// for the line (rows the text skips are left empty).  A line past the end
// of pauiLineRows goes in the row after the line before; the rows must go
// down, and a line whose row doesn't is cut off.  False if any of it was
// cut off.
//
//////////////////////////////////////////////////////////////////////////
bool CTileTextWriter::Write(const char* szText, CArrayTileTextSpan* pasSpans, CArrayTileTextLineRows* pauiLineRows)
{
	size					uiPosition;
	size					uiEnd;
	size					uiTextLength;
	size					y;
	size					uiLine;
	size					uiRows;
	size					uiColumns;
	bool					bFitted;
	CArrayTileLayerTile		auiTiles;
	CArrayTileTextLineRows	auiRowLengths;

	if (mbClearCels)
	{
		mpLayer->Clear();
		mbClearCels = false;
	}
	CheckCelVersion();

	//Every row's tiles, the rows the text skips empty, given to the layer at once.
	auiTiles.Init();
	auiRowLengths.Init();
	uiTextLength = StrLen(szText);
	uiRows = (size)mpLayer->GetLayerSizeY();
	bFitted = true;
	uiPosition = 0;
	y = 0;
	for (uiLine = 0; ; uiLine++)
	{
		if (pauiLineRows && (uiLine < pauiLineRows->NumElements()))
		{
			y = *pauiLineRows->Get(uiLine);
		}
		else if (uiLine > 0)
		{
			y++;
		}
		uiEnd = uiPosition;
		while ((uiEnd < uiTextLength) && (szText[uiEnd] != '\n'))
		{
			uiEnd++;
		}

		if (y >= uiRows)
		{
			//The text has lines past the bottom of the layer.
			if ((uiEnd > uiPosition) || (uiEnd < uiTextLength))
			{
				bFitted = false;
			}
			break;
		}

		if (y < auiRowLengths.NumElements())
		{
			bFitted = false;
		}
		else
		{
			while (auiRowLengths.NumElements() < y)
			{
				*auiRowLengths.Add() = 0;
			}
			bFitted &= WriteRowTiles(&szText[uiPosition], uiEnd - uiPosition, uiPosition, pasSpans, &auiTiles, &uiColumns);
			*auiRowLengths.Add() = uiColumns;
		}
		if (uiEnd >= uiTextLength)
		{
			break;
		}
		uiPosition = uiEnd + 1;
	}
	while (auiRowLengths.NumElements() < uiRows)
	{
		*auiRowLengths.Add() = 0;
	}

	bFitted &= mpLayer->SetRowTiles(0, uiRows, auiTiles.GetData(), auiRowLengths.GetData());
	auiRowLengths.Kill();
	auiTiles.Kill();
	return bFitted;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the tiles of row y with the uiLength bytes of szLine, which
// are from uiLineStart in the text pasSpans colours.  False if any of it
// was cut off.
//
//////////////////////////////////////////////////////////////////////////
bool CTileTextWriter::WriteRow(size y, const char* szLine, size uiLength, size uiLineStart, CArrayTileTextSpan* pasSpans)
{
	CArrayTileLayerTile	auiTiles;
	size				uiColumns;
	bool				bFitted;

	if (y >= (size)mpLayer->GetLayerSizeY())
	{
		return uiLength == 0;
	}

	CheckCelVersion();
	auiTiles.Init();
	bFitted = WriteRowTiles(szLine, uiLength, uiLineStart, pasSpans, &auiTiles, &uiColumns);
	bFitted &= mpLayer->SetRowTiles(y, 1, auiTiles.GetData(), &uiColumns);
	auiTiles.Kill();
	return bFitted;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the tile values of the uiLength bytes of szLine (from uiLineStart
// in the text pasSpans colours) to pauiTiles, from column 0 to the last
// character's: puiColumns is set to how many.  It stops at a new line or
// a terminator.  False if the line was cut off at the layer's right.
//
//////////////////////////////////////////////////////////////////////////
bool CTileTextWriter::WriteRowTiles(const char* szLine, size uiLength, size uiLineStart, CArrayTileTextSpan* pasSpans, CArrayTileLayerTile* pauiTiles, size* puiColumns)
{
	size			i;
	size			x;
	size			uiColumns;
	size			uiWritten;
	size			uiCharacter;
	size			uiSpan;
	size			uiNumSpans;
	size			uiPosition;
	STileTextSpan*	psSpans;
	ARGB32			sColour;
	uint32			uiTile;
	bool			bFitted;
	char			c;

	uiColumns = (size)mpLayer->GetLayerSizeX();
	uiNumSpans = pasSpans ? pasSpans->NumElements() : 0;
	psSpans = pasSpans ? pasSpans->GetData() : NULL;
	uiSpan = FindSpan(pasSpans, uiLineStart);
	bFitted = true;
	uiWritten = 0;
	x = 0;
	i = 0;
	while (i < uiLength)
	{
		c = szLine[i];
		if ((c == '\n') || (c == '\0'))
		{
			break;
		}

		uiCharacter = TileTextCharacterLength(&szLine[i], uiLength - i);
		if ((c == '\r') || (c == '\t'))
		{
			x = TileTextColumns(&szLine[i], uiCharacter, x, muiTabWidth);
			i += uiCharacter;
			continue;
		}
		if (c != ' ')
		{
			if (x >= uiColumns)
			{
				bFitted = false;
				break;
			}

			uiPosition = uiLineStart + i;
			while ((uiSpan < uiNumSpans) && (psSpans[uiSpan].uiStart + psSpans[uiSpan].uiLength <= uiPosition))
			{
				uiSpan++;
			}
			sColour = ((uiSpan < uiNumSpans) && (psSpans[uiSpan].uiStart <= uiPosition)) ? psSpans[uiSpan].sColour : msColour;

			uiTile = GetTile(sColour, &szLine[i], uiCharacter);
			if (uiTile != TILE_TEXT_NO_GLYPH)
			{
				while (uiWritten < x)
				{
					*pauiTiles->Add() = 0;
					uiWritten++;
				}
				*pauiTiles->Add() = uiTile;
				uiWritten++;
			}
		}
		x++;
		i += uiCharacter;
	}
	*puiColumns = uiWritten;
	return bFitted;
}


//////////////////////////////////////////////////////////////////////////
//
// The tile value of the uiLength byte character at szCharacter in
// sColour: from its page if it has been written before, else looked up and
// kept there.  A character that isn't a code point isn't kept.
//
//////////////////////////////////////////////////////////////////////////
uint32 CTileTextWriter::GetTile(ARGB32 sColour, const char* szCharacter, size uiLength)
{
	uint32	uiCodePoint;
	uint32	uiPage;
	uint32*	puiTile;

	uiCodePoint = TileTextCodePoint(szCharacter, uiLength);
	if (uiCodePoint == TILE_TEXT_NO_CODE_POINT)
	{
		return LookUpTile(sColour, szCharacter, uiLength);
	}

	uiPage = uiCodePoint / TILE_TEXT_PAGE_SIZE;
	if ((mpuiPage == NULL) || (msPageColour != sColour) || (muiPage != uiPage))
	{
		FindPage(sColour, uiPage);
	}
	puiTile = &mpuiPage[uiCodePoint % TILE_TEXT_PAGE_SIZE];
	if (*puiTile == 0)
	{
		*puiTile = LookUpTile(sColour, szCharacter, uiLength);
	}
	return *puiTile;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes uiPage of sColour the page looked in, adding it if it hasn't
// been.
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::FindPage(ARGB32 sColour, uint32 uiPage)
{
	int64			iKey;
	size*			puiIndex;
	size			uiIndex;
	STileTextPage*	psPage;

	iKey = (int64)(((uint64)sColour << 32) | uiPage);
	puiIndex = mcPageIndices.Get(iKey);
	if (puiIndex)
	{
		uiIndex = *puiIndex;
	}
	else
	{
		uiIndex = masPages.NumElements();
		psPage = masPages.Add();
		memset(psPage, 0, sizeof(STileTextPage));
		mcPageIndices.Put(iKey, &uiIndex);
	}

	//Adding a page may move them all.
	mpuiPage = masPages.Get(uiIndex)->auiTiles;
	msPageColour = sColour;
	muiPage = uiPage;
}


//////////////////////////////////////////////////////////////////////////
//
// The tile value of the uiLength byte character at szCharacter in
// sColour, from its glyph's cel: TILE_TEXT_NO_GLYPH if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
uint32 CTileTextWriter::LookUpTile(ARGB32 sColour, const char* szCharacter, size uiLength)
{
	Ptr<CGlyph>	pGlyph;
	uint32		uiTile;

	pGlyph = GetGlyph(GetFont(sColour), szCharacter, uiLength);
	if (pGlyph.IsNull())
	{
		return TILE_TEXT_NO_GLYPH;
	}
	uiTile = mpLayer->AddCel(pGlyph->GetCel());
	return (uiTile != 0) ? uiTile : TILE_TEXT_NO_GLYPH;
}


//////////////////////////////////////////////////////////////////////////
//
// The tile values looked up so far are no longer good.
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::ForgetTiles(void)
{
	masPages.ReInit();
	mcPageIndices.ReInit();
	mpuiPage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The tile values are forgotten if the layer has forgotten its cels since
// they were looked up.
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::CheckCelVersion(void)
{
	if (muiCelVersion != mpLayer->GetCelVersion())
	{
		ForgetTiles();
		muiCelVersion = mpLayer->GetCelVersion();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first span that ends after uiPosition (the number of spans if none
// does).
//
//////////////////////////////////////////////////////////////////////////
size CTileTextWriter::FindSpan(CArrayTileTextSpan* pasSpans, size uiPosition)
{
	size			uiLow;
	size			uiHigh;
	size			uiMiddle;
	STileTextSpan*	psSpan;

	if (pasSpans == NULL)
	{
		return 0;
	}

	uiLow = 0;
	uiHigh = pasSpans->NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = uiLow + (uiHigh - uiLow) / 2;
		psSpan = pasSpans->Get(uiMiddle);
		if (psSpan->uiStart + psSpan->uiLength <= uiPosition)
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
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTileTextWriter::GetFont(ARGB32 sColour)
{
	if (mpFont.IsNull() || (msFontColour != sColour))
	{
		mpFont = mpColours->GetFont(sColour);
		msFontColour = sColour;
	}
	return mpFont;
}


//////////////////////////////////////////////////////////////////////////
//
// The glyph of the uiLength bytes at szCharacter, or '?' if the font
// hasn't got it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CTileTextWriter::GetGlyph(Ptr<CFont> pFont, const char* szCharacter, size uiLength)
{
	Ptr<CGlyph>	pGlyph;
	CUTF8		cUTF8;

	if (pFont.IsNull())
	{
		return NULL;
	}

	//The font's glyphs are found by code point, not by UTF-8 bytes.
	cUTF8.Init((uint8*)szCharacter, uiLength);
	pGlyph = pFont->GetGlyph(&cUTF8);
	cUTF8.Kill();
	if (pGlyph.IsNull())
	{
		pGlyph = pFont->GetGlyph((uint8*)"?", 1);
	}
	return pGlyph;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTileTextWriter::GetTabWidth(void)
{
	return muiTabWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CTileTextWriter::GetColour(void)
{
	return msColour;
}
