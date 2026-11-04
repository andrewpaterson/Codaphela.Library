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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::Free(void)
{
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The font (and the colour of text no span covers) to write in.
//
//////////////////////////////////////////////////////////////////////////
void CTileTextWriter::SetColours(Ptr<CFontColours> pColours, ARGB32 sColour)
{
	mpColours = pColours;
	msColour = sColour;
	mpFont = NULL;
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
// of pauiLineRows goes in the row after the line before.  False if any of
// it was cut off.
//
//////////////////////////////////////////////////////////////////////////
bool CTileTextWriter::Write(const char* szText, CArrayTileTextSpan* pasSpans, CArrayTileTextLineRows* pauiLineRows)
{
	size	uiPosition;
	size	uiEnd;
	size	uiTextLength;
	size	y;
	size	uiLine;
	size	uiRows;
	bool	bFitted;

	mpLayer->Clear();

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
			if (uiEnd > uiPosition)
			{
				bFitted = false;
			}
			if (uiEnd < uiTextLength)
			{
				bFitted = false;
			}
			break;
		}

		bFitted &= WriteRow(y, &szText[uiPosition], uiEnd - uiPosition, uiPosition, pasSpans);
		if (uiEnd >= uiTextLength)
		{
			break;
		}
		uiPosition = uiEnd + 1;
	}
	return bFitted;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces row y's tiles with the uiLength bytes of szLine.  uiLineStart is
// where the line starts in the text pasSpans colours.  False if any of it
// was past the layer's right.
//
//////////////////////////////////////////////////////////////////////////
bool CTileTextWriter::WriteRow(size y, const char* szLine, size uiLength, size uiLineStart, CArrayTileTextSpan* pasSpans)
{
	size			i;
	size			x;
	size			uiColumns;
	size			uiCharacter;
	size			uiSpan;
	size			uiNumSpans;
	size			uiPosition;
	STileTextSpan*	psSpan;
	ARGB32			sColour;
	Ptr<CGlyph>		pGlyph;
	bool			bFitted;
	char			c;

	if (y >= (size)mpLayer->GetLayerSizeY())
	{
		return uiLength == 0;
	}

	mpLayer->ClearRow(y);

	uiColumns = (size)mpLayer->GetLayerSizeX();
	uiNumSpans = pasSpans ? pasSpans->NumElements() : 0;
	uiSpan = FindSpan(pasSpans, uiLineStart);
	bFitted = true;
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
			while ((uiSpan < uiNumSpans) && (pasSpans->Get(uiSpan)->uiStart + pasSpans->Get(uiSpan)->uiLength <= uiPosition))
			{
				uiSpan++;
			}
			sColour = msColour;
			if (uiSpan < uiNumSpans)
			{
				psSpan = pasSpans->Get(uiSpan);
				if (psSpan->uiStart <= uiPosition)
				{
					sColour = psSpan->sColour;
				}
			}

			pGlyph = GetGlyph(GetFont(sColour), &szLine[i], uiCharacter);
			if (pGlyph.IsNotNull())
			{
				mpLayer->SetTile(x, y, pGlyph->GetCel());
			}
		}
		x = TileTextColumns(&szLine[i], uiCharacter, x, muiTabWidth);
		i += uiCharacter;
	}
	return bFitted;
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
