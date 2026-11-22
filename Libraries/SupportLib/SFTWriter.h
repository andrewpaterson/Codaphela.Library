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
#ifndef __SFT_WRITER_H__
#define __SFT_WRITER_H__
#include "SFTCommon.h"
#include "Image.h"
#include "ImageCel.h"
#include "SFTQuantiser.h"


bool SaveSFT(Ptr<CImage> pImage, char* szPathName, bool bForceTransparent = false, lsize uiMergeGap = 0);
bool SaveSFTCelArray(Ptr<CArrayImageCel> pacImageCels, char* szFilename);

bool SaveSFTOpaque(Ptr<CImage> pImage, CFileBasic* pcFile, lsize uiImageLeftOffset, lsize uiImageTopOffset, uint16 uiCelWidth, uint16 uiCelHeight);
bool SaveSFTTransparent(Ptr<CImage> pImage, CFileBasic* pcFile, lsize uiImageLeftOffset, lsize uiImageTopOffset, uint16 uiCelWidth, uint16 uiCelHeight, lsize uiMergeGap = 0);
bool SaveSFTContainer(Ptr<CArrayImageCel> pacImageCels, CFileBasic* pcFile);

//A transparent image's record from its pixels and which of them are drawn (pabOpaque), each a row after another.
bool SaveSFTTransparentPixels(CFileBasic* pcFile, uint16 uiWidth, uint16 uiHeight, const uint8* puiPixels, const bool* pabOpaque, lsize uiMergeGap = 0);

//A container's record after its SSFTContained, at the next 64KB of the file if it would cross one; the record before
//(at *puiPrevious, unless bFirst) is given the distance to it.
bool AppendSFTContained(CFileBasic* pcFile, const void* pvRecord, lsize uiSize, filePos* puiPrevious, bool bFirst);

//Images palettised (QuantiseSFT, SFTQuantiser.h) as a container: their palette, then each image (SFTCommon.h).
bool SaveSFTQuantised(CSFTQuantised* pcQuantised, char* szFilename, lsize uiMergeGap = 0);

//A palette record (SFTCommon.h): uiCount colours (1 to 256) from index uiFirst, the reds, greens and blues each
//uiCount long, and their hash.
bool SaveSFTPalette(CFileBasic* pcFile, uint16 uiFirst, uint16 uiCount, const uint8* puiRed, const uint8* puiGreen, const uint8* puiBlue);


#endif // __SFT_WRITER_H__

