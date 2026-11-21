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
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/FNVHash.h"
#include "BaseLib/FileBasic.h"
#include "SFTCommon.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadSFTHeader(CFileReader* pcFile, uint16* puiMagic, uint8* puiVersion, uint8* puiType)
{
	return	pcFile->ReadLittleEndianInt16(puiMagic) &&
			pcFile->ReadInt8(puiVersion) &&
			pcFile->ReadInt8(puiType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool WriteSFTHeader(CFileWriter* pcFile, uint16 uiMagic, uint8 uiVersion, uint8 uiType)
{
	return	pcFile->WriteLittleEndianInt16(uiMagic) &&
			pcFile->WriteInt8(uiVersion) &&
			pcFile->WriteInt8(uiType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTHeader::Init(uint8 uiType)
{
	uiMagic = SFT_MAGIC;
	uiVersion = SFT_VERSION;
	this->uiType = uiType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTHeader::Read(CFileReader* pcFile)
{
	return ReadSFTHeader(pcFile, &uiMagic, &uiVersion, &uiType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTHeader::Write(CFileWriter* pcFile)
{
	return WriteSFTHeader(pcFile, uiMagic, uiVersion, uiType);
}


//////////////////////////////////////////////////////////////////////////
//
// A record of this layout of the type.
//
//////////////////////////////////////////////////////////////////////////
bool SSFTHeader::Is(uint8 uiType) const
{
	return IsValid() && (this->uiType == uiType);
}


//////////////////////////////////////////////////////////////////////////
//
// A record of this layout.
//
//////////////////////////////////////////////////////////////////////////
bool SSFTHeader::IsValid(void) const
{
	return (uiMagic == SFT_MAGIC) && (uiVersion == SFT_VERSION);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTImage::Init(uint8 uiType, uint16 uiImageWidth, uint16 uiImageHeight)
{
	uiMagic = SFT_MAGIC;
	uiVersion = SFT_VERSION;
	this->uiType = uiType;
	this->uiImageWidth = uiImageWidth;
	this->uiImageHeight = uiImageHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTImage::Read(CFileReader* pcFile)
{
	return	ReadSFTHeader(pcFile, &uiMagic, &uiVersion, &uiType) &&
			pcFile->ReadLittleEndianInt16(&uiImageWidth) &&
			pcFile->ReadLittleEndianInt16(&uiImageHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTImage::Write(CFileWriter* pcFile)
{
	return	WriteSFTHeader(pcFile, uiMagic, uiVersion, uiType) &&
			pcFile->WriteLittleEndianInt16(uiImageWidth) &&
			pcFile->WriteLittleEndianInt16(uiImageHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// An image of this layout of the type.
//
//////////////////////////////////////////////////////////////////////////
bool SSFTImage::Is(uint8 uiType) const
{
	return (uiMagic == SFT_MAGIC) && (uiVersion == SFT_VERSION) && (this->uiType == uiType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTContainer::Init(uint16 uiCount)
{
	uiMagic = SFT_MAGIC;
	uiVersion = SFT_VERSION;
	uiType = SFT_TYPE_CONTAINER;
	this->uiCount = uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTContainer::Read(CFileReader* pcFile)
{
	return	ReadSFTHeader(pcFile, &uiMagic, &uiVersion, &uiType) &&
			pcFile->ReadLittleEndianInt16(&uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTContainer::Write(CFileWriter* pcFile)
{
	return	WriteSFTHeader(pcFile, uiMagic, uiVersion, uiType) &&
			pcFile->WriteLittleEndianInt16(uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTContainer::IsValid(void) const
{
	return (uiMagic == SFT_MAGIC) && (uiVersion == SFT_VERSION) && (uiType == SFT_TYPE_CONTAINER);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTContained::Init(void)
{
	uiSkipToNext = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTContained::Init(uint16 uiSkipToNext)
{
	this->uiSkipToNext = uiSkipToNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTContained::Read(CFileReader* pcFile)
{
	return	pcFile->ReadLittleEndianInt16(&uiSkipToNext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTContained::Write(CFileWriter* pcFile)
{
	return	pcFile->WriteLittleEndianInt16(uiSkipToNext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFTPalette::Init(uint16 uiFirst, uint16 uiCount, uint32 uiHash)
{
	uiMagic = SFT_MAGIC;
	uiVersion = SFT_VERSION;
	uiType = SFT_TYPE_PALETTE;
	this->uiFirst = uiFirst;
	this->uiCount = uiCount;
	this->uiHash = uiHash;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTPalette::Read(CFileReader* pcFile)
{
	return	ReadSFTHeader(pcFile, &uiMagic, &uiVersion, &uiType) &&
			pcFile->ReadLittleEndianInt16(&uiFirst) &&
			pcFile->ReadLittleEndianInt16(&uiCount) &&
			pcFile->ReadLittleEndianInt32(&uiHash);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSFTPalette::Write(CFileWriter* pcFile)
{
	return	WriteSFTHeader(pcFile, uiMagic, uiVersion, uiType) &&
			pcFile->WriteLittleEndianInt16(uiFirst) &&
			pcFile->WriteLittleEndianInt16(uiCount) &&
			pcFile->WriteLittleEndianInt32(uiHash);
}


//////////////////////////////////////////////////////////////////////////
//
// A palette of this layout with 1 to 256 colours, none past index 255.
//
//////////////////////////////////////////////////////////////////////////
bool SSFTPalette::IsValid(void) const
{
	return	(uiMagic == SFT_MAGIC) && (uiVersion == SFT_VERSION) && (uiType == SFT_TYPE_PALETTE) &&
			(uiCount > 0) && (uiCount <= SFT_PALETTE_COLOURS) && ((uint32)uiFirst + uiCount <= SFT_PALETTE_COLOURS);
}


//////////////////////////////////////////////////////////////////////////
//
// No palette: every pixel is a colour.
//
//////////////////////////////////////////////////////////////////////////
void SSFTColours::Init(void)
{
	uiFirst = 0;
	uiCount = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A palette record, which must be whole and its hash right.
//
//////////////////////////////////////////////////////////////////////////
bool SSFTColours::Read(CFileReader* pcFile)
{
	SSFTPalette	sPalette;

	if (!sPalette.Read(pcFile) || !sPalette.IsValid())
	{
		return false;
	}
	if (!pcFile->ReadData(&auiRed[sPalette.uiFirst], sPalette.uiCount) ||
		!pcFile->ReadData(&auiGreen[sPalette.uiFirst], sPalette.uiCount) ||
		!pcFile->ReadData(&auiBlue[sPalette.uiFirst], sPalette.uiCount))
	{
		return false;
	}
	if (SFTPaletteHash(sPalette.uiFirst, sPalette.uiCount, &auiRed[sPalette.uiFirst], &auiGreen[sPalette.uiFirst], &auiBlue[sPalette.uiFirst]) != sPalette.uiHash)
	{
		return false;
	}
	uiFirst = sPalette.uiFirst;
	uiCount = sPalette.uiCount;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a pixel of uiIndex has a colour: any does with no palette.
//
//////////////////////////////////////////////////////////////////////////
bool SSFTColours::Has(uint8 uiIndex) const
{
	return (uiCount == 0) || ((uiIndex >= uiFirst) && (uiIndex < uiFirst + uiCount));
}


//////////////////////////////////////////////////////////////////////////
//
// A pixel of uiIndex as red, green, blue and an opaque alpha, low byte
// first (Has must be true).
//
//////////////////////////////////////////////////////////////////////////
uint32 SSFTColours::Colour32(uint8 uiIndex) const
{
	if (uiCount == 0)
	{
		return SFTColour32(uiIndex);
	}
	return (uint32)auiRed[uiIndex] | ((uint32)auiGreen[uiIndex] << 8) | ((uint32)auiBlue[uiIndex] << 16) | ((uint32)0xff << 24);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 SFTPaletteHash(uint16 uiFirst, uint16 uiCount, const uint8* puiRed, const uint8* puiGreen, const uint8* puiBlue)
{
	uint8	auiNumbers[4];
	uint32	uiHash;

	auiNumbers[0] = (uint8)uiFirst;
	auiNumbers[1] = (uint8)(uiFirst >> 8);
	auiNumbers[2] = (uint8)uiCount;
	auiNumbers[3] = (uint8)(uiCount >> 8);
	uiHash = FNVHash32(auiNumbers, 4);
	uiHash = FNVHash32(puiRed, uiCount, uiHash);
	uiHash = FNVHash32(puiGreen, uiCount, uiHash);
	return FNVHash32(puiBlue, uiCount, uiHash);
}


//////////////////////////////////////////////////////////////////////////
//
// Three bits and two bits repeated to fill eight.
//
//////////////////////////////////////////////////////////////////////////
uint32 SFTColour32(uint8 uiPixel)
{
	uint8	r;
	uint8	g;
	uint8	b;

	b = (uiPixel >> 6) & 0x3;
	g = (uiPixel >> 3) & 0x7;
	r = (uiPixel >> 0) & 0x7;

	b = (uint8)((b << 6) + (b << 4) + (b << 2) + b);
	g = (uint8)((g << 5) + (g << 2) + (g >> 1));
	r = (uint8)((r << 5) + (r << 2) + (r >> 1));
	return (uint32)r | ((uint32)g << 8) | ((uint32)b << 16) | ((uint32)0xff << 24);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PeekSFTHeader(CFileBasic* pcFile, SSFTHeader* psHeader)
{
	filePos	uiPosition;
	bool	bResult;

	uiPosition = pcFile->Tell();
	bResult = psHeader->Read(pcFile);
	return pcFile->Seek(uiPosition) && bResult;
}
