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
//
//////////////////////////////////////////////////////////////////////////
void BMPWriteUInt16(uint8* pui, uint16 ui)
{
	pui[0] = (uint8)ui;
	pui[1] = (uint8)(ui >> 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void BMPWriteUInt32(uint8* pui, uint32 ui)
{
	pui[0] = (uint8)ui;
	pui[1] = (uint8)(ui >> 8);
	pui[2] = (uint8)(ui >> 16);
	pui[3] = (uint8)(ui >> 24);
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
	size			uiPixelOffset;
	size			uiFileSize;
	uint8*			pvPixels;
	uint8*			pvFile;
	uint8*			pvRow;
	uint8*			puiSource;
	CImage			cImageExport;
	CImageCopier	cCopier;
	CFileBasic		cFile;
	int				x;
	int				y;
	int				i;
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
	uiPixelOffset = BMP_FILE_HEADER_SIZE + uiHeaderSize + uiPaletteSize;
	uiFileSize = uiPixelOffset + uiStride * iHeight;
	pvFile = (uint8*)malloc(uiFileSize);
	memset(pvFile, 0, uiFileSize);

	pvFile[0] = 'B';
	pvFile[1] = 'M';
	BMPWriteUInt32(&pvFile[2], (uint32)uiFileSize);
	BMPWriteUInt32(&pvFile[10], (uint32)uiPixelOffset);

	BMPWriteUInt32(&pvFile[14], (uint32)uiHeaderSize);
	BMPWriteUInt32(&pvFile[18], (uint32)iWidth);
	BMPWriteUInt32(&pvFile[22], (uint32)iHeight);  //Positive height means the rows are stored bottom up.
	BMPWriteUInt16(&pvFile[26], 1);
	BMPWriteUInt16(&pvFile[28], (uint16)iBitsPerPixel);
	BMPWriteUInt32(&pvFile[30], bAlpha ? BMP_BITFIELDS : BMP_RGB);
	BMPWriteUInt32(&pvFile[34], (uint32)(uiStride * iHeight));
	BMPWriteUInt32(&pvFile[38], BMP_PIXELS_PER_METRE);
	BMPWriteUInt32(&pvFile[42], BMP_PIXELS_PER_METRE);
	BMPWriteUInt32(&pvFile[46], bGrey && !bAlpha ? 256 : 0);
	if (bAlpha)
	{
		BMPWriteUInt32(&pvFile[54], 0x00FF0000);
		BMPWriteUInt32(&pvFile[58], 0x0000FF00);
		BMPWriteUInt32(&pvFile[62], 0x000000FF);
		BMPWriteUInt32(&pvFile[66], 0xFF000000);
		BMPWriteUInt32(&pvFile[70], BMP_LCS_sRGB);
	}

	if (uiPaletteSize != 0)
	{
		for (i = 0; i < 256; i++)
		{
			pvFile[BMP_FILE_HEADER_SIZE + uiHeaderSize + i * 4] = (uint8)i;
			pvFile[BMP_FILE_HEADER_SIZE + uiHeaderSize + i * 4 + 1] = (uint8)i;
			pvFile[BMP_FILE_HEADER_SIZE + uiHeaderSize + i * 4 + 2] = (uint8)i;
		}
	}

	for (y = 0; y < iHeight; y++)
	{
		pvRow = &pvFile[uiPixelOffset + (iHeight - 1 - y) * uiStride];
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
		bResult = cFile.Write(pvFile, uiFileSize, 1) == 1;
		cFile.Close();
	}
	cFile.Kill();
	free(pvFile);

	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not write [", szPathName, "].", NULL);
	}
	return bResult;
}
