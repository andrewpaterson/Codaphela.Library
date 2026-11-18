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


//An SFT file is an image or a container of them.  A pixel is a byte: red in D0-D2, green in D3-D5 and blue in D6-D7.
//
//An opaque image is an SSFTImage, then its rows from the top, each its width of pixels.
//
//A transparent image is an SSFTImage, then bytes that say where its pixels are, starting at its top left:
//  SFT_RUN_END              the image is done.
//  $01-$7F                  that many pixels follow: they are drawn from here to the right.
//  SFT_RUN_SKIP + (n - 1)   n pixels (1 to SFT_RUN_MAX_COUNT) are skipped to the right.
//  SFT_RUN_ROWS + (n - 1)   n rows (1 to SFT_RUN_MAX_ROWS) down, at the row's left.
//  SFT_RUN_MIXED + (n - 1)  n pixels (1 to SFT_RUN_MAX_MIXED) follow, of which those that are SFT_PIXEL_TRANSPARENT
//                           aren't drawn and the rest are, from here to the right: a run with short gaps in it, drawn
//                           as one copy that leaves a colour out (a blitter's transparent colour).  A pixel that is
//                           drawn and is SFT_PIXEL_TRANSPARENT's colour (black) can't be in one.
//A longer run, skip or move down is several of them.  What isn't drawn is transparent.  (SFT_TYPE_TRANSPARENT was $e4f9
//when a run was two words and its pixels, which is no longer read.  Moves down were up to 64 rows before the mixed runs
//took the upper half of their codes.)
//
//A container is an SSFTContainer, then its images, each after an SSFTContained.
#define SFT_TYPE_OPAQUE			(uint16)0xe457
#define SFT_TYPE_TRANSPARENT	(uint16)0xe4fa
#define SFT_TYPE_CONTAINER		(uint16)0xe4ef

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


struct SSFTImage
{
	uint16	uiType;
	uint16	uiImageWidth;
	uint16	uiImageHeight;

	void	Init(uint16 uiType, uint16 uiImageWidth, uint16 uiImageHeight);
	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);
};


struct SSFTContainer
{
	uint16	uiType;
	uint16	uiCount;  //Count of contained images.

	void	Init(uint16 uicount);
	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);
};


struct SSFTContained
{
	uint16	uiSkipToNext;  //How many bytes from the start of this struct to the next contained's (or zero if no more images contained).

	void	Init(void);
	void	Init(uint16 uiSkipToNext);
	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);
};  //end of struct == start of SSFT image type.


#endif // __SFT_COMMON_H__

