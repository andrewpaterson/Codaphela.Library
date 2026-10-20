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
#include <stdlib.h>
#include <string.h>
#include "BaseLib/NaiveFile.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FixedPoint.h"
#include "TrueTypeFont.h"


#define TRUE_TYPE_HEAD_MAGIC			0x5F0F3CF5

//Simple glyph flags.
#define TRUE_TYPE_ON_CURVE				0x01
#define TRUE_TYPE_X_SHORT				0x02
#define TRUE_TYPE_Y_SHORT				0x04
#define TRUE_TYPE_REPEAT				0x08
#define TRUE_TYPE_X_SAME_OR_POSITIVE	0x10
#define TRUE_TYPE_Y_SAME_OR_POSITIVE	0x20

//Composite glyph flags.
#define TRUE_TYPE_ARGS_ARE_WORDS		0x0001
#define TRUE_TYPE_ARGS_ARE_XY_VALUES	0x0002
#define TRUE_TYPE_HAVE_A_SCALE			0x0008
#define TRUE_TYPE_MORE_COMPONENTS		0x0020
#define TRUE_TYPE_HAVE_AN_X_AND_Y_SCALE	0x0040
#define TRUE_TYPE_HAVE_A_TWO_BY_TWO		0x0080
#define TRUE_TYPE_SCALED_OFFSET			0x0800
#define TRUE_TYPE_UNSCALED_OFFSET		0x1000

//Composite glyphs that nest deeper than this are taken to be corrupt (or recursive).
#define TRUE_TYPE_MAX_COMPOSITE_DEPTH	8

//Composite glyph scales are 2.14 fixed point; 1.0 is 16384.
#define TRUE_TYPE_F2DOT14_ONE			16384


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFont::Init(void)
{
	mpuiData = NULL;
	muiSize = 0;
	muiHeadOffset = 0;
	muiLocaOffset = 0;
	muiLocaLength = 0;
	muiGlyfOffset = 0;
	muiGlyfLength = 0;
	muiHmtxOffset = 0;
	muiHmtxLength = 0;
	muiNameOffset = 0;
	muiNameLength = 0;
	muiCmapOffset = 0;
	muiCmapLength = 0;
	miCmapFormat = 0;
	miUnitsPerEm = 0;
	miXMin = 0;
	miYMin = 0;
	miXMax = 0;
	miYMax = 0;
	mbLongLoca = false;
	miNumGlyphs = 0;
	miAscender = 0;
	miDescender = 0;
	miLineGap = 0;
	miNumHMetrics = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFont::Kill(void)
{
	if (mpuiData)
	{
		free(mpuiData);
		mpuiData = NULL;
	}
	muiSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::Load(char* szFilename)
{
	CNaiveFile	cNaiveFile;
	bool		bResult;

	cNaiveFile.Init();
	if (!cNaiveFile.Read(szFilename))
	{
		cNaiveFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not read [", szFilename, "].", NULL);
		return false;
	}

	bResult = Load(cNaiveFile.Get(), (size)cNaiveFile.Size());
	cNaiveFile.Kill();
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not a supported TrueType font.", NULL);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The data is copied.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::Load(void* pvData, size uiSize)
{
	Kill();
	Init();

	if (uiSize == 0)
	{
		return false;
	}
	mpuiData = (uint8*)malloc(uiSize);
	if (mpuiData == NULL)
	{
		return false;
	}
	memcpy(mpuiData, pvData, uiSize);
	muiSize = uiSize;

	if (!Parse())
	{
		Kill();
		Init();
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::IsInFile(size uiOffset, size uiLength)
{
	return (uiOffset <= muiSize) && (uiLength <= muiSize - uiOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::ReadUint8(size uiOffset, uint8* pui)
{
	if (!IsInFile(uiOffset, 1))
	{
		return false;
	}
	*pui = mpuiData[uiOffset];
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// TrueType fonts are big endian.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::ReadUint16(size uiOffset, uint16* pui)
{
	if (!IsInFile(uiOffset, 2))
	{
		return false;
	}
	*pui = (uint16)((mpuiData[uiOffset] << 8) | mpuiData[uiOffset + 1]);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::ReadInt16(size uiOffset, int16* pi)
{
	uint16	ui;

	if (!ReadUint16(uiOffset, &ui))
	{
		return false;
	}
	*pi = (int16)ui;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::ReadUint32(size uiOffset, uint32* pui)
{
	if (!IsInFile(uiOffset, 4))
	{
		return false;
	}
	*pui = ((uint32)mpuiData[uiOffset] << 24) | ((uint32)mpuiData[uiOffset + 1] << 16) | ((uint32)mpuiData[uiOffset + 2] << 8) | (uint32)mpuiData[uiOffset + 3];
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds a table in the table directory.  The table must lie inside the
// file.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::FindTable(const char* szTag, size* puiOffset, size* puiLength)
{
	uint16	uiNumTables;
	uint16	i;
	size	uiRecord;
	uint32	uiOffset;
	uint32	uiLength;

	if (!ReadUint16(4, &uiNumTables))
	{
		return false;
	}

	for (i = 0; i < uiNumTables; i++)
	{
		uiRecord = 12 + (size)i * 16;
		if (!IsInFile(uiRecord, 16))
		{
			return false;
		}
		if (memcmp(&mpuiData[uiRecord], szTag, 4) == 0)
		{
			if (!ReadUint32(uiRecord + 8, &uiOffset) || !ReadUint32(uiRecord + 12, &uiLength) || !IsInFile(uiOffset, uiLength))
			{
				return false;
			}
			*puiOffset = uiOffset;
			*puiLength = uiLength;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the tables needed to find glyphs and their metrics.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::Parse(void)
{
	uint32	uiVersion;
	uint32	uiMagic;
	uint16	uiUnitsPerEm;
	int16	iXMin;
	int16	iYMin;
	int16	iXMax;
	int16	iYMax;
	int16	iLocaFormat;
	uint16	uiNumGlyphs;
	int16	iAscender;
	int16	iDescender;
	int16	iLineGap;
	uint16	uiNumHMetrics;
	size	uiHeadLength;
	size	uiMaxpOffset;
	size	uiMaxpLength;
	size	uiHheaOffset;
	size	uiHheaLength;
	size	uiCmapOffset;
	size	uiCmapLength;

	//0x00010000 for TrueType outlines, 'true' for old Apple fonts.  'OTTO' (CFF outlines) isn't supported.
	if (!ReadUint32(0, &uiVersion) || ((uiVersion != 0x00010000) && (uiVersion != 0x74727565)))
	{
		return false;
	}

	if (!FindTable("head", &muiHeadOffset, &uiHeadLength) || (uiHeadLength < 54) ||
		!FindTable("maxp", &uiMaxpOffset, &uiMaxpLength) || (uiMaxpLength < 6) ||
		!FindTable("hhea", &uiHheaOffset, &uiHheaLength) || (uiHheaLength < 36) ||
		!FindTable("hmtx", &muiHmtxOffset, &muiHmtxLength) ||
		!FindTable("loca", &muiLocaOffset, &muiLocaLength) ||
		!FindTable("glyf", &muiGlyfOffset, &muiGlyfLength) ||
		!FindTable("cmap", &uiCmapOffset, &uiCmapLength))
	{
		return false;
	}

	if (!ReadUint32(muiHeadOffset + 12, &uiMagic) || (uiMagic != TRUE_TYPE_HEAD_MAGIC))
	{
		return false;
	}
	ReadUint16(muiHeadOffset + 18, &uiUnitsPerEm);
	ReadInt16(muiHeadOffset + 36, &iXMin);
	ReadInt16(muiHeadOffset + 38, &iYMin);
	ReadInt16(muiHeadOffset + 40, &iXMax);
	ReadInt16(muiHeadOffset + 42, &iYMax);
	ReadInt16(muiHeadOffset + 50, &iLocaFormat);
	ReadUint16(uiMaxpOffset + 4, &uiNumGlyphs);
	ReadInt16(uiHheaOffset + 4, &iAscender);
	ReadInt16(uiHheaOffset + 6, &iDescender);
	ReadInt16(uiHheaOffset + 8, &iLineGap);
	ReadUint16(uiHheaOffset + 34, &uiNumHMetrics);

	if ((uiUnitsPerEm == 0) || (iLocaFormat < 0) || (iLocaFormat > 1) || (uiNumGlyphs == 0) || (uiNumHMetrics == 0) || (uiNumHMetrics > uiNumGlyphs))
	{
		return false;
	}

	miUnitsPerEm = uiUnitsPerEm;
	miXMin = iXMin;
	miYMin = iYMin;
	miXMax = iXMax;
	miYMax = iYMax;
	mbLongLoca = iLocaFormat == 1;
	miNumGlyphs = uiNumGlyphs;
	miAscender = iAscender;
	miDescender = iDescender;
	miLineGap = iLineGap;
	miNumHMetrics = uiNumHMetrics;

	//Every glyph needs a loca entry, and one more to give the length of the last glyph.
	if (muiLocaLength < ((size)miNumGlyphs + 1) * (mbLongLoca ? 4 : 2))
	{
		return false;
	}
	//Every glyph needs a left side bearing; the first ones have an advance width too.
	if (muiHmtxLength < (size)miNumHMetrics * 4 + ((size)miNumGlyphs - miNumHMetrics) * 2)
	{
		return false;
	}

	if (!FindTable("name", &muiNameOffset, &muiNameLength))
	{
		muiNameOffset = 0;
		muiNameLength = 0;
	}

	return ChooseCmap(uiCmapOffset, uiCmapLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Picks the Unicode character map.  A full repertoire format 12 map is
// preferred to a Basic Multilingual Plane format 4 map.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::ChooseCmap(size uiCmapOffset, size uiCmapLength)
{
	uint16	uiNumTables;
	uint16	i;
	size	uiRecord;
	uint16	uiPlatform;
	uint16	uiEncoding;
	uint32	uiSubtableOffset;
	uint16	uiFormat;
	uint16	uiLength16;
	uint32	uiLength32;
	bool	bUnicode;
	bool	bFullUnicode;
	size	uiSubtable;
	size	uiSubtableLength;

	if (!ReadUint16(uiCmapOffset + 2, &uiNumTables))
	{
		return false;
	}

	for (i = 0; i < uiNumTables; i++)
	{
		uiRecord = uiCmapOffset + 4 + (size)i * 8;
		if (!ReadUint16(uiRecord, &uiPlatform) || !ReadUint16(uiRecord + 2, &uiEncoding) || !ReadUint32(uiRecord + 4, &uiSubtableOffset))
		{
			return false;
		}

		bUnicode = (uiPlatform == 0) || ((uiPlatform == 3) && (uiEncoding == 1));
		bFullUnicode = (uiPlatform == 0) || ((uiPlatform == 3) && (uiEncoding == 10));
		if (!bUnicode && !bFullUnicode)
		{
			continue;
		}
		if (uiSubtableOffset >= uiCmapLength)
		{
			continue;
		}

		uiSubtable = uiCmapOffset + uiSubtableOffset;
		if (!ReadUint16(uiSubtable, &uiFormat))
		{
			continue;
		}

		if ((uiFormat == 12) && bFullUnicode)
		{
			if (!ReadUint32(uiSubtable + 4, &uiLength32) || !IsInFile(uiSubtable, uiLength32))
			{
				continue;
			}
			uiSubtableLength = uiLength32;
		}
		else if ((uiFormat == 4) && bUnicode && (miCmapFormat != 12))
		{
			if (!ReadUint16(uiSubtable + 2, &uiLength16) || !IsInFile(uiSubtable, uiLength16))
			{
				continue;
			}
			uiSubtableLength = uiLength16;
		}
		else
		{
			continue;
		}

		muiCmapOffset = uiSubtable;
		muiCmapLength = uiSubtableLength;
		miCmapFormat = uiFormat;
	}

	return miCmapFormat != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFont::NumGlyphs(void)
{
	return miNumGlyphs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFont::GetUnitsPerEm(void)
{
	return miUnitsPerEm;
}


//////////////////////////////////////////////////////////////////////////
//
// The distance from the baseline to the top of a line, positive.
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFont::GetAscender(void)
{
	return miAscender;
}


//////////////////////////////////////////////////////////////////////////
//
// The distance from the baseline to the bottom of a line, negative.
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFont::GetDescender(void)
{
	return miDescender;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFont::GetLineGap(void)
{
	return miLineGap;
}


//////////////////////////////////////////////////////////////////////////
//
// The bounds of every glyph.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFont::GetBounds(int* piXMin, int* piYMin, int* piXMax, int* piYMax)
{
	*piXMin = miXMin;
	*piYMin = miYMin;
	*piXMax = miXMax;
	*piYMax = miYMax;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends a name (TRUE_TYPE_NAME_FAMILY etc...) to psz as UTF-8.
// English Windows names are preferred.  Returns false if the font
// doesn't have the name.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::GetName(int iNameID, CChars* psz)
{
	uint16	uiCount;
	uint16	uiStringOffset;
	uint16	i;
	size	uiRecord;
	uint16	uiPlatform;
	uint16	uiEncoding;
	uint16	uiLanguage;
	uint16	uiNameID;
	uint16	uiLength;
	uint16	uiOffset;
	int		iScore;
	int		iBestScore;
	size	uiBestRecord;
	bool	bUTF16;
	size	uiString;
	size	j;
	uint16	uiChar;

	if ((muiNameLength < 6) || !ReadUint16(muiNameOffset + 2, &uiCount) || !ReadUint16(muiNameOffset + 4, &uiStringOffset))
	{
		return false;
	}

	iBestScore = 0;
	uiBestRecord = 0;
	for (i = 0; i < uiCount; i++)
	{
		uiRecord = muiNameOffset + 6 + (size)i * 12;
		if (!ReadUint16(uiRecord, &uiPlatform) || !ReadUint16(uiRecord + 2, &uiEncoding) || !ReadUint16(uiRecord + 4, &uiLanguage) || !ReadUint16(uiRecord + 6, &uiNameID))
		{
			return false;
		}
		if (uiNameID != iNameID)
		{
			continue;
		}

		if ((uiPlatform == 3) && (uiEncoding == 1))
		{
			iScore = (uiLanguage == 0x0409) ? 4 : 3;
		}
		else if (uiPlatform == 0)
		{
			iScore = 2;
		}
		else if ((uiPlatform == 1) && (uiEncoding == 0))
		{
			iScore = 1;
		}
		else
		{
			iScore = 0;
		}

		if (iScore > iBestScore)
		{
			iBestScore = iScore;
			uiBestRecord = uiRecord;
		}
	}

	if (iBestScore == 0)
	{
		return false;
	}

	ReadUint16(uiBestRecord, &uiPlatform);
	ReadUint16(uiBestRecord + 8, &uiLength);
	ReadUint16(uiBestRecord + 10, &uiOffset);
	uiString = muiNameOffset + uiStringOffset + uiOffset;
	if (!IsInFile(uiString, uiLength))
	{
		return false;
	}

	//Windows and Unicode names are UTF-16 big endian, Macintosh names are Mac Roman (only ASCII is kept).
	bUTF16 = uiPlatform != 1;
	if (bUTF16)
	{
		for (j = 0; j + 1 < uiLength; j += 2)
		{
			ReadUint16(uiString + j, &uiChar);
			if (uiChar < 0x80)
			{
				psz->Append((char)uiChar);
			}
			else if (uiChar < 0x800)
			{
				psz->Append((char)(0xC0 | (uiChar >> 6)));
				psz->Append((char)(0x80 | (uiChar & 0x3F)));
			}
			else
			{
				psz->Append((char)(0xE0 | (uiChar >> 12)));
				psz->Append((char)(0x80 | ((uiChar >> 6) & 0x3F)));
				psz->Append((char)(0x80 | (uiChar & 0x3F)));
			}
		}
	}
	else
	{
		for (j = 0; j < uiLength; j++)
		{
			uiChar = mpuiData[uiString + j];
			psz->Append((uiChar < 0x80) ? (char)uiChar : '?');
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns 0 (the missing glyph) for characters not in the font.
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFont::GetGlyphIndex(uint32 uiCodePoint)
{
	int		iGlyph;

	if (miCmapFormat == 12)
	{
		iGlyph = GetGlyphIndexFormat12(uiCodePoint);
	}
	else if (miCmapFormat == 4)
	{
		iGlyph = GetGlyphIndexFormat4(uiCodePoint);
	}
	else
	{
		return 0;
	}

	if ((iGlyph < 0) || (iGlyph >= miNumGlyphs))
	{
		return 0;
	}
	return iGlyph;
}


//////////////////////////////////////////////////////////////////////////
//
// Segment mapping to delta values.  Segments are sorted by their end code.
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFont::GetGlyphIndexFormat4(uint32 uiCodePoint)
{
	uint16	uiSegCountX2;
	size	uiSegCount;
	size	uiEndCodes;
	size	uiStartCodes;
	size	uiDeltas;
	size	uiRangeOffsets;
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;
	uint16	uiEnd;
	uint16	uiStart;
	int16	iDelta;
	uint16	uiRangeOffset;
	uint16	uiGlyph;
	size	uiGlyphAddress;

	if ((uiCodePoint > 0xFFFF) || !ReadUint16(muiCmapOffset + 6, &uiSegCountX2))
	{
		return 0;
	}

	uiSegCount = uiSegCountX2 / 2;
	uiEndCodes = muiCmapOffset + 14;
	uiStartCodes = uiEndCodes + uiSegCountX2 + 2;
	uiDeltas = uiStartCodes + uiSegCountX2;
	uiRangeOffsets = uiDeltas + uiSegCountX2;
	if (!IsInFile(uiEndCodes, (size)uiSegCountX2 * 4 + 2))
	{
		return 0;
	}

	//Find the first segment that ends at or after the code point.
	uiLow = 0;
	uiHigh = uiSegCount;
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		ReadUint16(uiEndCodes + uiMiddle * 2, &uiEnd);
		if (uiEnd < uiCodePoint)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	if (uiLow == uiSegCount)
	{
		return 0;
	}

	ReadUint16(uiStartCodes + uiLow * 2, &uiStart);
	if (uiStart > uiCodePoint)
	{
		return 0;
	}
	ReadInt16(uiDeltas + uiLow * 2, &iDelta);
	ReadUint16(uiRangeOffsets + uiLow * 2, &uiRangeOffset);

	if (uiRangeOffset == 0)
	{
		return (uint16)(uiCodePoint + iDelta);
	}

	//The range offset is relative to where it is stored.
	uiGlyphAddress = uiRangeOffsets + uiLow * 2 + uiRangeOffset + (uiCodePoint - uiStart) * 2;
	if (!ReadUint16(uiGlyphAddress, &uiGlyph) || (uiGlyph == 0))
	{
		return 0;
	}
	return (uint16)(uiGlyph + iDelta);
}


//////////////////////////////////////////////////////////////////////////
//
// Segmented coverage.  Groups are sorted by their start code.
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFont::GetGlyphIndexFormat12(uint32 uiCodePoint)
{
	uint32	uiNumGroups;
	size	uiGroups;
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;
	uint32	uiStart;
	uint32	uiEnd;
	uint32	uiStartGlyph;

	if (!ReadUint32(muiCmapOffset + 12, &uiNumGroups))
	{
		return 0;
	}
	uiGroups = muiCmapOffset + 16;
	if ((uiNumGroups > muiSize / 12) || !IsInFile(uiGroups, (size)uiNumGroups * 12))
	{
		return 0;
	}

	uiLow = 0;
	uiHigh = uiNumGroups;
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		ReadUint32(uiGroups + uiMiddle * 12, &uiStart);
		ReadUint32(uiGroups + uiMiddle * 12 + 4, &uiEnd);
		if (uiCodePoint < uiStart)
		{
			uiHigh = uiMiddle;
		}
		else if (uiCodePoint > uiEnd)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			ReadUint32(uiGroups + uiMiddle * 12 + 8, &uiStartGlyph);
			return (int)(uiStartGlyph + (uiCodePoint - uiStart));
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Glyphs without contours (spaces) have an empty bounding box.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::GetGlyphMetrics(int iGlyph, STrueTypeGlyphMetrics* psMetrics)
{
	uint16	uiAdvance;
	int16	iLeftSideBearing;
	size	uiOffset;
	size	uiLength;
	int16	iXMin;
	int16	iYMin;
	int16	iXMax;
	int16	iYMax;

	if ((iGlyph < 0) || (iGlyph >= miNumGlyphs))
	{
		return false;
	}

	//Glyphs past the last long metric share its advance width.
	if (iGlyph < miNumHMetrics)
	{
		ReadUint16(muiHmtxOffset + (size)iGlyph * 4, &uiAdvance);
		ReadInt16(muiHmtxOffset + (size)iGlyph * 4 + 2, &iLeftSideBearing);
	}
	else
	{
		ReadUint16(muiHmtxOffset + ((size)miNumHMetrics - 1) * 4, &uiAdvance);
		ReadInt16(muiHmtxOffset + (size)miNumHMetrics * 4 + ((size)iGlyph - miNumHMetrics) * 2, &iLeftSideBearing);
	}
	psMetrics->iAdvanceWidth = uiAdvance;
	psMetrics->iLeftSideBearing = iLeftSideBearing;

	if (!GetGlyphData(iGlyph, &uiOffset, &uiLength))
	{
		return false;
	}
	if (uiLength == 0)
	{
		psMetrics->iXMin = 0;
		psMetrics->iYMin = 0;
		psMetrics->iXMax = 0;
		psMetrics->iYMax = 0;
		return true;
	}
	if ((uiLength < 10) || !ReadInt16(uiOffset + 2, &iXMin) || !ReadInt16(uiOffset + 4, &iYMin) || !ReadInt16(uiOffset + 6, &iXMax) || !ReadInt16(uiOffset + 8, &iYMax))
	{
		return false;
	}
	psMetrics->iXMin = iXMin;
	psMetrics->iYMin = iYMin;
	psMetrics->iXMax = iXMax;
	psMetrics->iYMax = iYMax;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds a glyph's data in the glyf table.  A length of zero is a glyph
// without an outline.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::GetGlyphData(int iGlyph, size* puiOffset, size* puiLength)
{
	uint16	uiStart16;
	uint16	uiEnd16;
	uint32	uiStart;
	uint32	uiEnd;

	if ((iGlyph < 0) || (iGlyph >= miNumGlyphs))
	{
		return false;
	}

	if (mbLongLoca)
	{
		ReadUint32(muiLocaOffset + (size)iGlyph * 4, &uiStart);
		ReadUint32(muiLocaOffset + (size)iGlyph * 4 + 4, &uiEnd);
	}
	else
	{
		//Short offsets are stored halved.
		ReadUint16(muiLocaOffset + (size)iGlyph * 2, &uiStart16);
		ReadUint16(muiLocaOffset + (size)iGlyph * 2 + 2, &uiEnd16);
		uiStart = (uint32)uiStart16 * 2;
		uiEnd = (uint32)uiEnd16 * 2;
	}

	if ((uiEnd < uiStart) || (uiEnd > muiGlyfLength))
	{
		return false;
	}
	*puiOffset = muiGlyfOffset + uiStart;
	*puiLength = uiEnd - uiStart;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends a glyph's contours to pcOutline (in font units).  Returns
// false if the glyph is corrupt, in which case pcOutline is cleared.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::GetGlyphOutline(int iGlyph, CTrueTypeOutline* pcOutline)
{
	if (!ReadGlyph(iGlyph, pcOutline, 0))
	{
		pcOutline->Clear();
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::ReadGlyph(int iGlyph, CTrueTypeOutline* pcOutline, int iDepth)
{
	size	uiOffset;
	size	uiLength;
	int16	iNumContours;

	if (iDepth > TRUE_TYPE_MAX_COMPOSITE_DEPTH)
	{
		return false;
	}
	if (!GetGlyphData(iGlyph, &uiOffset, &uiLength))
	{
		return false;
	}
	if (uiLength == 0)
	{
		return true;
	}
	if ((uiLength < 10) || !ReadInt16(uiOffset, &iNumContours))
	{
		return false;
	}

	//The glyph header is the number of contours (negative for a composite glyph) and the bounding box.
	if (iNumContours >= 0)
	{
		return ReadSimpleGlyph(uiOffset + 10, uiLength - 10, iNumContours, pcOutline);
	}
	else
	{
		return ReadCompositeGlyph(uiOffset + 10, uiLength - 10, pcOutline, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A simple glyph is the end point of each contour, hinting instructions
// and then the flags, x and y coordinates of the points.  Flags can
// repeat and coordinates are deltas from the previous point.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::ReadSimpleGlyph(size uiOffset, size uiLength, int iNumContours, CTrueTypeOutline* pcOutline)
{
	size			uiEnd;
	size			uiFirstPoint;
	int				i;
	uint16			uiContourEnd;
	int				iLastContourEnd;
	size			uiNumPoints;
	uint16			uiInstructionLength;
	size			uiPos;
	size			uiPoint;
	uint8			uiFlag;
	uint8			uiRepeat;
	uint8			uiDelta8;
	int16			iDelta16;
	int				iValue;
	STrueTypePoint*	psPoint;
	uint8*			puiFlags;

	uiEnd = uiOffset + uiLength;
	if (!IsInFile(uiOffset, uiLength) || ((size)iNumContours * 2 + 2 > uiLength))
	{
		return false;
	}
	if (iNumContours == 0)
	{
		return true;
	}

	uiFirstPoint = pcOutline->NumPoints();
	iLastContourEnd = -1;
	for (i = 0; i < iNumContours; i++)
	{
		ReadUint16(uiOffset + (size)i * 2, &uiContourEnd);
		if ((int)uiContourEnd <= iLastContourEnd)
		{
			//An empty contour.
			if ((int)uiContourEnd < iLastContourEnd)
			{
				return false;
			}
			continue;
		}
		iLastContourEnd = uiContourEnd;
		pcOutline->maiContourEnds.Add((int)(uiFirstPoint + uiContourEnd));
	}
	uiNumPoints = (size)iLastContourEnd + 1;

	uiPos = uiOffset + (size)iNumContours * 2;
	ReadUint16(uiPos, &uiInstructionLength);
	uiPos += 2 + uiInstructionLength;
	if (uiPos > uiEnd)
	{
		return false;
	}

	//The flags are read first, then they say how big each coordinate is.
	pcOutline->macPoints.GrowToAtLeastNumElements(uiFirstPoint + uiNumPoints, true);
	puiFlags = (uint8*)malloc(uiNumPoints);
	if (puiFlags == NULL)
	{
		return false;
	}

	uiPoint = 0;
	while (uiPoint < uiNumPoints)
	{
		if ((uiPos >= uiEnd) || !ReadUint8(uiPos, &uiFlag))
		{
			free(puiFlags);
			return false;
		}
		uiPos++;
		puiFlags[uiPoint] = uiFlag;
		uiPoint++;
		if (uiFlag & TRUE_TYPE_REPEAT)
		{
			if ((uiPos >= uiEnd) || !ReadUint8(uiPos, &uiRepeat))
			{
				free(puiFlags);
				return false;
			}
			uiPos++;
			for (; (uiRepeat > 0) && (uiPoint < uiNumPoints); uiRepeat--)
			{
				puiFlags[uiPoint] = uiFlag;
				uiPoint++;
			}
		}
	}

	iValue = 0;
	for (uiPoint = 0; uiPoint < uiNumPoints; uiPoint++)
	{
		uiFlag = puiFlags[uiPoint];
		if (uiFlag & TRUE_TYPE_X_SHORT)
		{
			if ((uiPos + 1 > uiEnd) || !ReadUint8(uiPos, &uiDelta8))
			{
				free(puiFlags);
				return false;
			}
			uiPos++;
			iValue += (uiFlag & TRUE_TYPE_X_SAME_OR_POSITIVE) ? uiDelta8 : -(int)uiDelta8;
		}
		else if (!(uiFlag & TRUE_TYPE_X_SAME_OR_POSITIVE))
		{
			if ((uiPos + 2 > uiEnd) || !ReadInt16(uiPos, &iDelta16))
			{
				free(puiFlags);
				return false;
			}
			uiPos += 2;
			iValue += iDelta16;
		}
		psPoint = pcOutline->GetPoint(uiFirstPoint + uiPoint);
		psPoint->x = iValue;
		psPoint->bOnCurve = (uiFlag & TRUE_TYPE_ON_CURVE) != 0;
	}

	iValue = 0;
	for (uiPoint = 0; uiPoint < uiNumPoints; uiPoint++)
	{
		uiFlag = puiFlags[uiPoint];
		if (uiFlag & TRUE_TYPE_Y_SHORT)
		{
			if ((uiPos + 1 > uiEnd) || !ReadUint8(uiPos, &uiDelta8))
			{
				free(puiFlags);
				return false;
			}
			uiPos++;
			iValue += (uiFlag & TRUE_TYPE_Y_SAME_OR_POSITIVE) ? uiDelta8 : -(int)uiDelta8;
		}
		else if (!(uiFlag & TRUE_TYPE_Y_SAME_OR_POSITIVE))
		{
			if ((uiPos + 2 > uiEnd) || !ReadInt16(uiPos, &iDelta16))
			{
				free(puiFlags);
				return false;
			}
			uiPos += 2;
			iValue += iDelta16;
		}
		psPoint = pcOutline->GetPoint(uiFirstPoint + uiPoint);
		psPoint->y = iValue;
	}

	free(puiFlags);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A composite glyph is a list of other glyphs, each moved and possibly
// scaled, rotated or skewed.  A component is either offset by x and y or
// placed so that one of its points lies on a point already in the glyph.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFont::ReadCompositeGlyph(size uiOffset, size uiLength, CTrueTypeOutline* pcOutline, int iDepth)
{
	size				uiEnd;
	size				uiPos;
	size				uiFirstPoint;
	uint16				uiFlags;
	uint16				uiGlyph;
	int					iArg1;
	int					iArg2;
	int16				iWord1;
	int16				iWord2;
	uint8				uiByte1;
	uint8				uiByte2;
	int16				iScale;
	int32				iXX;
	int32				iXY;
	int32				iYX;
	int32				iYY;
	int32				iDX;
	int32				iDY;
	int32				x;
	int32				y;
	CTrueTypeOutline	cComponent;
	size				i;
	STrueTypePoint*		psSource;
	STrueTypePoint*		psDest;
	size				uiNumComponentPoints;

	uiEnd = uiOffset + uiLength;
	uiPos = uiOffset;
	uiFirstPoint = pcOutline->NumPoints();

	cComponent.Init();
	do
	{
		if ((uiPos + 4 > uiEnd) || !ReadUint16(uiPos, &uiFlags) || !ReadUint16(uiPos + 2, &uiGlyph))
		{
			cComponent.Kill();
			return false;
		}
		uiPos += 4;

		//The arguments are either x and y offsets, or a parent point and a component point to match.
		if (uiFlags & TRUE_TYPE_ARGS_ARE_WORDS)
		{
			if ((uiPos + 4 > uiEnd) || !ReadInt16(uiPos, &iWord1) || !ReadInt16(uiPos + 2, &iWord2))
			{
				cComponent.Kill();
				return false;
			}
			uiPos += 4;
			if (uiFlags & TRUE_TYPE_ARGS_ARE_XY_VALUES)
			{
				iArg1 = iWord1;
				iArg2 = iWord2;
			}
			else
			{
				iArg1 = (uint16)iWord1;
				iArg2 = (uint16)iWord2;
			}
		}
		else
		{
			if ((uiPos + 2 > uiEnd) || !ReadUint8(uiPos, &uiByte1) || !ReadUint8(uiPos + 1, &uiByte2))
			{
				cComponent.Kill();
				return false;
			}
			uiPos += 2;
			if (uiFlags & TRUE_TYPE_ARGS_ARE_XY_VALUES)
			{
				iArg1 = (int8)uiByte1;
				iArg2 = (int8)uiByte2;
			}
			else
			{
				iArg1 = uiByte1;
				iArg2 = uiByte2;
			}
		}

		//x' = iXX * x + iYX * y,  y' = iXY * x + iYY * y in 2.14 fixed point.
		iXX = TRUE_TYPE_F2DOT14_ONE;
		iXY = 0;
		iYX = 0;
		iYY = TRUE_TYPE_F2DOT14_ONE;
		if (uiFlags & TRUE_TYPE_HAVE_A_SCALE)
		{
			if ((uiPos + 2 > uiEnd) || !ReadInt16(uiPos, &iScale))
			{
				cComponent.Kill();
				return false;
			}
			uiPos += 2;
			iXX = iScale;
			iYY = iScale;
		}
		else if (uiFlags & TRUE_TYPE_HAVE_AN_X_AND_Y_SCALE)
		{
			if ((uiPos + 4 > uiEnd) || !ReadInt16(uiPos, &iWord1) || !ReadInt16(uiPos + 2, &iWord2))
			{
				cComponent.Kill();
				return false;
			}
			uiPos += 4;
			iXX = iWord1;
			iYY = iWord2;
		}
		else if (uiFlags & TRUE_TYPE_HAVE_A_TWO_BY_TWO)
		{
			if (uiPos + 8 > uiEnd)
			{
				cComponent.Kill();
				return false;
			}
			ReadInt16(uiPos, &iScale);
			iXX = iScale;
			ReadInt16(uiPos + 2, &iScale);
			iXY = iScale;
			ReadInt16(uiPos + 4, &iScale);
			iYX = iScale;
			ReadInt16(uiPos + 6, &iScale);
			iYY = iScale;
			uiPos += 8;
		}

		cComponent.Clear();
		if (!ReadGlyph(uiGlyph, &cComponent, iDepth + 1))
		{
			cComponent.Kill();
			return false;
		}
		uiNumComponentPoints = cComponent.NumPoints();

		//Transform the component's points.
		for (i = 0; i < uiNumComponentPoints; i++)
		{
			psSource = cComponent.GetPoint(i);
			x = psSource->x;
			y = psSource->y;
			psSource->x = (int32)DivideRound((int64)iXX * x + (int64)iYX * y, TRUE_TYPE_F2DOT14_ONE);
			psSource->y = (int32)DivideRound((int64)iXY * x + (int64)iYY * y, TRUE_TYPE_F2DOT14_ONE);
		}

		if (uiFlags & TRUE_TYPE_ARGS_ARE_XY_VALUES)
		{
			iDX = iArg1;
			iDY = iArg2;
			if ((uiFlags & TRUE_TYPE_SCALED_OFFSET) && !(uiFlags & TRUE_TYPE_UNSCALED_OFFSET))
			{
				x = iDX;
				y = iDY;
				iDX = (int32)DivideRound((int64)iXX * x + (int64)iYX * y, TRUE_TYPE_F2DOT14_ONE);
				iDY = (int32)DivideRound((int64)iXY * x + (int64)iYY * y, TRUE_TYPE_F2DOT14_ONE);
			}
		}
		else
		{
			if (((size)iArg1 >= pcOutline->NumPoints() - uiFirstPoint) || ((size)iArg2 >= uiNumComponentPoints))
			{
				cComponent.Kill();
				return false;
			}
			psDest = pcOutline->GetPoint(uiFirstPoint + iArg1);
			psSource = cComponent.GetPoint(iArg2);
			iDX = psDest->x - psSource->x;
			iDY = psDest->y - psSource->y;
		}

		//Append the component's contours after the glyph's.
		for (i = 0; i < cComponent.NumContours(); i++)
		{
			pcOutline->maiContourEnds.Add((int)(pcOutline->NumPoints() + cComponent.GetContourEnd(i)));
		}
		for (i = 0; i < uiNumComponentPoints; i++)
		{
			psSource = cComponent.GetPoint(i);
			psDest = pcOutline->macPoints.Add();
			psDest->x = psSource->x + iDX;
			psDest->y = psSource->y + iDY;
			psDest->bOnCurve = psSource->bOnCurve;
		}
	}
	while (uiFlags & TRUE_TYPE_MORE_COMPONENTS);

	cComponent.Kill();
	return true;
}
