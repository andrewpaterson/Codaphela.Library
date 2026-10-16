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
#include "BaseLib/Logger.h"
#include "RLEBitmapDecoder.h"
#include "ImageReaderCommon.h"
#include "BMPReader.h"


#define BMP_FILE_HEADER_SIZE		14
#define BMP_CORE_HEADER_SIZE		12
#define BMP_INFO_HEADER_SIZE		40
#define BMP_V3_HEADER_SIZE			56

#define BMP_RGB						0
#define BMP_RLE8					1
#define BMP_RLE4					2
#define BMP_BITFIELDS				3
#define BMP_ALPHABITFIELDS			6

//RLE data can claim a huge image in a tiny file, so the size is limited (8192 x 8192).
#define BMP_MAX_PIXELS				0x4000000


struct SBMPInfo
{
	int		iWidth;
	int		iHeight;
	bool	bTopDown;
	int		iBitsPerPixel;
	uint32	uiCompression;
	uint32	auiMasks[4];			//Red, green, blue, alpha.
	int		iNumColours;
	uint8	aauiPalette[256][3];	//Red, green, blue.
	size	uiPixelOffset;
};


//////////////////////////////////////////////////////////////////////////
//
// Extracts the channel selected by uiMask from uiPixel and scales it to 8
// bits.  Channels narrower than 8 bits are scaled the same way the image
// type converter does (value * 255 / max, truncating).
//
//////////////////////////////////////////////////////////////////////////
uint8 BMPExtractChannel(uint32 uiPixel, uint32 uiMask)
{
	int		iShift;
	int		iBits;
	uint32	uiValue;

	if (uiMask == 0)
	{
		return 0;
	}

	iShift = 0;
	while (((uiMask >> iShift) & 1) == 0)
	{
		iShift++;
	}
	iBits = 0;
	while ((iShift + iBits < 32) && (((uiMask >> (iShift + iBits)) & 1) != 0))
	{
		iBits++;
	}

	uiValue = (uiPixel & uiMask) >> iShift;
	if (iBits >= 8)
	{
		return (uint8)(uiValue >> (iBits - 8));
	}
	return (uint8)((uiValue * 255) / ((1 << iBits) - 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool BMPReadInfo(uint8* pvFile, size uiFileSize, SBMPInfo* psInfo)
{
	uint32	uiHeaderSize;
	size	uiPaletteOffset;
	size	uiPaletteEntrySize;
	uint32	uiColoursUsed;
	int		i;
	uint8*	pvEntry;

	if ((uiFileSize < BMP_FILE_HEADER_SIZE + BMP_CORE_HEADER_SIZE) || (pvFile[0] != 'B') || (pvFile[1] != 'M'))
	{
		return false;
	}

	psInfo->uiPixelOffset = ReadLittleEndianUInt32(&pvFile[10]);
	uiHeaderSize = ReadLittleEndianUInt32(&pvFile[14]);
	if ((uiHeaderSize < BMP_CORE_HEADER_SIZE) || (BMP_FILE_HEADER_SIZE + uiHeaderSize > uiFileSize))
	{
		return false;
	}

	memset(psInfo->auiMasks, 0, sizeof(psInfo->auiMasks));
	uiColoursUsed = 0;
	if (uiHeaderSize == BMP_CORE_HEADER_SIZE)
	{
		//OS/2 1.x BITMAPCOREHEADER.
		psInfo->iWidth = ReadLittleEndianUInt16(&pvFile[18]);
		psInfo->iHeight = ReadLittleEndianUInt16(&pvFile[20]);
		psInfo->iBitsPerPixel = ReadLittleEndianUInt16(&pvFile[24]);
		psInfo->uiCompression = BMP_RGB;
		uiPaletteEntrySize = 3;
	}
	else if (uiHeaderSize >= BMP_INFO_HEADER_SIZE)
	{
		psInfo->iWidth = (int32)ReadLittleEndianUInt32(&pvFile[18]);
		psInfo->iHeight = (int32)ReadLittleEndianUInt32(&pvFile[22]);
		psInfo->iBitsPerPixel = ReadLittleEndianUInt16(&pvFile[28]);
		psInfo->uiCompression = ReadLittleEndianUInt32(&pvFile[30]);
		uiColoursUsed = ReadLittleEndianUInt32(&pvFile[46]);
		uiPaletteEntrySize = 4;
	}
	else
	{
		return false;
	}

	psInfo->bTopDown = psInfo->iHeight < 0;
	if (psInfo->bTopDown)
	{
		psInfo->iHeight = -psInfo->iHeight;
	}
	if ((psInfo->iWidth <= 0) || (psInfo->iHeight <= 0) || ((uint64)psInfo->iWidth * (uint64)psInfo->iHeight > BMP_MAX_PIXELS))
	{
		return false;
	}

	uiPaletteOffset = BMP_FILE_HEADER_SIZE + uiHeaderSize;
	if ((psInfo->uiCompression == BMP_BITFIELDS) || (psInfo->uiCompression == BMP_ALPHABITFIELDS))
	{
		if (uiHeaderSize == BMP_INFO_HEADER_SIZE)
		{
			//The masks follow a BITMAPINFOHEADER.
			if (uiPaletteOffset + 12 > uiFileSize)
			{
				return false;
			}
			psInfo->auiMasks[0] = ReadLittleEndianUInt32(&pvFile[uiPaletteOffset]);
			psInfo->auiMasks[1] = ReadLittleEndianUInt32(&pvFile[uiPaletteOffset + 4]);
			psInfo->auiMasks[2] = ReadLittleEndianUInt32(&pvFile[uiPaletteOffset + 8]);
			uiPaletteOffset += 12;
			if (psInfo->uiCompression == BMP_ALPHABITFIELDS)
			{
				if (uiPaletteOffset + 4 > uiFileSize)
				{
					return false;
				}
				psInfo->auiMasks[3] = ReadLittleEndianUInt32(&pvFile[uiPaletteOffset]);
				uiPaletteOffset += 4;
			}
		}
		else
		{
			//The masks are part of the V2 to V5 headers.
			psInfo->auiMasks[0] = ReadLittleEndianUInt32(&pvFile[54]);
			psInfo->auiMasks[1] = ReadLittleEndianUInt32(&pvFile[58]);
			psInfo->auiMasks[2] = ReadLittleEndianUInt32(&pvFile[62]);
			if (uiHeaderSize >= BMP_V3_HEADER_SIZE)
			{
				psInfo->auiMasks[3] = ReadLittleEndianUInt32(&pvFile[66]);
			}
		}
	}
	else if (psInfo->iBitsPerPixel == 16)
	{
		psInfo->auiMasks[0] = 0x7C00;
		psInfo->auiMasks[1] = 0x03E0;
		psInfo->auiMasks[2] = 0x001F;
	}
	else if (psInfo->iBitsPerPixel == 32)
	{
		//The fourth byte of a BI_RGB 32 bit pixel is reserved, not alpha.
		psInfo->auiMasks[0] = 0x00FF0000;
		psInfo->auiMasks[1] = 0x0000FF00;
		psInfo->auiMasks[2] = 0x000000FF;
	}

	switch (psInfo->iBitsPerPixel)
	{
	case 1:
	case 4:
	case 8:
		if (!((psInfo->uiCompression == BMP_RGB) ||
			  ((psInfo->uiCompression == BMP_RLE8) && (psInfo->iBitsPerPixel == 8)) ||
			  ((psInfo->uiCompression == BMP_RLE4) && (psInfo->iBitsPerPixel == 4))))
		{
			return false;
		}
		break;
	case 16:
	case 32:
		if (!((psInfo->uiCompression == BMP_RGB) || (psInfo->uiCompression == BMP_BITFIELDS) || (psInfo->uiCompression == BMP_ALPHABITFIELDS)))
		{
			return false;
		}
		break;
	case 24:
		if (psInfo->uiCompression != BMP_RGB)
		{
			return false;
		}
		break;
	default:
		return false;
	}

	psInfo->iNumColours = 0;
	if (psInfo->iBitsPerPixel <= 8)
	{
		psInfo->iNumColours = 1 << psInfo->iBitsPerPixel;
		if ((uiColoursUsed != 0) && (uiColoursUsed < (uint32)psInfo->iNumColours))
		{
			psInfo->iNumColours = (int)uiColoursUsed;
		}
		if (uiPaletteOffset + psInfo->iNumColours * uiPaletteEntrySize > uiFileSize)
		{
			return false;
		}
		memset(psInfo->aauiPalette, 0, sizeof(psInfo->aauiPalette));
		for (i = 0; i < psInfo->iNumColours; i++)
		{
			pvEntry = &pvFile[uiPaletteOffset + i * uiPaletteEntrySize];
			psInfo->aauiPalette[i][0] = pvEntry[2];
			psInfo->aauiPalette[i][1] = pvEntry[1];
			psInfo->aauiPalette[i][2] = pvEntry[0];
		}
	}

	if (psInfo->uiPixelOffset >= uiFileSize)
	{
		return false;
	}

	if ((psInfo->uiCompression != BMP_RLE8) && (psInfo->uiCompression != BMP_RLE4))
	{
		//Uncompressed rows must all be in the file.
		if ((uint64)(((psInfo->iWidth * (uint64)psInfo->iBitsPerPixel + 31) / 32) * 4) * psInfo->iHeight > uiFileSize - psInfo->uiPixelOffset)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes the pixels into puiRGBA (4 bytes per pixel, top row first).
//
//////////////////////////////////////////////////////////////////////////
bool BMPDecodePixels(uint8* pvFile, size uiFileSize, SBMPInfo* psInfo, uint8* puiRGBA)
{
	uint8*				pvData;
	size				uiDataSize;
	size				uiStride;
	int					x;
	int					y;
	int					iFileRow;
	uint8*				pvRow;
	uint8*				puiPixel;
	uint8*				puiIndices;
	int					iIndex;
	uint32				uiPixel;
	CRLEBitmapDecoder	cDecoder;

	pvData = &pvFile[psInfo->uiPixelOffset];
	uiDataSize = uiFileSize - psInfo->uiPixelOffset;

	puiIndices = NULL;
	if ((psInfo->uiCompression == BMP_RLE8) || (psInfo->uiCompression == BMP_RLE4))
	{
		puiIndices = (uint8*)malloc(psInfo->iWidth * psInfo->iHeight);
		if (puiIndices == NULL)
		{
			return false;
		}
		memset(puiIndices, 0, psInfo->iWidth * psInfo->iHeight);
		cDecoder.Init(psInfo->iWidth, psInfo->iHeight, psInfo->uiCompression == BMP_RLE4);
		if (!cDecoder.Decode(pvData, uiDataSize, puiIndices))
		{
			cDecoder.Kill();
			free(puiIndices);
			return false;
		}
		cDecoder.Kill();
		uiStride = 0;
	}
	else
	{
		uiStride = ((psInfo->iWidth * psInfo->iBitsPerPixel + 31) / 32) * 4;
		if (uiStride * psInfo->iHeight > uiDataSize)
		{
			return false;
		}
	}

	for (y = 0; y < psInfo->iHeight; y++)
	{
		iFileRow = psInfo->bTopDown ? y : psInfo->iHeight - 1 - y;
		pvRow = &pvData[iFileRow * uiStride];
		for (x = 0; x < psInfo->iWidth; x++)
		{
			puiPixel = &puiRGBA[(y * psInfo->iWidth + x) * 4];
			if (psInfo->iBitsPerPixel <= 8)
			{
				if (puiIndices)
				{
					iIndex = puiIndices[iFileRow * psInfo->iWidth + x];
				}
				else if (psInfo->iBitsPerPixel == 8)
				{
					iIndex = pvRow[x];
				}
				else if (psInfo->iBitsPerPixel == 4)
				{
					iIndex = (x & 1) ? (pvRow[x / 2] & 0xF) : (pvRow[x / 2] >> 4);
				}
				else
				{
					iIndex = (pvRow[x / 8] >> (7 - (x & 7))) & 1;
				}
				if (iIndex >= psInfo->iNumColours)
				{
					iIndex = 0;
				}
				puiPixel[0] = psInfo->aauiPalette[iIndex][0];
				puiPixel[1] = psInfo->aauiPalette[iIndex][1];
				puiPixel[2] = psInfo->aauiPalette[iIndex][2];
				puiPixel[3] = 255;
			}
			else if (psInfo->iBitsPerPixel == 24)
			{
				puiPixel[0] = pvRow[x * 3 + 2];
				puiPixel[1] = pvRow[x * 3 + 1];
				puiPixel[2] = pvRow[x * 3];
				puiPixel[3] = 255;
			}
			else
			{
				if (psInfo->iBitsPerPixel == 16)
				{
					uiPixel = ReadLittleEndianUInt16(&pvRow[x * 2]);
				}
				else
				{
					uiPixel = ReadLittleEndianUInt32(&pvRow[x * 4]);
				}
				puiPixel[0] = BMPExtractChannel(uiPixel, psInfo->auiMasks[0]);
				puiPixel[1] = BMPExtractChannel(uiPixel, psInfo->auiMasks[1]);
				puiPixel[2] = BMPExtractChannel(uiPixel, psInfo->auiMasks[2]);
				puiPixel[3] = psInfo->auiMasks[3] ? BMPExtractChannel(uiPixel, psInfo->auiMasks[3]) : 255;
			}
		}
	}

	if (puiIndices)
	{
		free(puiIndices);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadBMP(char* szFilename, bool bAddDebug)
{
	size			uiFileSize;
	uint8*			pvFile;
	SBMPInfo		sInfo;
	uint8*			puiRGBA;
	bool			bGrey;
	bool			bAlpha;
	int				i;
	int				iNumPixels;
	uint8*			puiDest;
	Ptr<CImage>		pcImage;

	pvFile = LoadFileIntoMemory(szFilename, &uiFileSize);
	if (pvFile == NULL)
	{
		return NULL;
	}

	if (!BMPReadInfo(pvFile, uiFileSize, &sInfo))
	{
		free(pvFile);
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not a supported bitmap.", NULL);
		return NULL;
	}

	iNumPixels = sInfo.iWidth * sInfo.iHeight;
	puiRGBA = (uint8*)malloc(iNumPixels * 4);
	if ((puiRGBA == NULL) || !BMPDecodePixels(pvFile, uiFileSize, &sInfo, puiRGBA))
	{
		free(puiRGBA);
		free(pvFile);
		gcLogger.Error2(__METHOD__, " [", szFilename, "] pixel data is truncated or corrupt.", NULL);
		return NULL;
	}
	free(pvFile);

	bGrey = false;
	bAlpha = false;
	if (sInfo.iBitsPerPixel <= 8)
	{
		bGrey = true;
		for (i = 0; i < sInfo.iNumColours; i++)
		{
			if ((sInfo.aauiPalette[i][0] != sInfo.aauiPalette[i][1]) || (sInfo.aauiPalette[i][0] != sInfo.aauiPalette[i][2]))
			{
				bGrey = false;
				break;
			}
		}
	}
	else
	{
		bAlpha = sInfo.auiMasks[3] != 0;
	}

	if (bGrey)
	{
		pcImage = OMalloc<CImage>(sInfo.iWidth, sInfo.iHeight, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	}
	else if (bAlpha)
	{
		pcImage = OMalloc<CImage>(sInfo.iWidth, sInfo.iHeight, PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	}
	else
	{
		pcImage = OMalloc<CImage>(sInfo.iWidth, sInfo.iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	}

	puiDest = (uint8*)pcImage->GetData();
	if (puiDest == NULL)
	{
		free(puiRGBA);
		gcLogger.Error2(__METHOD__, " Could not allocate image for [", szFilename, "].", NULL);
		return NULL;
	}
	for (i = 0; i < iNumPixels; i++)
	{
		if (bGrey)
		{
			puiDest[i] = puiRGBA[i * 4];
		}
		else if (bAlpha)
		{
			puiDest[i * 4] = puiRGBA[i * 4 + 3];
			memcpy(&puiDest[i * 4 + 1], &puiRGBA[i * 4], 3);
		}
		else
		{
			memcpy(&puiDest[i * 3], &puiRGBA[i * 4], 3);
		}
	}
	free(puiRGBA);

	return pcImage;
}
