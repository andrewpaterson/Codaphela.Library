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
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Inflater.h"
#include "ImageCopier.h"
#include "ImageReaderHelper.h"
#include "ImageReaderCommon.h"
#include "PNGCommon.h"
#include "PNGReader.h"


//Compressed data can claim a huge image in a tiny file, so the size is limited (8192 x 8192).
#define PNG_MAX_PIXELS			0x4000000

#define PNG_ADAM7_PASSES		7


struct SPNGInfo
{
	int		iWidth;
	int		iHeight;
	int		iBitDepth;
	int		iColourType;
	int		iInterlace;
	int		iBitsPerPixel;
	size	uiRowBytes;
	int		iNumPalette;
	uint8	aauiPalette[256][4];	//Red, green, blue, alpha.
	bool	bPaletteAlpha;
};


//Where each Adam7 pass starts and how far apart its pixels are.
static const int gaiAdam7StartX[PNG_ADAM7_PASSES] = { 0, 4, 0, 2, 0, 1, 0 };
static const int gaiAdam7StartY[PNG_ADAM7_PASSES] = { 0, 0, 4, 0, 2, 0, 1 };
static const int gaiAdam7StepX[PNG_ADAM7_PASSES] = { 8, 8, 4, 4, 2, 2, 1 };
static const int gaiAdam7StepY[PNG_ADAM7_PASSES] = { 8, 8, 8, 4, 4, 2, 2 };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size PNGRowBytes(int iWidth, int iBitsPerPixel)
{
	return ((size)iWidth * iBitsPerPixel + 7) / 8;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads and checks the image header.
//
//////////////////////////////////////////////////////////////////////////
bool PNGReadHeader(CFileBasic* pcFile, SPNGInfo* psInfo)
{
	uint32	uiWidth;
	uint32	uiHeight;
	uint8	uiBitDepth;
	uint8	uiColourType;
	uint8	uiCompression;
	uint8	uiFilter;
	uint8	uiInterlace;
	int		iSamples;

	if (!pcFile->ReadBigEndianInt32(&uiWidth) || !pcFile->ReadBigEndianInt32(&uiHeight) || !pcFile->ReadInt8(&uiBitDepth) || !pcFile->ReadInt8(&uiColourType) ||
		!pcFile->ReadInt8(&uiCompression) || !pcFile->ReadInt8(&uiFilter) || !pcFile->ReadInt8(&uiInterlace))
	{
		return false;
	}

	if ((uiWidth == 0) || (uiHeight == 0) || (uiWidth > 0x7FFFFFFF) || (uiHeight > 0x7FFFFFFF) || ((uint64)uiWidth * uiHeight > PNG_MAX_PIXELS))
	{
		return false;
	}
	if ((uiCompression != 0) || (uiFilter != 0) || (uiInterlace > PNG_INTERLACE_ADAM7))
	{
		return false;
	}

	switch (uiColourType)
	{
	case PNG_COLOUR_GREY:
		iSamples = 1;
		if ((uiBitDepth != 1) && (uiBitDepth != 2) && (uiBitDepth != 4) && (uiBitDepth != 8) && (uiBitDepth != 16))
		{
			return false;
		}
		break;
	case PNG_COLOUR_PALETTE:
		iSamples = 1;
		if ((uiBitDepth != 1) && (uiBitDepth != 2) && (uiBitDepth != 4) && (uiBitDepth != 8))
		{
			return false;
		}
		break;
	case PNG_COLOUR_RGB:
		iSamples = 3;
		break;
	case PNG_COLOUR_GREY_ALPHA:
		iSamples = 2;
		break;
	case PNG_COLOUR_RGB_ALPHA:
		iSamples = 4;
		break;
	default:
		return false;
	}
	if ((iSamples > 1) && (uiBitDepth != 8) && (uiBitDepth != 16))
	{
		return false;
	}

	psInfo->iWidth = (int)uiWidth;
	psInfo->iHeight = (int)uiHeight;
	psInfo->iBitDepth = uiBitDepth;
	psInfo->iColourType = uiColourType;
	psInfo->iInterlace = uiInterlace;
	psInfo->iBitsPerPixel = iSamples * uiBitDepth;
	psInfo->uiRowBytes = PNGRowBytes(psInfo->iWidth, psInfo->iBitsPerPixel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the chunks up to IEND.  The image data is gathered into
// pacCompressed.  Every chunk's CRC is checked.  Unknown ancillary chunks
// are skipped, unknown critical chunks fail.  The transparency of grey
// and RGB images isn't used.
//
//////////////////////////////////////////////////////////////////////////
bool PNGReadChunks(CFileBasic* pcFile, uint8* pvFile, size uiFileSize, SPNGInfo* psInfo, CArrayUint8* pacCompressed)
{
	uint8	auiSignature[PNG_SIGNATURE_SIZE];
	uint32	uiLength;
	uint8	auiType[4];
	uint32	uiCRC;
	size	uiDataPosition;
	bool	bHeader;
	bool	bData;
	int		i;

	if (!pcFile->ReadData(auiSignature, PNG_SIGNATURE_SIZE) || (memcmp(auiSignature, gauiPNGSignature, PNG_SIGNATURE_SIZE) != 0))
	{
		return false;
	}

	psInfo->iNumPalette = 0;
	psInfo->bPaletteAlpha = false;
	bHeader = false;
	bData = false;
	for (;;)
	{
		if (!pcFile->ReadBigEndianInt32(&uiLength) || !pcFile->ReadData(auiType, 4))
		{
			return false;
		}
		uiDataPosition = (size)pcFile->GetFilePos();
		if ((uiLength > 0x7FFFFFFF) || (uiLength > uiFileSize - uiDataPosition) || (uiFileSize - uiDataPosition - uiLength < 4))
		{
			return false;
		}
		if (!pcFile->Seek(uiDataPosition + uiLength) || !pcFile->ReadBigEndianInt32(&uiCRC) || (uiCRC != PNGChunkCRC(auiType, &pvFile[uiDataPosition], uiLength)))
		{
			return false;
		}
		if (!pcFile->Seek(uiDataPosition))
		{
			return false;
		}

		if (memcmp(auiType, "IHDR", 4) == 0)
		{
			if (bHeader || (uiLength != PNG_IHDR_SIZE) || !PNGReadHeader(pcFile, psInfo))
			{
				return false;
			}
			bHeader = true;
		}
		else if (!bHeader)
		{
			//IHDR must be first.
			return false;
		}
		else if (memcmp(auiType, "PLTE", 4) == 0)
		{
			if (bData || (psInfo->iNumPalette != 0) || (uiLength == 0) || (uiLength % 3 != 0) || (uiLength > 256 * 3))
			{
				return false;
			}
			psInfo->iNumPalette = uiLength / 3;
			for (i = 0; i < psInfo->iNumPalette; i++)
			{
				if (!pcFile->ReadData(psInfo->aauiPalette[i], 3))
				{
					return false;
				}
				psInfo->aauiPalette[i][3] = 255;
			}
		}
		else if (memcmp(auiType, "tRNS", 4) == 0)
		{
			if (psInfo->iColourType == PNG_COLOUR_PALETTE)
			{
				//One alpha per palette entry, missing ones are opaque.
				if (bData || (psInfo->iNumPalette == 0) || (uiLength > (uint32)psInfo->iNumPalette))
				{
					return false;
				}
				for (i = 0; i < (int)uiLength; i++)
				{
					if (!pcFile->ReadInt8(&psInfo->aauiPalette[i][3]))
					{
						return false;
					}
				}
				psInfo->bPaletteAlpha = uiLength > 0;
			}
		}
		else if (memcmp(auiType, "IDAT", 4) == 0)
		{
			if (uiLength > 0)
			{
				pacCompressed->Add(&pvFile[uiDataPosition], uiLength);
			}
			bData = true;
		}
		else if (memcmp(auiType, "IEND", 4) == 0)
		{
			return bData && ((psInfo->iColourType != PNG_COLOUR_PALETTE) || (psInfo->iNumPalette != 0));
		}
		else if ((auiType[0] & 0x20) == 0)
		{
			//A lower case first letter marks an ancillary chunk, which can be skipped.
			return false;
		}

		if (!pcFile->Seek(uiDataPosition + uiLength + 4))
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Undoes the filter on one row of iRowBytes bytes.  puiPrior is the
// unfiltered row above, or NULL for the first row.
//
//////////////////////////////////////////////////////////////////////////
bool PNGUnfilterRow(int iFilter, uint8* puiRow, uint8* puiPrior, size uiRowBytes, size uiPixelBytes)
{
	size	i;
	uint8	uiLeft;
	uint8	uiUp;
	uint8	uiUpLeft;

	if (iFilter == PNG_FILTER_NONE)
	{
		return true;
	}
	if (iFilter >= PNG_NUM_FILTERS)
	{
		return false;
	}

	for (i = 0; i < uiRowBytes; i++)
	{
		uiLeft = (i >= uiPixelBytes) ? puiRow[i - uiPixelBytes] : 0;
		uiUp = puiPrior ? puiPrior[i] : 0;
		uiUpLeft = (puiPrior && (i >= uiPixelBytes)) ? puiPrior[i - uiPixelBytes] : 0;
		switch (iFilter)
		{
		case PNG_FILTER_SUB:
			puiRow[i] = (uint8)(puiRow[i] + uiLeft);
			break;
		case PNG_FILTER_UP:
			puiRow[i] = (uint8)(puiRow[i] + uiUp);
			break;
		case PNG_FILTER_AVERAGE:
			puiRow[i] = (uint8)(puiRow[i] + (((int)uiLeft + (int)uiUp) >> 1));
			break;
		default:
			puiRow[i] = (uint8)(puiRow[i] + PNGPaethPredictor(uiLeft, uiUp, uiUpLeft));
			break;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Unfilters iHeight rows that each start with a filter type byte, in
// place.  Returns a pointer just past the last row.
//
//////////////////////////////////////////////////////////////////////////
uint8* PNGUnfilterRows(uint8* puiData, int iHeight, size uiRowBytes, size uiPixelBytes)
{
	int		y;
	uint8*	puiRow;
	uint8*	puiPrior;

	puiPrior = NULL;
	for (y = 0; y < iHeight; y++)
	{
		puiRow = &puiData[1];
		if (!PNGUnfilterRow(puiData[0], puiRow, puiPrior, uiRowBytes, uiPixelBytes))
		{
			return NULL;
		}
		puiPrior = puiRow;
		puiData += uiRowBytes + 1;
	}
	return puiData;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the pixel at x in puiSource to x in puiDest.  Pixels of less
// than 8 bits are packed most significant first.
//
//////////////////////////////////////////////////////////////////////////
void PNGCopyPixel(uint8* puiSource, int iSourceX, uint8* puiDest, int iDestX, int iBitsPerPixel)
{
	size	uiSourceBit;
	size	uiDestBit;
	uint8	uiMask;
	uint8	uiValue;

	if (iBitsPerPixel >= 8)
	{
		memcpy(&puiDest[iDestX * (iBitsPerPixel / 8)], &puiSource[iSourceX * (iBitsPerPixel / 8)], iBitsPerPixel / 8);
	}
	else
	{
		uiMask = (uint8)((1 << iBitsPerPixel) - 1);
		uiSourceBit = (size)iSourceX * iBitsPerPixel;
		uiDestBit = (size)iDestX * iBitsPerPixel;
		uiValue = (uint8)((puiSource[uiSourceBit / 8] >> (8 - iBitsPerPixel - (uiSourceBit % 8))) & uiMask);
		puiDest[uiDestBit / 8] |= (uint8)(uiValue << (8 - iBitsPerPixel - (uiDestBit % 8)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The size of the decompressed data: each row of each pass has a filter
// byte.
//
//////////////////////////////////////////////////////////////////////////
size PNGFilteredSize(SPNGInfo* psInfo)
{
	size	uiSize;
	int		iPass;
	int		iPassWidth;
	int		iPassHeight;

	if (psInfo->iInterlace == PNG_INTERLACE_NONE)
	{
		return (psInfo->uiRowBytes + 1) * psInfo->iHeight;
	}

	uiSize = 0;
	for (iPass = 0; iPass < PNG_ADAM7_PASSES; iPass++)
	{
		iPassWidth = (psInfo->iWidth - gaiAdam7StartX[iPass] + gaiAdam7StepX[iPass] - 1) / gaiAdam7StepX[iPass];
		iPassHeight = (psInfo->iHeight - gaiAdam7StartY[iPass] + gaiAdam7StepY[iPass] - 1) / gaiAdam7StepY[iPass];
		if ((iPassWidth > 0) && (iPassHeight > 0))
		{
			uiSize += (PNGRowBytes(iPassWidth, psInfo->iBitsPerPixel) + 1) * iPassHeight;
		}
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
// Unfilters (and de-interlaces) the decompressed data into puiPixels, one
// packed row of uiRowBytes per image row, as the PNG stores them.
//
//////////////////////////////////////////////////////////////////////////
bool PNGDecodeRows(uint8* puiFiltered, SPNGInfo* psInfo, uint8* puiPixels)
{
	size	uiPixelBytes;
	int		iPass;
	int		iPassWidth;
	int		iPassHeight;
	size	uiPassRowBytes;
	int		x;
	int		y;
	uint8*	puiPassRow;
	int		iDestY;

	uiPixelBytes = (psInfo->iBitsPerPixel + 7) / 8;
	if (psInfo->iInterlace == PNG_INTERLACE_NONE)
	{
		if (PNGUnfilterRows(puiFiltered, psInfo->iHeight, psInfo->uiRowBytes, uiPixelBytes) == NULL)
		{
			return false;
		}
		for (y = 0; y < psInfo->iHeight; y++)
		{
			memcpy(&puiPixels[y * psInfo->uiRowBytes], &puiFiltered[y * (psInfo->uiRowBytes + 1) + 1], psInfo->uiRowBytes);
		}
		return true;
	}

	memset(puiPixels, 0, psInfo->uiRowBytes * psInfo->iHeight);
	for (iPass = 0; iPass < PNG_ADAM7_PASSES; iPass++)
	{
		iPassWidth = (psInfo->iWidth - gaiAdam7StartX[iPass] + gaiAdam7StepX[iPass] - 1) / gaiAdam7StepX[iPass];
		iPassHeight = (psInfo->iHeight - gaiAdam7StartY[iPass] + gaiAdam7StepY[iPass] - 1) / gaiAdam7StepY[iPass];
		if ((iPassWidth <= 0) || (iPassHeight <= 0))
		{
			continue;
		}

		uiPassRowBytes = PNGRowBytes(iPassWidth, psInfo->iBitsPerPixel);
		if (PNGUnfilterRows(puiFiltered, iPassHeight, uiPassRowBytes, uiPixelBytes) == NULL)
		{
			return false;
		}
		for (y = 0; y < iPassHeight; y++)
		{
			puiPassRow = &puiFiltered[y * (uiPassRowBytes + 1) + 1];
			iDestY = gaiAdam7StartY[iPass] + y * gaiAdam7StepY[iPass];
			for (x = 0; x < iPassWidth; x++)
			{
				PNGCopyPixel(puiPassRow, x, &puiPixels[iDestY * psInfo->uiRowBytes], gaiAdam7StartX[iPass] + x * gaiAdam7StepX[iPass], psInfo->iBitsPerPixel);
			}
		}
		puiFiltered += (uiPassRowBytes + 1) * iPassHeight;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Expands palette indices to red, green, blue and alpha.  An index past
// the end of the palette is opaque black.
//
//////////////////////////////////////////////////////////////////////////
void PNGExpandPalette(SPNGInfo* psInfo, uint8* puiPixels, uint8* puiRGBA)
{
	int		x;
	int		y;
	uint8	uiIndex;
	uint8*	puiRow;
	uint8*	puiDest;
	size	uiBit;
	uint8	uiMask;

	uiMask = (uint8)((1 << psInfo->iBitDepth) - 1);
	for (y = 0; y < psInfo->iHeight; y++)
	{
		puiRow = &puiPixels[y * psInfo->uiRowBytes];
		for (x = 0; x < psInfo->iWidth; x++)
		{
			uiBit = (size)x * psInfo->iBitDepth;
			uiIndex = (uint8)((puiRow[uiBit / 8] >> (8 - psInfo->iBitDepth - (uiBit % 8))) & uiMask);
			puiDest = &puiRGBA[((size)y * psInfo->iWidth + x) * 4];
			if (uiIndex < psInfo->iNumPalette)
			{
				memcpy(puiDest, psInfo->aauiPalette[uiIndex], 4);
			}
			else
			{
				puiDest[0] = 0;
				puiDest[1] = 0;
				puiDest[2] = 0;
				puiDest[3] = 255;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Palettes that are all grey are read as grey.  Alpha from tRNS is read
// as opacity, the first channel.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> PNGCreatePaletteImage(SPNGInfo* psInfo, uint8* puiPixels, bool bAddDebug)
{
	uint8*			puiRGBA;
	bool			bGrey;
	bool			bAlpha;
	int				i;
	Ptr<CImage>		pcImage;

	bGrey = true;
	bAlpha = false;
	for (i = 0; i < psInfo->iNumPalette; i++)
	{
		if ((psInfo->aauiPalette[i][0] != psInfo->aauiPalette[i][1]) || (psInfo->aauiPalette[i][0] != psInfo->aauiPalette[i][2]))
		{
			bGrey = false;
		}
		if (psInfo->aauiPalette[i][3] != 255)
		{
			bAlpha = true;
		}
	}

	puiRGBA = (uint8*)malloc((size)psInfo->iWidth * psInfo->iHeight * 4);
	if (puiRGBA == NULL)
	{
		return NULL;
	}
	PNGExpandPalette(psInfo, puiPixels, puiRGBA);

	pcImage = OMalloc<CImage>();
	pcImage->BeginChange();
	if (bAlpha)
	{
		AddDebugChannel(pcImage, PT_uint8, IMAGE_OPACITY, bAddDebug);
	}
	if (bGrey)
	{
		AddDebugChannel(pcImage, PT_uint8, IMAGE_DIFFUSE_GREY, bAddDebug);
	}
	else
	{
		AddDebugChannel(pcImage, PT_uint8, IMAGE_DIFFUSE_RED, bAddDebug);
		AddDebugChannel(pcImage, PT_uint8, IMAGE_DIFFUSE_GREEN, bAddDebug);
		AddDebugChannel(pcImage, PT_uint8, IMAGE_DIFFUSE_BLUE, bAddDebug);
	}
	pcImage->SetSize(psInfo->iWidth, psInfo->iHeight);
	if (!pcImage->EndChange() || !CopyRGBAIntoImage(puiRGBA, bGrey, pcImage))
	{
		pcImage = NULL;
	}
	free(puiRGBA);
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Grey, grey with alpha, RGB and RGBA images keep their bit depth (1, 2
// and 4 bit grey are read as 8 bit).  Opacity is the first channel.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> PNGCreateImage(SPNGInfo* psInfo, uint8* puiPixels, bool bAddDebug)
{
	EPrimitiveType		eSourceType;
	bool				bReverse;
	CImage				cImageImport;
	CImageCopier		cCopier;
	int					y;
	bool				bResult;
	Ptr<CImage>			pcImage;

	//Samples are stored most significant bit (and byte) first.
	bReverse = false;
	switch (psInfo->iBitDepth)
	{
	case 1:
		eSourceType = PT_bit;
		bReverse = true;
		break;
	case 2:
		eSourceType = PT_crumb;
		break;
	case 4:
		eSourceType = PT_nybble;
		break;
	case 8:
		eSourceType = PT_uint8;
		break;
	default:
		eSourceType = PT_uint16;
		bReverse = true;
		break;
	}

	pcImage = OMalloc<CImage>();
	pcImage->BeginChange();
	cImageImport.Init();
	cImageImport.BeginChange();
	if (psInfo->iColourType == PNG_COLOUR_GREY)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_GREY, eSourceType, bReverse);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_GREY, bAddDebug);
	}
	else if (psInfo->iColourType == PNG_COLOUR_RGB)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, eSourceType, bReverse);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_RED, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_GREEN, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_BLUE, bAddDebug);
	}
	else if (psInfo->iColourType == PNG_COLOUR_RGB_ALPHA)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, eSourceType, bReverse);
		AddDebugChannel(pcImage, eSourceType, IMAGE_OPACITY, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_RED, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_GREEN, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_BLUE, bAddDebug);
	}
	else
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, eSourceType, bReverse);
		AddDebugChannel(pcImage, eSourceType, IMAGE_OPACITY, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_GREY, bAddDebug);
	}

	pcImage->ByteAlignChannels();
	pcImage->SetSize(psInfo->iWidth, psInfo->iHeight);
	bResult = pcImage->EndChange();

	//The import image is one row, moved down the decoded rows.
	cImageImport.SetData(puiPixels);
	cImageImport.SetSize(psInfo->iWidth, psInfo->iHeight);
	cImageImport.EndChange();

	bResult = bResult && cCopier.Init(&cImageImport, pcImage);
	if (bResult)
	{
		for (y = 0; y < psInfo->iHeight; y++)
		{
			cImageImport.SetData(&puiPixels[y * psInfo->uiRowBytes]);
			cCopier.Copy(0, y, 0, 0, psInfo->iWidth, 1);
		}
	}
	cCopier.Kill();
	cImageImport.Kill();

	if (!bResult)
	{
		return NULL;
	}
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Decompresses and decodes the image data into puiPixels.
//
//////////////////////////////////////////////////////////////////////////
bool PNGDecodeImage(CArrayUint8* pacCompressed, SPNGInfo* psInfo, uint8* puiPixels)
{
	CFileBasic		cFile;
	CInflater		cInflater;
	CArrayUint8		acFiltered;
	size			uiFilteredSize;
	bool			bResult;

	uiFilteredSize = PNGFilteredSize(psInfo);
	acFiltered.Init();
	cFile.Init(MemoryFile(pacCompressed->GetData(), pacCompressed->NumElements()));
	bResult = cFile.Open(EFM_Read);
	if (bResult)
	{
		cInflater.Init(&cFile, true, uiFilteredSize);
		bResult = cInflater.Inflate(&acFiltered) && (acFiltered.NumElements() == uiFilteredSize);
		cInflater.Kill();
		cFile.Close();
	}
	cFile.Kill();

	bResult = bResult && PNGDecodeRows(acFiltered.GetData(), psInfo, puiPixels);
	acFiltered.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadPNG(char* szFilename, bool bAddDebug)
{
	CNaiveFile		cNaiveFile;
	uint8*			pvFile;
	size			uiFileSize;
	CFileBasic		cFile;
	SPNGInfo		sInfo;
	CArrayUint8		acCompressed;
	uint8*			puiPixels;
	bool			bValid;
	bool			bDecoded;
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

	acCompressed.Init();
	cFile.Init(MemoryFile(pvFile, uiFileSize));
	bValid = cFile.Open(EFM_Read);
	if (bValid)
	{
		bValid = PNGReadChunks(&cFile, pvFile, uiFileSize, &sInfo, &acCompressed);
		cFile.Close();
	}
	cFile.Kill();
	cNaiveFile.Kill();

	if (!bValid)
	{
		acCompressed.Kill();
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not a supported PNG image.", NULL);
		return NULL;
	}

	puiPixels = (uint8*)malloc(sInfo.uiRowBytes * sInfo.iHeight);
	bDecoded = (puiPixels != NULL) && PNGDecodeImage(&acCompressed, &sInfo, puiPixels);
	acCompressed.Kill();
	if (!bDecoded)
	{
		free(puiPixels);
		gcLogger.Error2(__METHOD__, " [", szFilename, "] image data is truncated or corrupt.", NULL);
		return NULL;
	}

	if (sInfo.iColourType == PNG_COLOUR_PALETTE)
	{
		pcImage = PNGCreatePaletteImage(&sInfo, puiPixels, bAddDebug);
	}
	else
	{
		pcImage = PNGCreateImage(&sInfo, puiPixels, bAddDebug);
	}
	free(puiPixels);

	if (pcImage.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Could not create image for [", szFilename, "].", NULL);
	}
	return pcImage;
}

