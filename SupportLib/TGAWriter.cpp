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
#include "BaseLib/LogString.h"
#include "ImageCopier.h"
#include "TGAWriter.h"


#define TGA_HEADER_SIZE			18
#define TGA_FOOTER_SIZE			26
#define TGA_TRUE_COLOUR			2
#define TGA_GREY				3
#define TGA_RLE					8
#define TGA_MAX_PACKET			128


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TGAWriteUInt16(uint8* pui, uint16 ui)
{
	pui[0] = (uint8)ui;
	pui[1] = (uint8)(ui >> 8);
}


//////////////////////////////////////////////////////////////////////////
//
// RLE compresses one row into puiDest and returns the number of bytes
// written.  Packets never cross rows.
//
//////////////////////////////////////////////////////////////////////////
size TGACompressRow(uint8* puiRow, int iWidth, int iPixelBytes, uint8* puiDest)
{
	size	uiPos;
	int		x;
	int		iRun;
	int		iRaw;

	uiPos = 0;
	x = 0;
	while (x < iWidth)
	{
		iRun = 1;
		while ((x + iRun < iWidth) && (iRun < TGA_MAX_PACKET) && (memcmp(&puiRow[x * iPixelBytes], &puiRow[(x + iRun) * iPixelBytes], iPixelBytes) == 0))
		{
			iRun++;
		}

		if (iRun > 1)
		{
			puiDest[uiPos] = (uint8)(0x80 | (iRun - 1));
			memcpy(&puiDest[uiPos + 1], &puiRow[x * iPixelBytes], iPixelBytes);
			uiPos += 1 + iPixelBytes;
			x += iRun;
		}
		else
		{
			//A raw packet ends where a run of two identical pixels starts.
			iRaw = 1;
			while ((x + iRaw < iWidth) && (iRaw < TGA_MAX_PACKET))
			{
				if ((x + iRaw + 1 < iWidth) && (memcmp(&puiRow[(x + iRaw) * iPixelBytes], &puiRow[(x + iRaw + 1) * iPixelBytes], iPixelBytes) == 0))
				{
					break;
				}
				iRaw++;
			}
			puiDest[uiPos] = (uint8)(iRaw - 1);
			memcpy(&puiDest[uiPos + 1], &puiRow[x * iPixelBytes], iRaw * iPixelBytes);
			uiPos += 1 + iRaw * iPixelBytes;
			x += iRaw;
		}
	}
	return uiPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveTGA(Ptr<CImage> pcImage, char* szPathName, bool bRLE)
{
	int				iWidth;
	int				iHeight;
	bool			bGrey;
	bool			bAlpha;
	int				iPixelBytes;
	size			uiRowBytes;
	size			uiMaxDataSize;
	size			uiFileSize;
	uint8*			pvPixels;
	uint8*			pvFile;
	uint8*			puiRow;
	uint8*			puiSource;
	uint8*			puiDest;
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
	if ((iWidth <= 0) || (iHeight <= 0) || (iWidth > 0xFFFF) || (iHeight > 0xFFFF))
	{
		gcLogger.Error2(__METHOD__, " Cannot write a [", IntToString(iWidth), " x ", IntToString(iHeight), "] image as a Targa [", szPathName, "].", NULL);
		return false;
	}

	if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP))
	{
		bGrey = false;
	}
	else if (pcImage->HasChannels(IMAGE_DIFFUSE_GREY, CHANNEL_STOP))
	{
		bGrey = true;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Image has no RGB or grey channels.  Cannot write [", szPathName, "].", NULL);
		return false;
	}
	bAlpha = pcImage->HasChannels(IMAGE_OPACITY, CHANNEL_STOP);
	iPixelBytes = (bGrey ? 1 : 3) + (bAlpha ? 1 : 0);

	//Export in Targa's byte order: blue, green, red, alpha or grey, alpha.
	pvPixels = (uint8*)malloc((size)iWidth * iHeight * iPixelBytes);
	if (pvPixels == NULL)
	{
		gcLogger.Error2(__METHOD__, " Could not allocate memory to write [", szPathName, "].", NULL);
		return false;
	}
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
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_STOP);
	}
	else
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_STOP);
	}
	cCopier.Init(pcImage, &cImageExport);
	cCopier.Copy(0, 0);
	cCopier.Kill();
	cImageExport.Kill();

	//The worst case RLE size is a one byte packet header for every pixel.
	uiRowBytes = (size)iWidth * iPixelBytes;
	uiMaxDataSize = bRLE ? (uiRowBytes + iWidth) * iHeight : uiRowBytes * iHeight;
	pvFile = (uint8*)malloc(TGA_HEADER_SIZE + uiMaxDataSize + TGA_FOOTER_SIZE);
	if (pvFile == NULL)
	{
		free(pvPixels);
		gcLogger.Error2(__METHOD__, " Could not allocate memory to write [", szPathName, "].", NULL);
		return false;
	}
	memset(pvFile, 0, TGA_HEADER_SIZE);
	pvFile[2] = (uint8)((bGrey ? TGA_GREY : TGA_TRUE_COLOUR) | (bRLE ? TGA_RLE : 0));
	TGAWriteUInt16(&pvFile[12], (uint16)iWidth);
	TGAWriteUInt16(&pvFile[14], (uint16)iHeight);
	pvFile[16] = (uint8)(iPixelBytes * 8);
	pvFile[17] = bAlpha ? 8 : 0;  //Alpha bits.  Rows are stored bottom up, left to right.

	puiDest = &pvFile[TGA_HEADER_SIZE];
	for (y = iHeight - 1; y >= 0; y--)
	{
		puiRow = &pvPixels[y * uiRowBytes];
		if (bRLE)
		{
			puiDest += TGACompressRow(puiRow, iWidth, iPixelBytes, puiDest);
		}
		else
		{
			for (x = 0; x < iWidth; x++)
			{
				puiSource = &puiRow[x * iPixelBytes];
				memcpy(puiDest, puiSource, iPixelBytes);
				puiDest += iPixelBytes;
			}
		}
	}
	free(pvPixels);

	//TGA 2.0 footer with no extension or developer areas.
	memset(puiDest, 0, 8);
	memcpy(&puiDest[8], "TRUEVISION-XFILE.", 18);
	puiDest += TGA_FOOTER_SIZE;
	uiFileSize = puiDest - pvFile;

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
