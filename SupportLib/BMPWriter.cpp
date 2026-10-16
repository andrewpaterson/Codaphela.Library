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
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/Logger.h"
#include "ImageCopier.h"
#include "BMPWriter.h"


#define BMP_FILE_HEADER_SIZE		14
#define BMP_INFO_HEADER_SIZE		40
#define BMP_V4_HEADER_SIZE			108
#define BMP_RGB						0
#define BMP_BITFIELDS				3
#define BMP_PIXELS_PER_METRE		2835		//72 DPI.
#define BMP_LCS_sRGB				0x73524742	//'sRGB'


//////////////////////////////////////////////////////////////////////////
//
// Writes the file header, the bitmap header and the palette (if any).
// The rows are stored bottom up.
//
//////////////////////////////////////////////////////////////////////////
bool BMPWriteHeaders(CFileWriter* pcFile, int iWidth, int iHeight, int iBitsPerPixel, size uiHeaderSize, size uiPaletteSize, size uiImageSize)
{
	size	uiPixelOffset;
	bool	bBitFields;
	uint8	auiZero[BMP_V4_HEADER_SIZE];
	uint32	uiGrey;
	bool	bResult;

	uiPixelOffset = BMP_FILE_HEADER_SIZE + uiHeaderSize + uiPaletteSize;
	bBitFields = uiHeaderSize == BMP_V4_HEADER_SIZE;
	memset(auiZero, 0, sizeof(auiZero));

	//BITMAPFILEHEADER.
	bResult =	pcFile->WriteInt8((uint8)'B') &&
				pcFile->WriteInt8((uint8)'M') &&
				pcFile->WriteInt32((uint32)(uiPixelOffset + uiImageSize)) &&
				pcFile->WriteInt32((uint32)0) &&
				pcFile->WriteInt32((uint32)uiPixelOffset);

	//BITMAPINFOHEADER.  Positive height means the rows are stored bottom up.
	bResult =	bResult &&
				pcFile->WriteInt32((uint32)uiHeaderSize) &&
				pcFile->WriteInt32((uint32)iWidth) &&
				pcFile->WriteInt32((uint32)iHeight) &&
				pcFile->WriteInt16((uint16)1) &&
				pcFile->WriteInt16((uint16)iBitsPerPixel) &&
				pcFile->WriteInt32((uint32)(bBitFields ? BMP_BITFIELDS : BMP_RGB)) &&
				pcFile->WriteInt32((uint32)uiImageSize) &&
				pcFile->WriteInt32((uint32)BMP_PIXELS_PER_METRE) &&
				pcFile->WriteInt32((uint32)BMP_PIXELS_PER_METRE) &&
				pcFile->WriteInt32((uint32)(uiPaletteSize / 4)) &&
				pcFile->WriteInt32((uint32)0);

	if (bBitFields)
	{
		//BITMAPV4HEADER masks and colour space.  The end points and gamma are unused for sRGB.
		bResult =	bResult &&
					pcFile->WriteInt32((uint32)0x00FF0000) &&
					pcFile->WriteInt32((uint32)0x0000FF00) &&
					pcFile->WriteInt32((uint32)0x000000FF) &&
					pcFile->WriteInt32((uint32)0xFF000000) &&
					pcFile->WriteInt32((uint32)BMP_LCS_sRGB) &&
					pcFile->WriteData(auiZero, BMP_V4_HEADER_SIZE - BMP_INFO_HEADER_SIZE - 20);
	}

	//Blue, green, red, reserved.
	for (uiGrey = 0; bResult && (uiGrey < uiPaletteSize / 4); uiGrey++)
	{
		bResult = pcFile->WriteInt32(uiGrey | (uiGrey << 8) | (uiGrey << 16));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveBMP(Ptr<CImage> pcImage, char* szPathName)
{
	int				iWidth;
	int				iHeight;
	bool			bGrey;
	bool			bAlpha;
	size			uiNumChannels;
	int				iBitsPerPixel;
	size			uiHeaderSize;
	size			uiPaletteSize;
	size			uiStride;
	size			uiImageSize;
	uint8*			pvPixels;
	uint8*			pvRows;
	uint8*			pvRow;
	uint8*			puiSource;
	CImage			cImageExport;
	CImageCopier	cCopier;
	CFileBasic		cFile;
	int				x;
	int				y;
	bool			bResult;

	if (pcImage.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Cannot write NULL image [", szPathName, "].", NULL);
		return false;
	}

	iWidth = pcImage->GetWidth();
	iHeight = pcImage->GetHeight();
	if ((iWidth <= 0) || (iHeight <= 0))
	{
		gcLogger.Error2(__METHOD__, " Cannot write empty image [", szPathName, "].", NULL);
		return false;
	}

	if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP))
	{
		bGrey = false;
		bAlpha = pcImage->HasChannels(IMAGE_OPACITY, CHANNEL_STOP);
	}
	else if (pcImage->HasChannels(IMAGE_DIFFUSE_GREY, CHANNEL_STOP))
	{
		bGrey = true;
		bAlpha = pcImage->HasChannels(IMAGE_OPACITY, CHANNEL_STOP);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Image has no RGB or grey channels.  Cannot write [", szPathName, "].", NULL);
		return false;
	}

	uiNumChannels = (bGrey ? 1 : 3) + (bAlpha ? 1 : 0);
	pvPixels = (uint8*)malloc(iWidth * iHeight * uiNumChannels);
	if (bGrey && bAlpha)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, CHANNEL_STOP);
	}
	else if (bGrey)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	}
	else if (bAlpha)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	}
	else
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	}
	cCopier.Init(pcImage, &cImageExport);
	cCopier.Copy(0, 0);
	cCopier.Kill();
	cImageExport.Kill();

	if (bAlpha)
	{
		//Alpha needs bit field masks, which BITMAPINFOHEADER only has for BI_ALPHABITFIELDS.  BITMAPV4HEADER is better supported.
		iBitsPerPixel = 32;
		uiHeaderSize = BMP_V4_HEADER_SIZE;
		uiPaletteSize = 0;
	}
	else if (bGrey)
	{
		iBitsPerPixel = 8;
		uiHeaderSize = BMP_INFO_HEADER_SIZE;
		uiPaletteSize = 256 * 4;
	}
	else
	{
		iBitsPerPixel = 24;
		uiHeaderSize = BMP_INFO_HEADER_SIZE;
		uiPaletteSize = 0;
	}

	uiStride = ((iWidth * iBitsPerPixel + 31) / 32) * 4;
	uiImageSize = uiStride * iHeight;
	pvRows = (uint8*)malloc(uiImageSize);
	memset(pvRows, 0, uiImageSize);

	for (y = 0; y < iHeight; y++)
	{
		pvRow = &pvRows[(iHeight - 1 - y) * uiStride];
		for (x = 0; x < iWidth; x++)
		{
			puiSource = &pvPixels[(y * iWidth + x) * uiNumChannels];
			if (bGrey && bAlpha)
			{
				pvRow[x * 4] = puiSource[0];
				pvRow[x * 4 + 1] = puiSource[0];
				pvRow[x * 4 + 2] = puiSource[0];
				pvRow[x * 4 + 3] = puiSource[1];
			}
			else if (bGrey)
			{
				pvRow[x] = puiSource[0];
			}
			else if (bAlpha)
			{
				pvRow[x * 4] = puiSource[2];
				pvRow[x * 4 + 1] = puiSource[1];
				pvRow[x * 4 + 2] = puiSource[0];
				pvRow[x * 4 + 3] = puiSource[3];
			}
			else
			{
				pvRow[x * 3] = puiSource[2];
				pvRow[x * 3 + 1] = puiSource[1];
				pvRow[x * 3 + 2] = puiSource[0];
			}
		}
	}
	free(pvPixels);

	cFile.Init(DiskFile(szPathName));
	bResult = cFile.Open(EFM_Write_Create);
	if (bResult)
	{
		bResult = BMPWriteHeaders(&cFile, iWidth, iHeight, iBitsPerPixel, uiHeaderSize, uiPaletteSize, uiImageSize) && cFile.WriteData(pvRows, uiImageSize);
		cFile.Close();
	}
	cFile.Kill();
	free(pvRows);

	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not write [", szPathName, "].", NULL);
	}
	return bResult;
}
