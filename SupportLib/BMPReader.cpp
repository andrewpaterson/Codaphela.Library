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
// Reads the file header, the bitmap header, the bit field masks and the
// palette.  Any read past the end of the file fails.
//
//////////////////////////////////////////////////////////////////////////
bool BMPReadInfo(CFileBasic* pcFile, size uiFileSize, SBMPInfo* psInfo)
{
	uint8	auiSignature[2];
	uint32	uiPixelOffset;
	uint32	uiHeaderSize;
	uint16	uiWidth;
	uint16	uiHeight;
	uint16	uiBitsPerPixel;
	int32	iWidth;
	int32	iHeight;
	size	uiPaletteOffset;
	size	uiPaletteEntrySize;
	uint32	uiColoursUsed;
	int		i;
	uint8	auiEntry[4];

	if ((uiFileSize < BMP_FILE_HEADER_SIZE + BMP_CORE_HEADER_SIZE) || !pcFile->ReadData(auiSignature, 2) || (auiSignature[0] != 'B') || (auiSignature[1] != 'M'))
	{
		return false;
	}

	//The file size and reserved fields are skipped.
	if (!pcFile->Seek(10) || !pcFile->ReadLittleEndianInt32(&uiPixelOffset) || !pcFile->ReadLittleEndianInt32(&uiHeaderSize))
	{
		return false;
	}
	psInfo->uiPixelOffset = uiPixelOffset;
	if ((uiHeaderSize < BMP_CORE_HEADER_SIZE) || (BMP_FILE_HEADER_SIZE + uiHeaderSize > uiFileSize))
	{
		return false;
	}

	memset(psInfo->auiMasks, 0, sizeof(psInfo->auiMasks));
	uiColoursUsed = 0;
	if (uiHeaderSize == BMP_CORE_HEADER_SIZE)
	{
		//OS/2 1.x BITMAPCOREHEADER.  The planes are skipped.
		if (!pcFile->ReadLittleEndianInt16(&uiWidth) || !pcFile->ReadLittleEndianInt16(&uiHeight) || !pcFile->Seek(24) || !pcFile->ReadLittleEndianInt16(&uiBitsPerPixel))
		{
			return false;
		}
		psInfo->iWidth = uiWidth;
		psInfo->iHeight = uiHeight;
		psInfo->iBitsPerPixel = uiBitsPerPixel;
		psInfo->uiCompression = BMP_RGB;
		uiPaletteEntrySize = 3;
	}
	else if (uiHeaderSize >= BMP_INFO_HEADER_SIZE)
	{
		//BITMAPINFOHEADER.  The planes, image size and resolution are skipped.
		if (!pcFile->ReadLittleEndianInt32(&iWidth) || !pcFile->ReadLittleEndianInt32(&iHeight) || !pcFile->Seek(28) || !pcFile->ReadLittleEndianInt16(&uiBitsPerPixel) ||
			!pcFile->ReadLittleEndianInt32(&psInfo->uiCompression) || !pcFile->Seek(46) || !pcFile->ReadLittleEndianInt32(&uiColoursUsed))
		{
			return false;
		}
		psInfo->iWidth = iWidth;
		psInfo->iHeight = iHeight;
		psInfo->iBitsPerPixel = uiBitsPerPixel;
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
		//The masks follow a BITMAPINFOHEADER or are part of the V2 to V5 headers.  Either way they start straight after the BITMAPINFOHEADER fields.
		if (!pcFile->Seek(BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE) || !pcFile->ReadLittleEndianInt32(&psInfo->auiMasks[0]) || !pcFile->ReadLittleEndianInt32(&psInfo->auiMasks[1]) || !pcFile->ReadLittleEndianInt32(&psInfo->auiMasks[2]))
		{
			return false;
		}
		if (uiHeaderSize == BMP_INFO_HEADER_SIZE)
		{
			uiPaletteOffset += 12;
			if (psInfo->uiCompression == BMP_ALPHABITFIELDS)
			{
				if (!pcFile->ReadLittleEndianInt32(&psInfo->auiMasks[3]))
				{
					return false;
				}
				uiPaletteOffset += 4;
			}
		}
		else if (uiHeaderSize >= BMP_V3_HEADER_SIZE)
		{
			if (!pcFile->ReadLittleEndianInt32(&psInfo->auiMasks[3]))
			{
				return false;
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
		if (!pcFile->Seek(uiPaletteOffset))
		{
			return false;
		}
		for (i = 0; i < psInfo->iNumColours; i++)
		{
			//Blue, green, red and (except for OS/2) reserved.
			if (!pcFile->ReadData(auiEntry, uiPaletteEntrySize))
			{
				return false;
			}
			psInfo->aauiPalette[i][0] = auiEntry[2];
			psInfo->aauiPalette[i][1] = auiEntry[1];
			psInfo->aauiPalette[i][2] = auiEntry[0];
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
// Looks up a palette index.  Indices past the end of the palette are
// treated as index 0.
//
//////////////////////////////////////////////////////////////////////////
void BMPPaletteColour(SBMPInfo* psInfo, int iIndex, uint8* puiRGBA)
{
	if (iIndex >= psInfo->iNumColours)
	{
		iIndex = 0;
	}
	puiRGBA[0] = psInfo->aauiPalette[iIndex][0];
	puiRGBA[1] = psInfo->aauiPalette[iIndex][1];
	puiRGBA[2] = psInfo->aauiPalette[iIndex][2];
	puiRGBA[3] = 255;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads one uncompressed row from the current position of pcFile into
// puiRGBA (4 bytes per pixel).  The row padding is not read.
//
//////////////////////////////////////////////////////////////////////////
bool BMPReadRow(CFileBasic* pcFile, SBMPInfo* psInfo, uint8* puiRGBA)
{
	int		x;
	uint8*	puiPixel;
	uint8	uiByte;
	uint8	auiBGR[3];
	uint16	uiPixel16;
	uint32	uiPixel;

	uiByte = 0;
	for (x = 0; x < psInfo->iWidth; x++)
	{
		puiPixel = &puiRGBA[x * 4];
		if (psInfo->iBitsPerPixel <= 8)
		{
			//Each byte holds one 8 bit, two 4 bit or eight 1 bit indices, most significant first.
			if ((psInfo->iBitsPerPixel == 8) || ((psInfo->iBitsPerPixel == 4) && ((x & 1) == 0)) || ((psInfo->iBitsPerPixel == 1) && ((x & 7) == 0)))
			{
				if (!pcFile->ReadInt8(&uiByte))
				{
					return false;
				}
			}

			if (psInfo->iBitsPerPixel == 8)
			{
				BMPPaletteColour(psInfo, uiByte, puiPixel);
			}
			else if (psInfo->iBitsPerPixel == 4)
			{
				BMPPaletteColour(psInfo, (x & 1) ? (uiByte & 0xF) : (uiByte >> 4), puiPixel);
			}
			else
			{
				BMPPaletteColour(psInfo, (uiByte >> (7 - (x & 7))) & 1, puiPixel);
			}
		}
		else if (psInfo->iBitsPerPixel == 24)
		{
			if (!pcFile->ReadData(auiBGR, 3))
			{
				return false;
			}
			puiPixel[0] = auiBGR[2];
			puiPixel[1] = auiBGR[1];
			puiPixel[2] = auiBGR[0];
			puiPixel[3] = 255;
		}
		else
		{
			if (psInfo->iBitsPerPixel == 16)
			{
				if (!pcFile->ReadLittleEndianInt16(&uiPixel16))
				{
					return false;
				}
				uiPixel = uiPixel16;
			}
			else
			{
				if (!pcFile->ReadLittleEndianInt32(&uiPixel))
				{
					return false;
				}
			}
			puiPixel[0] = BMPExtractChannel(uiPixel, psInfo->auiMasks[0]);
			puiPixel[1] = BMPExtractChannel(uiPixel, psInfo->auiMasks[1]);
			puiPixel[2] = BMPExtractChannel(uiPixel, psInfo->auiMasks[2]);
			puiPixel[3] = psInfo->auiMasks[3] ? BMPExtractChannel(uiPixel, psInfo->auiMasks[3]) : 255;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes RLE8 or RLE4 pixels into puiRGBA (4 bytes per pixel, top row
// first).  Pixels that are skipped are palette index 0.
//
//////////////////////////////////////////////////////////////////////////
bool BMPDecodeRLEPixels(CFileBasic* pcFile, SBMPInfo* psInfo, uint8* puiRGBA)
{
	uint8*				puiIndices;
	int					x;
	int					y;
	int					iFileRow;
	CRLEBitmapDecoder	cDecoder;
	bool				bResult;

	puiIndices = (uint8*)malloc(psInfo->iWidth * psInfo->iHeight);
	if (puiIndices == NULL)
	{
		return false;
	}
	memset(puiIndices, 0, psInfo->iWidth * psInfo->iHeight);

	cDecoder.Init(psInfo->iWidth, psInfo->iHeight, psInfo->uiCompression == BMP_RLE4);
	bResult = pcFile->Seek(psInfo->uiPixelOffset) && cDecoder.Decode(pcFile, puiIndices);
	cDecoder.Kill();

	if (bResult)
	{
		for (y = 0; y < psInfo->iHeight; y++)
		{
			iFileRow = psInfo->bTopDown ? y : psInfo->iHeight - 1 - y;
			for (x = 0; x < psInfo->iWidth; x++)
			{
				BMPPaletteColour(psInfo, puiIndices[iFileRow * psInfo->iWidth + x], &puiRGBA[(y * psInfo->iWidth + x) * 4]);
			}
		}
	}
	free(puiIndices);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes the pixels into puiRGBA (4 bytes per pixel, top row first).
//
//////////////////////////////////////////////////////////////////////////
bool BMPDecodePixels(CFileBasic* pcFile, size uiFileSize, SBMPInfo* psInfo, uint8* puiRGBA)
{
	size	uiStride;
	int		y;
	int		iFileRow;

	if ((psInfo->uiCompression == BMP_RLE8) || (psInfo->uiCompression == BMP_RLE4))
	{
		return BMPDecodeRLEPixels(pcFile, psInfo, puiRGBA);
	}

	uiStride = ((psInfo->iWidth * psInfo->iBitsPerPixel + 31) / 32) * 4;
	if (uiStride * psInfo->iHeight > uiFileSize - psInfo->uiPixelOffset)
	{
		return false;
	}

	for (y = 0; y < psInfo->iHeight; y++)
	{
		iFileRow = psInfo->bTopDown ? y : psInfo->iHeight - 1 - y;
		if (!pcFile->Seek(psInfo->uiPixelOffset + iFileRow * uiStride) || !BMPReadRow(pcFile, psInfo, &puiRGBA[y * psInfo->iWidth * 4]))
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
Ptr<CImage> LoadBMP(char* szFilename, bool bAddDebug)
{
	size			uiFileSize;
	CNaiveFile		cNaiveFile;
	uint8*			pvFile;
	CFileBasic		cFile;
	SBMPInfo		sInfo;
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
	uiFileSize = (size)cNaiveFile.Size();

	puiRGBA = NULL;
	bDecoded = false;
	cFile.Init(MemoryFile(pvFile, uiFileSize));
	bValid = cFile.Open(EFM_Read);
	if (bValid)
	{
		bValid = BMPReadInfo(&cFile, uiFileSize, &sInfo);
		if (bValid)
		{
			puiRGBA = (uint8*)malloc(sInfo.iWidth * sInfo.iHeight * 4);
			bDecoded = (puiRGBA != NULL) && BMPDecodePixels(&cFile, uiFileSize, &sInfo, puiRGBA);
		}
		cFile.Close();
	}
	cFile.Kill();
	cNaiveFile.Kill();

	if (!bValid)
	{
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not a supported bitmap.", NULL);
		return NULL;
	}
	if (!bDecoded)
	{
		free(puiRGBA);
		gcLogger.Error2(__METHOD__, " [", szFilename, "] pixel data is truncated or corrupt.", NULL);
		return NULL;
	}

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

	if ((pcImage->GetData() == NULL) || !CopyRGBAIntoImage(puiRGBA, bGrey, pcImage))
	{
		free(puiRGBA);
		gcLogger.Error2(__METHOD__, " Could not create image for [", szFilename, "].", NULL);
		return NULL;
	}
	free(puiRGBA);

	return pcImage;
}
