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
#ifndef __SFT_COMMON_H__
#define __SFT_COMMON_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/FileReader.h"
#include "BaseLib/FileWriter.h"


class CFileBasic;


//An SFT file is a record: an image, a palette or a container of them.  Every record starts with the same header: the
//magic number SFT_MAGIC ("SF"), the layout's version (SFT_VERSION) and its type (SFT_TYPE_*).  Every word is low byte
//first.
//
//A pixel is a byte.  It is an index into the palette that comes before the image in its container, or with no palette a
//colour: red in D0-D2, green in D3-D5 and blue in D6-D7 (SFTColours' palette).
//
//An opaque image is an SSFTImage (SFT_TYPE_OPAQUE), then its rows from the top, each its width of pixels.
//
//A transparent image is an SSFTImage (SFT_TYPE_TRANSPARENT), then bytes that say where its pixels are, starting at its
//top left:
//  SFT_RUN_END              the image is done.
//  $01-$7F                  that many pixels follow: they are drawn from here to the right.
//  SFT_RUN_SKIP + (n - 1)   n pixels (1 to SFT_RUN_MAX_COUNT) are skipped to the right.
//  SFT_RUN_ROWS + (n - 1)   n rows (1 to SFT_RUN_MAX_ROWS) down, at the row's left.
//  SFT_RUN_MIXED + (n - 1)  n pixels (1 to SFT_RUN_MAX_MIXED) follow, of which those that are SFT_PIXEL_TRANSPARENT
//                           aren't drawn and the rest are, from here to the right: a run with short gaps in it, drawn
//                           as one copy that leaves a colour out (a blitter's transparent colour).  A pixel that is
//                           drawn and is SFT_PIXEL_TRANSPARENT can't be in one.
//A longer run, skip or move down is several of them.  What isn't drawn is transparent.
//
//A palette is an SSFTPalette (SFT_TYPE_PALETTE), then its uiCount reds, its uiCount greens and its uiCount blues: the
//colours of the indices uiFirst to uiFirst + uiCount - 1 (no more than 256 of them).  The images that use it have no
//pixel outside them (but SFT_PIXEL_TRANSPARENT in a transparent image's mixed run, which isn't drawn).  Its uiHash is
//SFTPaletteHash's: palettes with the same hash are very likely the same, so images can share one.
//
//A container is an SSFTContainer, then its uiCount records (images and palettes, no containers), each after an
//SSFTContained.  A palette is the palette of the images after it, up to the next.  An image doesn't cross a 64KB
//boundary of the file (SaveSFTContainer skips to the next).
#define SFT_MAGIC				(uint16)0x4653
#define SFT_VERSION				(uint8)1

#define SFT_TYPE_OPAQUE			(uint8)1
#define SFT_TYPE_TRANSPARENT	(uint8)2
#define SFT_TYPE_CONTAINER		(uint8)3
#define SFT_TYPE_PALETTE		(uint8)4

#define SFT_RUN_END				0x00
#define SFT_RUN_MAX_PIXELS		0x7f
#define SFT_RUN_SKIP			0x80
#define SFT_RUN_ROWS			0xc0
#define SFT_RUN_MIXED			0xe0
#define SFT_RUN_COUNT_MASK		0x3f
#define SFT_RUN_MAX_COUNT		0x40
#define SFT_RUN_ROWS_MASK		0x1f
#define SFT_RUN_MAX_ROWS		0x20
#define SFT_RUN_MIXED_MASK		0x1f
#define SFT_RUN_MAX_MIXED		0x20
#define SFT_PIXEL_TRANSPARENT	0x00

#define SFT_PALETTE_COLOURS		256

#define SFT_HEADER_SIZE			4
#define SFT_IMAGE_SIZE			8
#define SFT_CONTAINER_SIZE		6
#define SFT_PALETTE_SIZE		12


//What starts every record.
struct SSFTHeader
{
	uint16	uiMagic;
	uint8	uiVersion;
	uint8	uiType;

	void	Init(uint8 uiType);
	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);
	bool	Is(uint8 uiType) const;
	bool	IsValid(void) const;
};


struct SSFTImage
{
	uint16	uiMagic;
	uint8	uiVersion;
	uint8	uiType;
	uint16	uiImageWidth;
	uint16	uiImageHeight;

	void	Init(uint8 uiType, uint16 uiImageWidth, uint16 uiImageHeight);
	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);
	bool	Is(uint8 uiType) const;
};


struct SSFTContainer
{
	uint16	uiMagic;
	uint8	uiVersion;
	uint8	uiType;
	uint16	uiCount;  //Count of contained records.

	void	Init(uint16 uiCount);
	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);
	bool	IsValid(void) const;
};


struct SSFTContained
{
	uint16	uiSkipToNext;  //How many bytes from the start of this struct to the next contained's (or zero if no more records contained).

	void	Init(void);
	void	Init(uint16 uiSkipToNext);
	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);
};  //end of struct == start of the contained record.


struct SSFTPalette
{
	uint16	uiMagic;
	uint8	uiVersion;
	uint8	uiType;
	uint16	uiFirst;
	uint16	uiCount;
	uint32	uiHash;

	void	Init(uint16 uiFirst, uint16 uiCount, uint32 uiHash);
	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);
	bool	IsValid(void) const;
};


//A palette's colours as they are read, and what an image's pixels are with them.
struct SSFTColours
{
	uint16	uiFirst;
	uint16	uiCount;
	uint8	auiRed[SFT_PALETTE_COLOURS];
	uint8	auiGreen[SFT_PALETTE_COLOURS];
	uint8	auiBlue[SFT_PALETTE_COLOURS];

	void	Init(void);
	bool	Read(CFileReader* pcFile);
	bool	Has(uint8 uiIndex) const;
	uint32	Colour32(uint8 uiIndex) const;
};


//FNV-1a (BaseLib/FNVHash.h) of a palette's uiFirst and uiCount (each low byte first) and then its reds, greens and
//blues as they are in the file: the same colours at the same indices have the same hash.
uint32	SFTPaletteHash(uint16 uiFirst, uint16 uiCount, const uint8* puiRed, const uint8* puiGreen, const uint8* puiBlue);

//A pixel with no palette (red D0-D2, green D3-D5, blue D6-D7) as red, green, blue and an opaque alpha, low byte first.
uint32	SFTColour32(uint8 uiPixel);

//The header of the record at the file's position, which is where it was after.
bool	PeekSFTHeader(CFileBasic* pcFile, SSFTHeader* psHeader);


#endif // __SFT_COMMON_H__
