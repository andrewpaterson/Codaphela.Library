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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayChar.h"
#include "ImageCopier.h"
#include "ImageCombiner.h"
#include "ImageCelsSource.h"
#include "ImageCelSourceSingle.h"
#include "SFTCommon.h"
#include "SFTReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadSFT(char* szFilename, bool bAddDebug)
{
	CFileBasic		cFile;
	SSFTHeader		sHeader;
	Ptr<CImage>		pImage;

	cFile.Init(DiskFile(szFilename));
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		return NULL;
	}

	if (PeekSFTHeader(&cFile, &sHeader) && sHeader.IsValid())
	{
		if (sHeader.uiType == SFT_TYPE_OPAQUE)
		{
			pImage = LoadSFTOpaque(&cFile, bAddDebug);
		}
		else if (sHeader.uiType == SFT_TYPE_TRANSPARENT)
		{
			pImage = LoadSFTTransparent(&cFile, bAddDebug);
		}
		else if (sHeader.uiType == SFT_TYPE_CONTAINER)
		{
			pImage = LoadSFTContainer(&cFile, bAddDebug);
		}
	}
	cFile.Close();
	cFile.Kill();
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// A pixel's colour in the palette, or with none its own.  False if the
// palette hasn't it.
//
//////////////////////////////////////////////////////////////////////////
static bool SFTPixelColour(const SSFTColours* psColours, uint8 uiPixel, uint32* puiColour)
{
	if (psColours == NULL)
	{
		*puiColour = SFTColour32(uiPixel);
		return true;
	}
	if (!psColours->Has(uiPixel))
	{
		return false;
	}
	*puiColour = psColours->Colour32(uiPixel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadSFTOpaque(CFileBasic* pcFile, bool bAddDebug, const SSFTColours* psColours)
{
	SSFTImage	sStruct;
	lsize		iResult;
	lsize		i;
	uint32		uiColour32;
	uint8*		puiImage;
	lsize		uiSize;
	uint8*		puiData;
	lsize		iPos;

	if (!sStruct.Read(pcFile) || !sStruct.Is(SFT_TYPE_OPAQUE))
	{
		return NULL;
	}

	Ptr<CImage> pImage = OMalloc<CImage>(sStruct.uiImageWidth, sStruct.uiImageHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	if (pImage.IsNull())
	{
		return NULL;
	}
	puiImage = (uint8*)pImage->GetData();

	if (bAddDebug)
	{
		pImage->SetChannelDebugNames(IMAGE_DIFFUSE_RED);
		pImage->SetChannelDebugNames(IMAGE_DIFFUSE_GREEN);
		pImage->SetChannelDebugNames(IMAGE_DIFFUSE_BLUE);
	}

	uiSize = (lsize)sStruct.uiImageWidth * sStruct.uiImageHeight;
	puiData = (uint8*)malloc(uiSize);
	iResult = pcFile->Read(puiData, 1, uiSize);
	if (iResult != uiSize)
	{
		free(puiData);
		return NULL;
	}

	for (i = 0, iPos = 0; i < uiSize; i++)
	{
		if (!SFTPixelColour(psColours, puiData[i], &uiColour32))
		{
			free(puiData);
			return NULL;
		}
		puiImage[iPos] = (uiColour32 >> 0) & 0xff;  iPos++;
		puiImage[iPos] = (uiColour32 >> 8) & 0xff;  iPos++;
		puiImage[iPos] = (uiColour32 >> 16) & 0xff;  iPos++;
	}

	free(puiData);
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadSFTTransparent(CFileBasic* pcFile, bool bAddDebug, const SSFTColours* psColours)
{
	SSFTImage			sStruct;
	lsize				x;
	lsize				y;
	lsize				i;
	uint8				uiCode;
	uint8				auiRun[SFT_RUN_MAX_PIXELS];
	uint32*				puiImage;
	lsize				uiWidth;
	lsize				uiHeight;

	if (!sStruct.Read(pcFile) || !sStruct.Is(SFT_TYPE_TRANSPARENT))
	{
		return NULL;
	}

	uiWidth = sStruct.uiImageWidth;
	Ptr<CImage> pImage = OMalloc<CImage>(sStruct.uiImageWidth, sStruct.uiImageHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	if (pImage.IsNull())
	{
		return NULL;
	}

	puiImage = (uint32*)pImage->GetData();
	memset(puiImage, 0, pImage->GetByteSize());

	if (bAddDebug)
	{
		pImage->SetChannelDebugNames(IMAGE_DIFFUSE_RED);
		pImage->SetChannelDebugNames(IMAGE_DIFFUSE_GREEN);
		pImage->SetChannelDebugNames(IMAGE_DIFFUSE_BLUE);
		pImage->SetChannelDebugNames(IMAGE_OPACITY);
	}

	uiHeight = sStruct.uiImageHeight;
	x = 0;
	y = 0;
	for (;;)
	{
		if (pcFile->Read(&uiCode, 1, 1) != 1)
		{
			return NULL;
		}
		if (uiCode == SFT_RUN_END)
		{
			break;
		}
		if (uiCode >= SFT_RUN_MIXED)
		{
			//A mixed run's pixels, which must be in the image: those that aren't transparent.
			uiCode = (uint8)((uiCode & SFT_RUN_MIXED_MASK) + 1);
			if ((y >= uiHeight) || (x + uiCode > uiWidth) || (pcFile->Read(auiRun, 1, uiCode) != uiCode))
			{
				return NULL;
			}
			for (i = 0; i < uiCode; i++, x++)
			{
				if ((auiRun[i] != SFT_PIXEL_TRANSPARENT) && !SFTPixelColour(psColours, auiRun[i], &puiImage[x + y * uiWidth]))
				{
					return NULL;
				}
			}
		}
		else if (uiCode >= SFT_RUN_ROWS)
		{
			y += (uiCode & SFT_RUN_ROWS_MASK) + 1;
			x = 0;
		}
		else if (uiCode >= SFT_RUN_SKIP)
		{
			x += (uiCode & SFT_RUN_COUNT_MASK) + 1;
		}
		else
		{
			//A run's pixels, which must be in the image.
			if ((y >= uiHeight) || (x + uiCode > uiWidth) || (pcFile->Read(auiRun, 1, uiCode) != uiCode))
			{
				return NULL;
			}
			for (i = 0; i < uiCode; i++, x++)
			{
				if (!SFTPixelColour(psColours, auiRun[i], &puiImage[x + y * uiWidth]))
				{
					return NULL;
				}
			}
		}
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// The container's images, each in the colours of the palette before it
// (or its own with none), combined into one.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadSFTContainer(CFileBasic* pcFile, bool bAddDebug)
{
	SSFTContainer			sContainer;
	SSFTContained			sContained;
	SSFTHeader				sHeader;
	SSFTColours*			psColours;
	bool					bResult;
	bool					bPalette;
	lsize					uiImages;
	filePos					uiFilePos;
	Ptr<CImage>				pImage;
	lsize					i;
	CImageCelsSource		cSource;
	CImageCombiner			cCombiner;
	CImageCelSourceSingle	cSingle;
	Ptr<CImage>				pDestImage;

	if (!sContainer.Read(pcFile) || !sContainer.IsValid())
	{
		return NULL;
	}

	psColours = (SSFTColours*)malloc(sizeof(SSFTColours));
	psColours->Init();
	bPalette = false;
	uiImages = 0;
	cSingle.Init(0, 0, 0, 0, NULL, false, false);
	cSource.Init();
	bResult = true;
	for (i = 0; i < sContainer.uiCount; i++)
	{
		if (!sContained.Read(pcFile))
		{
			bResult = false;
			break;
		}

		uiFilePos = pcFile->Tell();
		if (!PeekSFTHeader(pcFile, &sHeader) || !sHeader.IsValid())
		{
			bResult = false;
			break;
		}

		pImage = NULL;
		if (sHeader.uiType == SFT_TYPE_PALETTE)
		{
			if (!psColours->Read(pcFile))
			{
				bResult = false;
				break;
			}
			bPalette = true;
		}
		else
		{
			if (sHeader.uiType == SFT_TYPE_OPAQUE)
			{
				pImage = LoadSFTOpaque(pcFile, bAddDebug, bPalette ? psColours : NULL);
			}
			else if (sHeader.uiType == SFT_TYPE_TRANSPARENT)
			{
				pImage = LoadSFTTransparent(pcFile, bAddDebug, bPalette ? psColours : NULL);
			}
			if (pImage.IsNull())
			{
				bResult = false;
				break;
			}
			cSource.AddMemorySource(pImage, &cSingle);
			uiImages++;
		}

		if (sContained.uiSkipToNext == 0)
		{
			break;
		}
		if (!pcFile->Seek(uiFilePos - sizeof(SSFTContained)) || !pcFile->Seek(sContained.uiSkipToNext, EFSO_CURRENT))
		{
			bResult = false;
			break;
		}
	}
	free(psColours);

	if (bResult && (uiImages > 0))
	{
		cSource.Load();
		cCombiner.Init(ICL_Best, ICS_PowerOf2, ICC_FromCels);
		cCombiner.AddCels(cSource.GetImageCels());
		pDestImage = cCombiner.Combine();
		cCombiner.Kill();
	}
	cSingle.Kill();
	cSource.Kill();
	return pDestImage;
}
