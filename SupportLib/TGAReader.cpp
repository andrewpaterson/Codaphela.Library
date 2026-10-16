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
#include "RLEPacketDecoder.h"
#include "ImageReaderCommon.h"
#include "TGAReader.h"


#define TGA_HEADER_SIZE			18

#define TGA_COLOUR_MAPPED		1
#define TGA_TRUE_COLOUR			2
#define TGA_GREY				3
#define TGA_RLE					8

#define TGA_RIGHT_TO_LEFT		0x10
#define TGA_TOP_TO_BOTTOM		0x20
#define TGA_ALPHA_BITS			0x0F

//RLE data can claim a huge image in a tiny file, so the size is limited (8192 x 8192).
#define TGA_MAX_PIXELS			0x4000000


struct STGAInfo
{
	int		iIDLength;
	int		iColourMapType;
	int		iImageType;
	int		iBaseType;
	bool	bRLE;
	int		iColourMapFirst;
	int		iColourMapLength;
	int		iColourMapEntryBits;
	int		iWidth;
	int		iHeight;
	int		iPixelBits;
	int		iPixelBytes;
	int		iAlphaBits;
	bool	bTopToBottom;
	bool	bRightToLeft;
	size	uiColourMapOffset;
	size	uiPixelOffset;
	uint8*	puiColourMap;	//Red, green, blue, alpha per entry.
};


//////////////////////////////////////////////////////////////////////////
//
// Scales a 5 bit channel to 8 bits the same way the image type converter
// does (truncating).
//
//////////////////////////////////////////////////////////////////////////
uint8 TGAScale5Bits(int iValue)
{
	return (uint8)((iValue * 255) / 31);
}


//////////////////////////////////////////////////////////////////////////
//
// Converts one 15, 16, 24 or 32 bit colour (from the colour map or a true
// colour pixel) to red, green, blue and alpha.  The attribute bit of a 16
// bit colour is ignored.
//
//////////////////////////////////////////////////////////////////////////
void TGADecodeColour(uint8* puiSource, int iBits, uint8* puiRGBA)
{
	uint16	uiValue;

	if ((iBits == 15) || (iBits == 16))
	{
		uiValue = ReadLittleEndianUInt16(puiSource);
		puiRGBA[0] = TGAScale5Bits((uiValue >> 10) & 0x1F);
		puiRGBA[1] = TGAScale5Bits((uiValue >> 5) & 0x1F);
		puiRGBA[2] = TGAScale5Bits(uiValue & 0x1F);
		puiRGBA[3] = 255;
	}
	else
	{
		puiRGBA[0] = puiSource[2];
		puiRGBA[1] = puiSource[1];
		puiRGBA[2] = puiSource[0];
		puiRGBA[3] = (iBits == 32) ? puiSource[3] : 255;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TGAReadInfo(uint8* pvFile, size uiFileSize, STGAInfo* psInfo)
{
	int		iEntryBytes;
	int		i;

	psInfo->puiColourMap = NULL;
	if (uiFileSize < TGA_HEADER_SIZE)
	{
		return false;
	}

	psInfo->iIDLength = pvFile[0];
	psInfo->iColourMapType = pvFile[1];
	psInfo->iImageType = pvFile[2];
	psInfo->iColourMapFirst = ReadLittleEndianUInt16(&pvFile[3]);
	psInfo->iColourMapLength = ReadLittleEndianUInt16(&pvFile[5]);
	psInfo->iColourMapEntryBits = pvFile[7];
	psInfo->iWidth = ReadLittleEndianUInt16(&pvFile[12]);
	psInfo->iHeight = ReadLittleEndianUInt16(&pvFile[14]);
	psInfo->iPixelBits = pvFile[16];
	psInfo->iAlphaBits = pvFile[17] & TGA_ALPHA_BITS;
	psInfo->bRightToLeft = (pvFile[17] & TGA_RIGHT_TO_LEFT) != 0;
	psInfo->bTopToBottom = (pvFile[17] & TGA_TOP_TO_BOTTOM) != 0;

	psInfo->bRLE = (psInfo->iImageType & TGA_RLE) != 0;
	psInfo->iBaseType = psInfo->iImageType & ~TGA_RLE;
	if ((psInfo->iImageType & ~(TGA_RLE | 3)) != 0)
	{
		return false;
	}
	if ((psInfo->iColourMapType != 0) && (psInfo->iColourMapType != 1))
	{
		return false;
	}
	if ((psInfo->iWidth <= 0) || (psInfo->iHeight <= 0) || ((uint64)psInfo->iWidth * (uint64)psInfo->iHeight > TGA_MAX_PIXELS))
	{
		return false;
	}

	switch (psInfo->iBaseType)
	{
	case TGA_COLOUR_MAPPED:
		if ((psInfo->iColourMapType != 1) || ((psInfo->iPixelBits != 8) && (psInfo->iPixelBits != 16)))
		{
			return false;
		}
		break;
	case TGA_TRUE_COLOUR:
		if ((psInfo->iPixelBits != 15) && (psInfo->iPixelBits != 16) && (psInfo->iPixelBits != 24) && (psInfo->iPixelBits != 32))
		{
			return false;
		}
		break;
	case TGA_GREY:
		if ((psInfo->iPixelBits != 8) && (psInfo->iPixelBits != 16))
		{
			return false;
		}
		break;
	default:
		return false;
	}
	psInfo->iPixelBytes = (psInfo->iPixelBits + 7) / 8;

	psInfo->uiColourMapOffset = TGA_HEADER_SIZE + psInfo->iIDLength;
	psInfo->uiPixelOffset = psInfo->uiColourMapOffset;
	if (psInfo->iColourMapType == 1)
	{
		//The colour map is skipped, but must still be valid, if the image isn't colour mapped.
		if ((psInfo->iColourMapEntryBits != 15) && (psInfo->iColourMapEntryBits != 16) && (psInfo->iColourMapEntryBits != 24) && (psInfo->iColourMapEntryBits != 32))
		{
			return false;
		}
		iEntryBytes = (psInfo->iColourMapEntryBits + 7) / 8;
		psInfo->uiPixelOffset += (size)psInfo->iColourMapLength * iEntryBytes;
		if (psInfo->uiPixelOffset > uiFileSize)
		{
			return false;
		}

		if (psInfo->iBaseType == TGA_COLOUR_MAPPED)
		{
			if (psInfo->iColourMapLength == 0)
			{
				return false;
			}
			psInfo->puiColourMap = (uint8*)malloc(psInfo->iColourMapLength * 4);
			if (psInfo->puiColourMap == NULL)
			{
				return false;
			}
			for (i = 0; i < psInfo->iColourMapLength; i++)
			{
				TGADecodeColour(&pvFile[psInfo->uiColourMapOffset + i * iEntryBytes], psInfo->iColourMapEntryBits, &psInfo->puiColourMap[i * 4]);
			}
		}
	}

	if (psInfo->uiPixelOffset > uiFileSize)
	{
		return false;
	}
	if (!psInfo->bRLE)
	{
		//Uncompressed pixels must all be in the file.
		if ((uint64)psInfo->iWidth * psInfo->iHeight * psInfo->iPixelBytes > uiFileSize - psInfo->uiPixelOffset)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TGADecodePixel(STGAInfo* psInfo, uint8* puiSource, uint8* puiRGBA)
{
	int		iIndex;

	if (psInfo->iBaseType == TGA_COLOUR_MAPPED)
	{
		iIndex = (psInfo->iPixelBits == 8) ? puiSource[0] : ReadLittleEndianUInt16(puiSource);
		iIndex -= psInfo->iColourMapFirst;
		if ((iIndex >= 0) && (iIndex < psInfo->iColourMapLength))
		{
			memcpy(puiRGBA, &psInfo->puiColourMap[iIndex * 4], 4);
		}
		else
		{
			puiRGBA[0] = 0;
			puiRGBA[1] = 0;
			puiRGBA[2] = 0;
			puiRGBA[3] = 255;
		}
	}
	else if (psInfo->iBaseType == TGA_TRUE_COLOUR)
	{
		TGADecodeColour(puiSource, psInfo->iPixelBits, puiRGBA);
	}
	else
	{
		puiRGBA[0] = puiSource[0];
		puiRGBA[1] = puiSource[0];
		puiRGBA[2] = puiSource[0];
		puiRGBA[3] = (psInfo->iPixelBits == 16) ? puiSource[1] : 255;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes the pixels into puiRGBA (4 bytes per pixel, top row first, left
// to right).
//
//////////////////////////////////////////////////////////////////////////
bool TGADecodePixels(uint8* pvFile, size uiFileSize, STGAInfo* psInfo, uint8* puiRGBA)
{
	uint8*				pvData;
	size				uiDataSize;
	int					iNumPixels;
	int					iPixel;
	uint8*				puiSource;
	uint8*				puiLastSource;
	uint8				auiPixel[4];
	int					iFileRow;
	int					iFileColumn;
	int					x;
	int					y;
	CRLEPacketDecoder	cDecoder;

	pvData = &pvFile[psInfo->uiPixelOffset];
	uiDataSize = uiFileSize - psInfo->uiPixelOffset;
	iNumPixels = psInfo->iWidth * psInfo->iHeight;
	cDecoder.Init(pvData, uiDataSize, psInfo->iPixelBytes);
	puiLastSource = NULL;
	for (iPixel = 0; iPixel < iNumPixels; iPixel++)
	{
		if (psInfo->bRLE)
		{
			puiSource = cDecoder.Next();
			if (puiSource == NULL)
			{
				cDecoder.Kill();
				return false;
			}
		}
		else
		{
			if ((size)(iPixel + 1) * psInfo->iPixelBytes > uiDataSize)
			{
				cDecoder.Kill();
				return false;
			}
			puiSource = &pvData[iPixel * psInfo->iPixelBytes];
		}

		//Each pixel of a run has the same source so only needs to be decoded once.
		if (puiSource != puiLastSource)
		{
			TGADecodePixel(psInfo, puiSource, auiPixel);
			puiLastSource = puiSource;
		}

		iFileRow = iPixel / psInfo->iWidth;
		iFileColumn = iPixel % psInfo->iWidth;
		y = psInfo->bTopToBottom ? iFileRow : psInfo->iHeight - 1 - iFileRow;
		x = psInfo->bRightToLeft ? psInfo->iWidth - 1 - iFileColumn : iFileColumn;
		memcpy(&puiRGBA[(y * psInfo->iWidth + x) * 4], auiPixel, 4);
	}
	cDecoder.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadTGA(char* szFilename, bool bAddDebug)
{
	size			uiFileSize;
	uint8*			pvFile;
	STGAInfo		sInfo;
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

	if (!TGAReadInfo(pvFile, uiFileSize, &sInfo))
	{
		free(sInfo.puiColourMap);
		free(pvFile);
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not a supported Targa image.", NULL);
		return NULL;
	}

	iNumPixels = sInfo.iWidth * sInfo.iHeight;
	puiRGBA = (uint8*)malloc(iNumPixels * 4);
	if ((puiRGBA == NULL) || !TGADecodePixels(pvFile, uiFileSize, &sInfo, puiRGBA))
	{
		free(puiRGBA);
		free(sInfo.puiColourMap);
		free(pvFile);
		gcLogger.Error2(__METHOD__, " [", szFilename, "] pixel data is truncated or corrupt.", NULL);
		return NULL;
	}
	free(pvFile);

	if (sInfo.iBaseType == TGA_COLOUR_MAPPED)
	{
		bGrey = true;
		bAlpha = false;
		for (i = 0; i < sInfo.iColourMapLength; i++)
		{
			if (sInfo.puiColourMap[i * 4 + 3] != 255)
			{
				bAlpha = true;
			}
			if ((sInfo.puiColourMap[i * 4] != sInfo.puiColourMap[i * 4 + 1]) || (sInfo.puiColourMap[i * 4] != sInfo.puiColourMap[i * 4 + 2]))
			{
				bGrey = false;
			}
		}
	}
	else if (sInfo.iBaseType == TGA_GREY)
	{
		bGrey = true;
		bAlpha = sInfo.iPixelBits == 16;
	}
	else
	{
		bGrey = false;
		bAlpha = (sInfo.iPixelBits == 32) && (sInfo.iAlphaBits != 0);
	}
	free(sInfo.puiColourMap);

	if (bGrey && bAlpha)
	{
		pcImage = OMalloc<CImage>(sInfo.iWidth, sInfo.iHeight, PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	}
	else if (bGrey)
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
		if (bGrey && bAlpha)
		{
			puiDest[i * 2] = puiRGBA[i * 4 + 3];
			puiDest[i * 2 + 1] = puiRGBA[i * 4];
		}
		else if (bGrey)
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
