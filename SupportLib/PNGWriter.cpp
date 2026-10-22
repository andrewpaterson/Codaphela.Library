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
#include "BaseLib/MemoryFile.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Deflater.h"
#include "ImageCopier.h"
#include "PNGCommon.h"
#include "PNGWriter.h"


//The image data is split into chunks of this size, as most encoders do.
#define PNG_IDAT_CHUNK_SIZE		8192


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsPNGChannel16Bit(Ptr<CImage> pcImage, size iChannel)
{
	CChannel*	pcChannel;

	pcChannel = pcImage->GetChannel(iChannel);
	return (pcChannel != NULL) && (pcChannel->miByteSize >= 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Filters puiRow into puiDest with iFilter.  puiPrior is the row above,
// or NULL for the first row.
//
//////////////////////////////////////////////////////////////////////////
void PNGFilterRow(int iFilter, uint8* puiRow, uint8* puiPrior, size uiRowBytes, size uiPixelBytes, uint8* puiDest)
{
	size	i;
	uint8	uiLeft;
	uint8	uiUp;
	uint8	uiUpLeft;

	for (i = 0; i < uiRowBytes; i++)
	{
		uiLeft = (i >= uiPixelBytes) ? puiRow[i - uiPixelBytes] : 0;
		uiUp = puiPrior ? puiPrior[i] : 0;
		uiUpLeft = (puiPrior && (i >= uiPixelBytes)) ? puiPrior[i - uiPixelBytes] : 0;
		switch (iFilter)
		{
		case PNG_FILTER_NONE:
			puiDest[i] = puiRow[i];
			break;
		case PNG_FILTER_SUB:
			puiDest[i] = (uint8)(puiRow[i] - uiLeft);
			break;
		case PNG_FILTER_UP:
			puiDest[i] = (uint8)(puiRow[i] - uiUp);
			break;
		case PNG_FILTER_AVERAGE:
			puiDest[i] = (uint8)(puiRow[i] - (((int)uiLeft + (int)uiUp) >> 1));
			break;
		default:
			puiDest[i] = (uint8)(puiRow[i] - PNGPaethPredictor(uiLeft, uiUp, uiUpLeft));
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Filters every row with whichever filter gives the smallest sum of
// absolute values (as signed bytes), the usual heuristic.  Each filtered
// row starts with its filter type.
//
//////////////////////////////////////////////////////////////////////////
void PNGFilterRows(uint8* puiPixels, int iHeight, size uiRowBytes, size uiPixelBytes, uint8* puiFiltered, uint8* puiTrial)
{
	int		y;
	int		iFilter;
	int		iBestFilter;
	size	uiSum;
	size	uiBestSum;
	size	i;
	uint8*	puiRow;
	uint8*	puiPrior;

	puiPrior = NULL;
	for (y = 0; y < iHeight; y++)
	{
		puiRow = &puiPixels[y * uiRowBytes];
		iBestFilter = PNG_FILTER_NONE;
		uiBestSum = (size)-1;
		for (iFilter = 0; iFilter < PNG_NUM_FILTERS; iFilter++)
		{
			PNGFilterRow(iFilter, puiRow, puiPrior, uiRowBytes, uiPixelBytes, puiTrial);
			uiSum = 0;
			for (i = 0; i < uiRowBytes; i++)
			{
				uiSum += (puiTrial[i] < 128) ? puiTrial[i] : 256 - puiTrial[i];
			}
			if (uiSum < uiBestSum)
			{
				uiBestSum = uiSum;
				iBestFilter = iFilter;
			}
		}

		puiFiltered[0] = (uint8)iBestFilter;
		PNGFilterRow(iBestFilter, puiRow, puiPrior, uiRowBytes, uiPixelBytes, &puiFiltered[1]);
		puiFiltered += uiRowBytes + 1;
		puiPrior = puiRow;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the signature, IHDR, the compressed image data split into IDAT
// chunks and IEND.
//
//////////////////////////////////////////////////////////////////////////
bool PNGWriteFile(CFileBasic* pcFile, int iWidth, int iHeight, int iBitDepth, int iColourType, uint8* puiCompressed, size uiCompressedSize)
{
	CMemoryFile		cHeaderMemory;
	CFileBasic		cHeader;
	bool			bResult;
	size			uiPosition;
	size			uiChunk;

	cHeaderMemory.Init();
	cHeader.Init(&cHeaderMemory);
	bResult =	cHeader.Open(EFM_Write_Create) &&
				cHeader.WriteBigEndianInt32((uint32)iWidth) &&
				cHeader.WriteBigEndianInt32((uint32)iHeight) &&
				cHeader.WriteInt8((uint8)iBitDepth) &&
				cHeader.WriteInt8((uint8)iColourType) &&
				cHeader.WriteInt8((uint8)0) &&						//Deflate compression.
				cHeader.WriteInt8((uint8)0) &&						//Adaptive filtering.
				cHeader.WriteInt8((uint8)PNG_INTERLACE_NONE);
	cHeader.Close();

	bResult =	bResult &&
				pcFile->WriteData(gauiPNGSignature, PNG_SIGNATURE_SIZE) &&
				PNGWriteChunk(pcFile, "IHDR", (uint8*)cHeaderMemory.GetBufferPointer(), cHeaderMemory.GetBufferSize());
	cHeader.Kill();
	cHeaderMemory.Kill();

	uiPosition = 0;
	while (bResult && (uiPosition < uiCompressedSize))
	{
		uiChunk = uiCompressedSize - uiPosition;
		if (uiChunk > PNG_IDAT_CHUNK_SIZE)
		{
			uiChunk = PNG_IDAT_CHUNK_SIZE;
		}
		bResult = PNGWriteChunk(pcFile, "IDAT", &puiCompressed[uiPosition], uiChunk);
		uiPosition += uiChunk;
	}

	return bResult && PNGWriteChunk(pcFile, "IEND", NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The image is copied into RGBA, RGB, grey with alpha or grey.  Samples
// are 16 bit if any of those channels is 16 bit or wider, otherwise 8 bit.
//
//////////////////////////////////////////////////////////////////////////
bool SavePNG(Ptr<CImage> pcImage, char* szPathName)
{
	int				iWidth;
	int				iHeight;
	int				iColourType;
	size			aiChannels[4];
	size			uiNumChannels;
	size			uiBytesPerSample;
	size			uiRowBytes;
	size			uiPixelBytes;
	EPrimitiveType	eSampleType;
	uint8*			pvPixels;
	uint8*			pvFiltered;
	uint8*			pvTrial;
	CImage			cImageExport;
	CImageCopier	cCopier;
	CMemoryFile		cCompressedMemory;
	CFileBasic		cCompressed;
	CDeflater		cDeflater;
	CFileBasic		cFile;
	size			i;
	uint8			uiSwap;
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

	if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP))
	{
		iColourType = PNG_COLOUR_RGB_ALPHA;
		aiChannels[0] = IMAGE_DIFFUSE_RED;
		aiChannels[1] = IMAGE_DIFFUSE_GREEN;
		aiChannels[2] = IMAGE_DIFFUSE_BLUE;
		aiChannels[3] = IMAGE_OPACITY;
		uiNumChannels = 4;
	}
	else if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP))
	{
		iColourType = PNG_COLOUR_RGB;
		aiChannels[0] = IMAGE_DIFFUSE_RED;
		aiChannels[1] = IMAGE_DIFFUSE_GREEN;
		aiChannels[2] = IMAGE_DIFFUSE_BLUE;
		uiNumChannels = 3;
	}
	else if (pcImage->HasChannels(IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, CHANNEL_STOP))
	{
		iColourType = PNG_COLOUR_GREY_ALPHA;
		aiChannels[0] = IMAGE_DIFFUSE_GREY;
		aiChannels[1] = IMAGE_OPACITY;
		uiNumChannels = 2;
	}
	else if (pcImage->HasChannels(IMAGE_DIFFUSE_GREY, CHANNEL_STOP))
	{
		iColourType = PNG_COLOUR_GREY;
		aiChannels[0] = IMAGE_DIFFUSE_GREY;
		uiNumChannels = 1;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Image has no RGB or grey channels.  Cannot write [", szPathName, "].", NULL);
		return false;
	}

	eSampleType = PT_uint8;
	uiBytesPerSample = 1;
	for (i = 0; i < uiNumChannels; i++)
	{
		if (IsPNGChannel16Bit(pcImage, aiChannels[i]))
		{
			eSampleType = PT_uint16;
			uiBytesPerSample = 2;
			break;
		}
	}

	uiPixelBytes = uiNumChannels * uiBytesPerSample;
	uiRowBytes = iWidth * uiPixelBytes;
	pvPixels = (uint8*)malloc(uiRowBytes * iHeight);
	pvFiltered = (uint8*)malloc((uiRowBytes + 1) * iHeight);
	pvTrial = (uint8*)malloc(uiRowBytes);
	if ((pvPixels == NULL) || (pvFiltered == NULL) || (pvTrial == NULL))
	{
		free(pvTrial);
		free(pvFiltered);
		free(pvPixels);
		gcLogger.Error2(__METHOD__, " Could not allocate memory to write [", szPathName, "].", NULL);
		return false;
	}

	if (uiNumChannels == 4)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, eSampleType, aiChannels[0], aiChannels[1], aiChannels[2], aiChannels[3], CHANNEL_STOP);
	}
	else if (uiNumChannels == 3)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, eSampleType, aiChannels[0], aiChannels[1], aiChannels[2], CHANNEL_STOP);
	}
	else if (uiNumChannels == 2)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, eSampleType, aiChannels[0], aiChannels[1], CHANNEL_STOP);
	}
	else
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, eSampleType, aiChannels[0], CHANNEL_STOP);
	}
	cCopier.Init(pcImage, &cImageExport);
	cCopier.Copy(0, 0);
	cCopier.Kill();
	cImageExport.Kill();

	if (uiBytesPerSample == 2)
	{
		//PNG stores 16 bit samples most significant byte first.
		for (i = 0; i < uiRowBytes * iHeight; i += 2)
		{
			uiSwap = pvPixels[i];
			pvPixels[i] = pvPixels[i + 1];
			pvPixels[i + 1] = uiSwap;
		}
	}

	PNGFilterRows(pvPixels, iHeight, uiRowBytes, uiPixelBytes, pvFiltered, pvTrial);
	free(pvTrial);
	free(pvPixels);

	cCompressedMemory.Init();
	cCompressed.Init(&cCompressedMemory);
	bResult = cCompressed.Open(EFM_Write_Create) && cDeflater.Init(&cCompressed, true);
	if (bResult)
	{
		bResult = cDeflater.Deflate(pvFiltered, (uiRowBytes + 1) * iHeight);
		cDeflater.Kill();
	}
	cCompressed.Close();
	free(pvFiltered);

	if (bResult)
	{
		cFile.Init(DiskFile(szPathName));
		bResult = cFile.Open(EFM_Write_Create);
		if (bResult)
		{
			bResult = PNGWriteFile(&cFile, iWidth, iHeight, (int)uiBytesPerSample * 8, iColourType, (uint8*)cCompressedMemory.GetBufferPointer(), cCompressedMemory.GetBufferSize());
			cFile.Close();
		}
		cFile.Kill();
	}
	cCompressed.Kill();
	cCompressedMemory.Kill();

	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not write [", szPathName, "].", NULL);
	}
	return bResult;
}
