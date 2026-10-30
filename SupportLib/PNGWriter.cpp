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
#ifdef __AVX2__
#include <immintrin.h>
#endif // __AVX2__
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
// uiByte filtered with iFilter, given its neighbours to the left, above
// and above left.
//
//////////////////////////////////////////////////////////////////////////
uint8 PNGFilterByte(int iFilter, uint8 uiByte, uint8 uiLeft, uint8 uiUp, uint8 uiUpLeft)
{
	switch (iFilter)
	{
	case PNG_FILTER_NONE:
		return uiByte;
	case PNG_FILTER_SUB:
		return (uint8)(uiByte - uiLeft);
	case PNG_FILTER_UP:
		return (uint8)(uiByte - uiUp);
	case PNG_FILTER_AVERAGE:
		return (uint8)(uiByte - (((int)uiLeft + (int)uiUp) >> 1));
	default:
		return (uint8)(uiByte - PNGPaethPredictor(uiLeft, uiUp, uiUpLeft));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The absolute value of a filtered byte as a signed byte.
//
//////////////////////////////////////////////////////////////////////////
lsize PNGFilterCost(uint8 uiFiltered)
{
	return (uiFiltered < 128) ? uiFiltered : 256 - uiFiltered;
}


#ifdef __AVX2__
//////////////////////////////////////////////////////////////////////////
//
// The Paeth predictor of sixteen 16 bit values.
//
//////////////////////////////////////////////////////////////////////////
__m256i PNGPaethPredictor16(__m256i sLeft, __m256i sUp, __m256i sUpLeft)
{
	__m256i	sUpChange;
	__m256i	sLeftChange;
	__m256i	sLeftDistance;
	__m256i	sUpDistance;
	__m256i	sUpLeftDistance;
	__m256i	sNotLeft;
	__m256i	sUseUpLeft;
	__m256i	sResult;

	//The estimate is left + up - up left, so its distance from left is |up - up left| and so on.
	sUpChange = _mm256_sub_epi16(sUp, sUpLeft);
	sLeftChange = _mm256_sub_epi16(sLeft, sUpLeft);
	sLeftDistance = _mm256_abs_epi16(sUpChange);
	sUpDistance = _mm256_abs_epi16(sLeftChange);
	sUpLeftDistance = _mm256_abs_epi16(_mm256_add_epi16(sUpChange, sLeftChange));

	sNotLeft = _mm256_or_si256(_mm256_cmpgt_epi16(sLeftDistance, sUpDistance), _mm256_cmpgt_epi16(sLeftDistance, sUpLeftDistance));
	sUseUpLeft = _mm256_and_si256(sNotLeft, _mm256_cmpgt_epi16(sUpDistance, sUpLeftDistance));
	sResult = _mm256_blendv_epi8(sLeft, sUp, sNotLeft);
	return _mm256_blendv_epi8(sResult, sUpLeft, sUseUpLeft);
}


//////////////////////////////////////////////////////////////////////////
//
// The Paeth predictor of 32 bytes.
//
//////////////////////////////////////////////////////////////////////////
__m256i PNGPaethPredictor8(__m256i sLeft, __m256i sUp, __m256i sUpLeft)
{
	__m256i	sZero;
	__m256i	sLow;
	__m256i	sHigh;

	//Unpacking and packing both work within each 128 bit lane so the bytes end up back where they started.
	sZero = _mm256_setzero_si256();
	sLow = PNGPaethPredictor16(_mm256_unpacklo_epi8(sLeft, sZero), _mm256_unpacklo_epi8(sUp, sZero), _mm256_unpacklo_epi8(sUpLeft, sZero));
	sHigh = PNGPaethPredictor16(_mm256_unpackhi_epi8(sLeft, sZero), _mm256_unpackhi_epi8(sUp, sZero), _mm256_unpackhi_epi8(sUpLeft, sZero));
	return _mm256_packus_epi16(sLow, sHigh);
}


//////////////////////////////////////////////////////////////////////////
//
// The rounded down average of 32 pairs of bytes.  _mm256_avg_epu8 rounds
// up.
//
//////////////////////////////////////////////////////////////////////////
__m256i PNGAverage8(__m256i sLeft, __m256i sUp)
{
	return _mm256_sub_epi8(_mm256_avg_epu8(sLeft, sUp), _mm256_and_si256(_mm256_xor_si256(sLeft, sUp), _mm256_set1_epi8(1)));
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the sum of the absolute values (as signed bytes) of 32 filtered
// bytes to four 64 bit sums.
//
//////////////////////////////////////////////////////////////////////////
__m256i PNGAddFilterCost8(__m256i sSums, __m256i sFiltered)
{
	return _mm256_add_epi64(sSums, _mm256_sad_epu8(_mm256_abs_epi8(sFiltered), _mm256_setzero_si256()));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize PNGSumFilterCost8(__m256i sSums)
{
	uint64	auiSums[4];

	_mm256_storeu_si256((__m256i*)auiSums, sSums);
	return (lsize)(auiSums[0] + auiSums[1] + auiSums[2] + auiSums[3]);
}
#endif // __AVX2__


//////////////////////////////////////////////////////////////////////////
//
// Filters puiRow into puiDest with iFilter.  puiPrior is the row above,
// all zeros for the first row.
//
//////////////////////////////////////////////////////////////////////////
void PNGFilterRow(int iFilter, uint8* puiRow, uint8* puiPrior, lsize uiRowBytes, lsize uiPixelBytes, uint8* puiDest)
{
	lsize	i;

	//The first pixel has nothing to its left.
	for (i = 0; (i < uiPixelBytes) && (i < uiRowBytes); i++)
	{
		puiDest[i] = PNGFilterByte(iFilter, puiRow[i], 0, puiPrior[i], 0);
	}

#ifdef __AVX2__
	__m256i	sByte;
	__m256i	sLeft;
	__m256i	sUp;
	__m256i	sUpLeft;
	__m256i	sFiltered;

	for (; i + 32 <= uiRowBytes; i += 32)
	{
		sByte = _mm256_loadu_si256((__m256i*)&puiRow[i]);
		sLeft = _mm256_loadu_si256((__m256i*)&puiRow[i - uiPixelBytes]);
		sUp = _mm256_loadu_si256((__m256i*)&puiPrior[i]);
		sUpLeft = _mm256_loadu_si256((__m256i*)&puiPrior[i - uiPixelBytes]);
		switch (iFilter)
		{
		case PNG_FILTER_NONE:
			sFiltered = sByte;
			break;
		case PNG_FILTER_SUB:
			sFiltered = _mm256_sub_epi8(sByte, sLeft);
			break;
		case PNG_FILTER_UP:
			sFiltered = _mm256_sub_epi8(sByte, sUp);
			break;
		case PNG_FILTER_AVERAGE:
			sFiltered = _mm256_sub_epi8(sByte, PNGAverage8(sLeft, sUp));
			break;
		default:
			sFiltered = _mm256_sub_epi8(sByte, PNGPaethPredictor8(sLeft, sUp, sUpLeft));
			break;
		}
		_mm256_storeu_si256((__m256i*)&puiDest[i], sFiltered);
	}
#endif // __AVX2__

	for (; i < uiRowBytes; i++)
	{
		puiDest[i] = PNGFilterByte(iFilter, puiRow[i], puiRow[i - uiPixelBytes], puiPrior[i], puiPrior[i - uiPixelBytes]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The cost of filtering puiRow with each filter, the sum of absolute
// values of the filtered bytes as signed bytes.  puiPrior is the row
// above, all zeros for the first row.
//
//////////////////////////////////////////////////////////////////////////
void PNGFilterRowCosts(uint8* puiRow, uint8* puiPrior, lsize uiRowBytes, lsize uiPixelBytes, lsize* puiCosts)
{
	lsize	i;
	int		iFilter;

	for (iFilter = 0; iFilter < PNG_NUM_FILTERS; iFilter++)
	{
		puiCosts[iFilter] = 0;
	}

	for (i = 0; (i < uiPixelBytes) && (i < uiRowBytes); i++)
	{
		for (iFilter = 0; iFilter < PNG_NUM_FILTERS; iFilter++)
		{
			puiCosts[iFilter] += PNGFilterCost(PNGFilterByte(iFilter, puiRow[i], 0, puiPrior[i], 0));
		}
	}

#ifdef __AVX2__
	__m256i	sByte;
	__m256i	sLeft;
	__m256i	sUp;
	__m256i	sUpLeft;
	__m256i	asSums[PNG_NUM_FILTERS];

	for (iFilter = 0; iFilter < PNG_NUM_FILTERS; iFilter++)
	{
		asSums[iFilter] = _mm256_setzero_si256();
	}
	for (; i + 32 <= uiRowBytes; i += 32)
	{
		sByte = _mm256_loadu_si256((__m256i*)&puiRow[i]);
		sLeft = _mm256_loadu_si256((__m256i*)&puiRow[i - uiPixelBytes]);
		sUp = _mm256_loadu_si256((__m256i*)&puiPrior[i]);
		sUpLeft = _mm256_loadu_si256((__m256i*)&puiPrior[i - uiPixelBytes]);
		asSums[PNG_FILTER_NONE] = PNGAddFilterCost8(asSums[PNG_FILTER_NONE], sByte);
		asSums[PNG_FILTER_SUB] = PNGAddFilterCost8(asSums[PNG_FILTER_SUB], _mm256_sub_epi8(sByte, sLeft));
		asSums[PNG_FILTER_UP] = PNGAddFilterCost8(asSums[PNG_FILTER_UP], _mm256_sub_epi8(sByte, sUp));
		asSums[PNG_FILTER_AVERAGE] = PNGAddFilterCost8(asSums[PNG_FILTER_AVERAGE], _mm256_sub_epi8(sByte, PNGAverage8(sLeft, sUp)));
		asSums[PNG_FILTER_PAETH] = PNGAddFilterCost8(asSums[PNG_FILTER_PAETH], _mm256_sub_epi8(sByte, PNGPaethPredictor8(sLeft, sUp, sUpLeft)));
	}
	for (iFilter = 0; iFilter < PNG_NUM_FILTERS; iFilter++)
	{
		puiCosts[iFilter] += PNGSumFilterCost8(asSums[iFilter]);
	}
#endif // __AVX2__

	for (; i < uiRowBytes; i++)
	{
		for (iFilter = 0; iFilter < PNG_NUM_FILTERS; iFilter++)
		{
			puiCosts[iFilter] += PNGFilterCost(PNGFilterByte(iFilter, puiRow[i], puiRow[i - uiPixelBytes], puiPrior[i], puiPrior[i - uiPixelBytes]));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Filters every row with whichever filter gives the smallest sum of
// absolute values (as signed bytes), the usual heuristic.  Each filtered
// row starts with its filter type.  puiZeros is a row of zeros, the row
// above the first.
//
//////////////////////////////////////////////////////////////////////////
void PNGFilterRows(uint8* puiPixels, int iHeight, lsize uiRowBytes, lsize uiPixelBytes, uint8* puiFiltered, uint8* puiZeros)
{
	int		y;
	int		iFilter;
	int		iBestFilter;
	lsize	auiCosts[PNG_NUM_FILTERS];
	uint8*	puiRow;
	uint8*	puiPrior;

	puiPrior = puiZeros;
	for (y = 0; y < iHeight; y++)
	{
		puiRow = &puiPixels[y * uiRowBytes];
		PNGFilterRowCosts(puiRow, puiPrior, uiRowBytes, uiPixelBytes, auiCosts);
		iBestFilter = PNG_FILTER_NONE;
		for (iFilter = 1; iFilter < PNG_NUM_FILTERS; iFilter++)
		{
			if (auiCosts[iFilter] < auiCosts[iBestFilter])
			{
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
bool PNGWriteFile(CFileBasic* pcFile, int iWidth, int iHeight, int iBitDepth, int iColourType, uint8* puiCompressed, lsize uiCompressedSize)
{
	CMemoryFile		cHeaderMemory;
	CFileBasic		cHeader;
	bool			bResult;
	lsize			uiPosition;
	lsize			uiChunk;

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
	lsize			uiRowBytes;
	lsize			uiPixelBytes;
	EPrimitiveType	eSampleType;
	uint8*			pvPixels;
	uint8*			pvFiltered;
	uint8*			pvZeros;
	CImage			cImageExport;
	CImageCopier	cCopier;
	CMemoryFile		cCompressedMemory;
	CFileBasic		cCompressed;
	CDeflater		cDeflater;
	CFileBasic		cFile;
	size			i;
	lsize			uiByte;
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

	uiPixelBytes = (lsize)uiNumChannels * uiBytesPerSample;
	uiRowBytes = iWidth * uiPixelBytes;
	pvPixels = (uint8*)malloc(uiRowBytes * iHeight);
	pvFiltered = (uint8*)malloc((uiRowBytes + 1) * iHeight);
	pvZeros = (uint8*)calloc(uiRowBytes, 1);
	if ((pvPixels == NULL) || (pvFiltered == NULL) || (pvZeros == NULL))
	{
		free(pvZeros);
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
		for (uiByte = 0; uiByte < uiRowBytes * iHeight; uiByte += 2)
		{
			uiSwap = pvPixels[uiByte];
			pvPixels[uiByte] = pvPixels[uiByte + 1];
			pvPixels[uiByte + 1] = uiSwap;
		}
	}

	PNGFilterRows(pvPixels, iHeight, uiRowBytes, uiPixelBytes, pvFiltered, pvZeros);
	free(pvZeros);
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
