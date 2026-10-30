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
#include "BaseLib/FileBasic.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/NaiveFile.h"
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
	int32	iWidth;
	int32	iHeight;
	int		iPixelBits;
	int		iPixelBytes;
	int		iAlphaBits;
	bool	bTopToBottom;
	bool	bRightToLeft;
	lsize	uiColourMapOffset;
	lsize	uiPixelOffset;
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
// Reads one 15, 16, 24 or 32 bit colour (from the colour map or a true
// colour pixel) as red, green, blue and alpha.  The attribute bit of a 16
// bit colour is ignored.
//
//////////////////////////////////////////////////////////////////////////
bool TGAReadColour(CFileReader* pcFile, int iBits, uint8* puiRGBA)
{
	uint16	uiValue;
	uint8	auiBGRA[4];

	if ((iBits == 15) || (iBits == 16))
	{
		if (!pcFile->ReadLittleEndianInt16(&uiValue))
		{
			return false;
		}
		puiRGBA[0] = TGAScale5Bits((uiValue >> 10) & 0x1F);
		puiRGBA[1] = TGAScale5Bits((uiValue >> 5) & 0x1F);
		puiRGBA[2] = TGAScale5Bits(uiValue & 0x1F);
		puiRGBA[3] = 255;
	}
	else
	{
		if (!pcFile->ReadData(auiBGRA, iBits / 8))
		{
			return false;
		}
		puiRGBA[0] = auiBGRA[2];
		puiRGBA[1] = auiBGRA[1];
		puiRGBA[2] = auiBGRA[0];
		puiRGBA[3] = (iBits == 32) ? auiBGRA[3] : 255;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the fixed size header.  The image origin is skipped.
//
//////////////////////////////////////////////////////////////////////////
bool TGAReadHeader(CFileBasic* pcFile, STGAInfo* psInfo)
{
	uint8	uiIDLength;
	uint8	uiColourMapType;
	uint8	uiImageType;
	uint16	uiColourMapFirst;
	uint16	uiColourMapLength;
	uint8	uiColourMapEntryBits;
	uint16	uiWidth;
	uint16	uiHeight;
	uint8	uiPixelBits;
	uint8	uiDescriptor;

	if (!pcFile->ReadInt8(&uiIDLength) || !pcFile->ReadInt8(&uiColourMapType) || !pcFile->ReadInt8(&uiImageType) ||
		!pcFile->ReadLittleEndianInt16(&uiColourMapFirst) || !pcFile->ReadLittleEndianInt16(&uiColourMapLength) || !pcFile->ReadInt8(&uiColourMapEntryBits) ||
		!pcFile->Seek(12) || !pcFile->ReadLittleEndianInt16(&uiWidth) || !pcFile->ReadLittleEndianInt16(&uiHeight) || !pcFile->ReadInt8(&uiPixelBits) || !pcFile->ReadInt8(&uiDescriptor))
	{
		return false;
	}

	psInfo->iIDLength = uiIDLength;
	psInfo->iColourMapType = uiColourMapType;
	psInfo->iImageType = uiImageType;
	psInfo->iColourMapFirst = uiColourMapFirst;
	psInfo->iColourMapLength = uiColourMapLength;
	psInfo->iColourMapEntryBits = uiColourMapEntryBits;
	psInfo->iWidth = uiWidth;
	psInfo->iHeight = uiHeight;
	psInfo->iPixelBits = uiPixelBits;
	psInfo->iAlphaBits = uiDescriptor & TGA_ALPHA_BITS;
	psInfo->bRightToLeft = (uiDescriptor & TGA_RIGHT_TO_LEFT) != 0;
	psInfo->bTopToBottom = (uiDescriptor & TGA_TOP_TO_BOTTOM) != 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TGAReadInfo(CFileBasic* pcFile, lsize uiFileSize, STGAInfo* psInfo)
{
	int		iEntryBytes;
	int		i;

	psInfo->puiColourMap = NULL;
	if ((uiFileSize < TGA_HEADER_SIZE) || !TGAReadHeader(pcFile, psInfo))
	{
		return false;
	}

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
		psInfo->uiPixelOffset += (lsize)psInfo->iColourMapLength * iEntryBytes;
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
			psInfo->puiColourMap = (uint8*)malloc((lsize)psInfo->iColourMapLength * 4);
			if (psInfo->puiColourMap == NULL)
			{
				return false;
			}
			if (!pcFile->Seek(psInfo->uiColourMapOffset))
			{
				return false;
			}
			for (i = 0; i < psInfo->iColourMapLength; i++)
			{
				if (!TGAReadColour(pcFile, psInfo->iColourMapEntryBits, &psInfo->puiColourMap[i * 4]))
				{
					return false;
				}
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
// Reads one pixel from the current position of pcFile.
//
//////////////////////////////////////////////////////////////////////////
bool TGAReadPixel(CFileReader* pcFile, STGAInfo* psInfo, uint8* puiRGBA)
{
	uint8	uiIndex8;
	uint16	uiIndex16;
	int		iIndex;
	uint8	uiGrey;
	uint8	uiAlpha;

	if (psInfo->iBaseType == TGA_COLOUR_MAPPED)
	{
		if (psInfo->iPixelBits == 8)
		{
			if (!pcFile->ReadInt8(&uiIndex8))
			{
				return false;
			}
			iIndex = uiIndex8;
		}
		else
		{
			if (!pcFile->ReadLittleEndianInt16(&uiIndex16))
			{
				return false;
			}
			iIndex = uiIndex16;
		}

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
		return true;
	}
	else if (psInfo->iBaseType == TGA_TRUE_COLOUR)
	{
		return TGAReadColour(pcFile, psInfo->iPixelBits, puiRGBA);
	}
	else
	{
		if (!pcFile->ReadInt8(&uiGrey))
		{
			return false;
		}
		uiAlpha = 255;
		if ((psInfo->iPixelBits == 16) && !pcFile->ReadInt8(&uiAlpha))
		{
			return false;
		}
		puiRGBA[0] = uiGrey;
		puiRGBA[1] = uiGrey;
		puiRGBA[2] = uiGrey;
		puiRGBA[3] = uiAlpha;
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes the pixels into puiRGBA (4 bytes per pixel, top row first, left
// to right).
//
//////////////////////////////////////////////////////////////////////////
bool TGADecodePixels(CFileBasic* pcFile, STGAInfo* psInfo, uint8* puiRGBA)
{
	int					iNumPixels;
	int					iPixel;
	bool				bRepeat;
	uint8				auiPixel[4];
	int32				iFileRow;
	int					iFileColumn;
	int32				x;
	int32				y;
	CRLEPacketDecoder	cDecoder;
	bool				bResult;

	if (!pcFile->Seek(psInfo->uiPixelOffset))
	{
		return false;
	}

	iNumPixels = psInfo->iWidth * psInfo->iHeight;
	cDecoder.Init(pcFile);
	bResult = true;
	bRepeat = false;
	for (iPixel = 0; iPixel < iNumPixels; iPixel++)
	{
		if (psInfo->bRLE)
		{
			bResult = cDecoder.Next(&bRepeat);
			if (!bResult)
			{
				break;
			}
		}

		//Each pixel of a run after the first is the same, so is only read once.
		if (!bRepeat)
		{
			bResult = TGAReadPixel(pcFile, psInfo, auiPixel);
			if (!bResult)
			{
				break;
			}
		}

		iFileRow = iPixel / psInfo->iWidth;
		iFileColumn = iPixel % psInfo->iWidth;
		y = psInfo->bTopToBottom ? iFileRow : psInfo->iHeight - 1 - iFileRow;
		x = psInfo->bRightToLeft ? psInfo->iWidth - 1 - iFileColumn : iFileColumn;
		memcpy(&puiRGBA[(y * psInfo->iWidth + x) * 4], auiPixel, 4);
	}
	cDecoder.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadTGA(char* szFilename, bool bAddDebug)
{
	lsize			uiFileSize;
	CNaiveFile		cNaiveFile;
	uint8*			pvFile;
	CFileBasic		cFile;
	STGAInfo		sInfo;
	bool			bValid;
	bool			bDecoded;
	uint8*			puiRGBA;
	bool			bGrey;
	bool			bAlpha;
	int				i;
	Ptr<CImage>		pcImage;

	cNaiveFile.Init();
	if (!cNaiveFile.Read(szFilename))
	{
		cNaiveFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not read [", szFilename, "].", NULL);
		return NULL;
	}
	pvFile = (uint8*)cNaiveFile.Get();
	uiFileSize = (lsize)cNaiveFile.Size();

	sInfo.puiColourMap = NULL;
	puiRGBA = NULL;
	bDecoded = false;
	cFile.Init(MemoryFile(pvFile, uiFileSize));
	bValid = cFile.Open(EFM_Read);
	if (bValid)
	{
		bValid = TGAReadInfo(&cFile, uiFileSize, &sInfo);
		if (bValid)
		{
			puiRGBA = (uint8*)malloc((lsize)sInfo.iWidth * sInfo.iHeight * 4);
			bDecoded = (puiRGBA != NULL) && TGADecodePixels(&cFile, &sInfo, puiRGBA);
		}
		cFile.Close();
	}
	cFile.Kill();
	cNaiveFile.Kill();

	if (!bValid)
	{
		free(sInfo.puiColourMap);
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not a supported Targa image.", NULL);
		return NULL;
	}
	if (!bDecoded)
	{
		free(puiRGBA);
		free(sInfo.puiColourMap);
		gcLogger.Error2(__METHOD__, " [", szFilename, "] pixel data is truncated or corrupt.", NULL);
		return NULL;
	}

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

	if ((pcImage->GetData() == NULL) || !CopyRGBAIntoImage(puiRGBA, bGrey, pcImage))
	{
		free(puiRGBA);
		gcLogger.Error2(__METHOD__, " Could not create image for [", szFilename, "].", NULL);
		return NULL;
	}
	free(puiRGBA);

	return pcImage;
}
