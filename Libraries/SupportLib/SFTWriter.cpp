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
#include "BaseLib/NaiveFile.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/MapPtrPtr.h"
#include "StandardLib/ChannelsAccessorCreator.h"
#include "ImageToR3G3B2A.h"
#include "ImageCel.h"
#include "SFTCommon.h"
#include "SFTWriter.h"
#include "SFTQuantiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool HasTransparency(Ptr<CImage> pImage, lsize uiImageLeftOffset, lsize uiImageTopOffset, uint16 uiCelWidth, uint16 uiCelHeight)
{
	lsize				x;
	lsize				y;
	lsize				y2;
	lsize				x2;
	CChannelsAccessor*	pcOppacityAccessor;
	CChannel*			pcAlpha;
	bool				bOpaque;
	lsize				uiImageWidth;

	uiImageWidth = pImage->GetWidth();
	pcAlpha = pImage->GetChannel(IMAGE_OPACITY);
	if (pcAlpha)
	{
		pcOppacityAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pImage->GetChannels(), IMAGE_OPACITY, PT_bit);
		y2 = uiImageTopOffset + uiCelHeight;
		x2 = uiImageLeftOffset + uiCelWidth;
		for (y = uiImageTopOffset; y < y2; y++)
		{
			for (x = uiImageLeftOffset; x < x2; x++)
			{
				bOpaque = *((bool*)pcOppacityAccessor->Get(x + y * uiImageWidth));
				if (!bOpaque)
				{
					pcOppacityAccessor->Kill();
					return true;
				}
			}
		}
		pcOppacityAccessor->Kill();
		return false;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFT(Ptr<CImage> pImage, char* szFilename, bool bForceTransparent, lsize uiMergeGap)
{
	ValidatePtr(pImage);

	CImageR3G3B2A	cRGBTo8bit;
	CFileBasic		cFile;
	bool			bResult;


	if ((szFilename == NULL) || pImage.IsNull())
	{
		return false;
	}

	cRGBTo8bit.Init();
	pImage = cRGBTo8bit.Modify(&pImage);
	cRGBTo8bit.Kill();

	if (pImage.IsNull())
	{
		return false;
	}

	if (StrEmpty(szFilename))
	{
		return false;
	}

	cFile.Init(DiskFile(szFilename));
	if (cFile.Open(EFM_Write_Replace))
	{
		if (HasTransparency(pImage, 0, 0, pImage->GetWidth(), pImage->GetHeight()) || bForceTransparent)
		{
			bResult = SaveSFTTransparent(pImage, &cFile, 0, 0, pImage->GetWidth(), pImage->GetHeight(), uiMergeGap);
		}
		else
		{
			bResult = SaveSFTOpaque(pImage, &cFile, 0, 0, pImage->GetWidth(), pImage->GetHeight());
		}

		cFile.Close();
		cFile.Kill();
		return bResult;
	}

	cFile.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTOpaque(Ptr<CImage> pImage, CFileBasic* pcFile, lsize uiImageLeftOffset, lsize uiImageTopOffset, uint16 uiCelWidth, uint16 uiCelHeight)
{
	SSFTImage	sStruct;
	lsize		iResult;
	lsize		uiImageWidth;
	uint8*		puiData;
	lsize		x;
	lsize		y;
	lsize		x2;
	lsize		y2;
	lsize		i;
	uint8*		puiImage;

	if (pcFile->IsOpen())
	{
		uiImageWidth = pImage->GetWidth();

		sStruct.Init(SFT_TYPE_OPAQUE, uiCelWidth, uiCelHeight);
		if (!sStruct.Write(pcFile))
		{
			return false;
		}

		puiImage = (uint8*)pImage->GetData();
		puiData = (uint8*)malloc(uiCelWidth);
		y2 = uiImageTopOffset + uiCelHeight;
		x2 = uiImageLeftOffset + uiCelWidth;
		for (y = uiImageTopOffset; y < y2; y++)
		{
			for (i = 0, x = uiImageLeftOffset; x < x2; x++, i++)
			{
				puiData[i] = puiImage[x + y * uiImageWidth];
			}

			iResult = pcFile->Write(puiData, 1, uiCelWidth);
			if (iResult != uiCelWidth)
			{
				free(puiData);
				return false;
			}
		}
		free(puiData);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiCount pixels skipped (SFT_RUN_SKIP, uiMax SFT_RUN_MAX_COUNT) or rows
// moved down (SFT_RUN_ROWS, SFT_RUN_MAX_ROWS), uiMax at a time.
//
//////////////////////////////////////////////////////////////////////////
static void AddSFTCount(CArrayChar* pauiData, uint8 uiCode, lsize uiCount, lsize uiMax)
{
	lsize	uiChunk;

	while (uiCount > 0)
	{
		uiChunk = (uiCount < uiMax) ? uiCount : uiMax;
		pauiData->Add((char)(uiCode | (uiChunk - 1)));
		uiCount -= uiChunk;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The cel's opaque pixels as runs (SaveSFTTransparentPixels), its
// pixels R3G3B2 already.
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTTransparent(Ptr<CImage> pImage, CFileBasic* pcFile, lsize uiImageLeftOffset, lsize uiImageTopOffset, uint16 uiCelWidth, uint16 uiCelHeight, lsize uiMergeGap)
{
	CChannelsAccessor*			pcOppacityAccessor;
	CChannelsAccessor*			pcDiffuseAccessor;
	CChannelsAccessorCreator	cCreator;
	lsize						x;
	lsize						y;
	lsize						i;
	lsize						uiImageWidth;
	uint8*						pauiPixels;
	bool*						pabOpaque;
	bool						bResult;

	if (!pcFile->IsOpen())
	{
		return false;
	}

	uiImageWidth = pImage->GetWidth();
	pcOppacityAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pImage->GetChannels(), IMAGE_OPACITY, PT_bit);
	cCreator.Init(pImage->GetChannels());
	cCreator.AddAccess(IMAGE_DIFFUSE_RED, PT_tribble);
	cCreator.AddAccess(IMAGE_DIFFUSE_GREEN, PT_tribble);
	cCreator.AddAccess(IMAGE_DIFFUSE_BLUE, PT_crumb);
	pcDiffuseAccessor = cCreator.CreateAndKill();

	//The cel: which pixels are drawn, and what they are.
	pauiPixels = (uint8*)malloc((lsize)uiCelWidth * uiCelHeight + 1);
	pabOpaque = (bool*)malloc(sizeof(bool) * ((lsize)uiCelWidth * uiCelHeight + 1));
	for (y = 0; y < uiCelHeight; y++)
	{
		for (x = 0; x < uiCelWidth; x++)
		{
			i = uiImageLeftOffset + x + (uiImageTopOffset + y) * uiImageWidth;
			pabOpaque[x + y * uiCelWidth] = (pcOppacityAccessor == NULL) || (*((bool*)pcOppacityAccessor->Get(i)));
			pauiPixels[x + y * uiCelWidth] = pabOpaque[x + y * uiCelWidth] ? *((uint8*)pcDiffuseAccessor->Get(i)) : (uint8)SFT_PIXEL_TRANSPARENT;
		}
	}
	if (pcOppacityAccessor)
	{
		pcOppacityAccessor->Kill();
	}
	pcDiffuseAccessor->Kill();

	bResult = SaveSFTTransparentPixels(pcFile, uiCelWidth, uiCelHeight, pauiPixels, pabOpaque, uiMergeGap);
	free(pauiPixels);
	free(pabOpaque);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A transparent image of uiWidth by uiHeight pixels (puiPixels, a row
// after another), those that pabOpaque says are drawn as runs
// (SFTCommon.h): each row that has any is reached by moving down from the
// last that had, and each of its runs by skipping from the one before.
// The rows after the last pixel aren't written.
//
// Runs of a row that are no more than uiMergeGap transparent pixels apart
// are written as one mixed run, their gaps as SFT_PIXEL_TRANSPARENT: fewer,
// longer runs for what draws a run as one copy.  A run with a pixel of
// that value (black, or a palette's index 0) is left by itself, as it
// can't be in one.  With 0 no runs are mixed.
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTTransparentPixels(CFileBasic* pcFile, uint16 uiWidth, uint16 uiHeight, const uint8* puiPixels, const bool* pabOpaque, lsize uiMergeGap)
{
	SSFTImage		sStruct;
	lsize			x;
	lsize			y;
	lsize			uiRows;
	lsize			uiSkip;
	lsize			uiStart;
	lsize			uiEnd;
	lsize			uiNext;
	lsize			uiAfter;
	lsize			uiChunk;
	lsize			uiMax;
	lsize			i;
	lsize			j;
	bool			bMixed;
	bool			bBlack;
	bool			bNextBlack;
	bool			bResult;
	const uint8*	pauiPixels;
	const bool*		pabRow;
	CArrayChar		auiData;
	lsize			iResult;

	if (!pcFile->IsOpen())
	{
		return false;
	}
	sStruct.Init(SFT_TYPE_TRANSPARENT, uiWidth, uiHeight);
	if (!sStruct.Write(pcFile))
	{
		return false;
	}

	auiData.Init();
	uiRows = 0;
	for (y = 0; y < uiHeight; y++)
	{
		pauiPixels = &puiPixels[y * uiWidth];
		pabRow = &pabOpaque[y * uiWidth];
		uiSkip = 0;
		x = 0;
		while (x < uiWidth)
		{
			if (!pabRow[x])
			{
				uiSkip++;
				x++;
				continue;
			}

			//A run, and the runs after it that are near enough to be one with it.
			uiStart = x;
			bMixed = false;
			bBlack = false;
			for (uiEnd = uiStart; (uiEnd < uiWidth) && pabRow[uiEnd]; uiEnd++)
			{
				bBlack = bBlack || (pauiPixels[uiEnd] == SFT_PIXEL_TRANSPARENT);
			}
			while (!bBlack && (uiMergeGap > 0))
			{
				for (uiNext = uiEnd; (uiNext < uiWidth) && !pabRow[uiNext]; uiNext++)
				{
				}
				if ((uiNext == uiWidth) || (uiNext - uiEnd > uiMergeGap))
				{
					break;
				}
				bNextBlack = false;
				for (uiAfter = uiNext; (uiAfter < uiWidth) && pabRow[uiAfter]; uiAfter++)
				{
					bNextBlack = bNextBlack || (pauiPixels[uiAfter] == SFT_PIXEL_TRANSPARENT);
				}
				if (bNextBlack)
				{
					break;
				}
				bMixed = true;
				uiEnd = uiAfter;
			}

			//Down to its row, across to it, then its pixels.
			AddSFTCount(&auiData, SFT_RUN_ROWS, uiRows, SFT_RUN_MAX_ROWS);
			AddSFTCount(&auiData, SFT_RUN_SKIP, uiSkip, SFT_RUN_MAX_COUNT);
			uiMax = bMixed ? SFT_RUN_MAX_MIXED : SFT_RUN_MAX_PIXELS;
			for (i = uiStart; i < uiEnd; i += uiChunk)
			{
				uiChunk = ((uiEnd - i) < uiMax) ? (uiEnd - i) : uiMax;
				auiData.Add(bMixed ? (char)(SFT_RUN_MIXED | (uiChunk - 1)) : (char)uiChunk);
				for (j = 0; j < uiChunk; j++)
				{
					auiData.Add((char)(bMixed && !pabRow[i + j] ? SFT_PIXEL_TRANSPARENT : pauiPixels[i + j]));
				}
			}
			uiRows = 0;
			uiSkip = 0;
			x = uiEnd;
		}
		uiRows++;
	}
	auiData.Add((char)SFT_RUN_END);

	iResult = pcFile->Write(auiData.GetData(), 1, auiData.NumElements());
	bResult = (iResult == auiData.NumElements());
	auiData.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A record of a container (uiSize bytes at pvRecord) after the ones
// already written, after its SSFTContained: it starts at the next 64KB of
// the file if it would cross one.  The record before (at *puiPrevious,
// unless bFirst) is given the distance to it, and *puiPrevious is this
// one's.
//
//////////////////////////////////////////////////////////////////////////
bool AppendSFTContained(CFileBasic* pcFile, const void* pvRecord, lsize uiSize, filePos* puiPrevious, bool bFirst)
{
	SSFTContained	sContained;
	filePos			uiPosition;
	bool			bResult;

	uiPosition = pcFile->Tell();
	if ((uiPosition / (64 KB)) != ((uiPosition + uiSize) / (64 KB)))
	{
		pcFile->Seek((uiPosition / (64 KB) + 1) * (64 KB));
		uiPosition = pcFile->Tell();
	}

	sContained.Init();
	bResult = sContained.Write(pcFile);
	bResult &= (pcFile->Write(pvRecord, 1, uiSize) == uiSize);
	if (!bFirst)
	{
		sContained.Init((uint16)(uiPosition - *puiPrevious));
		bResult &= pcFile->Seek(*puiPrevious);
		bResult &= sContained.Write(pcFile);
	}
	bResult &= pcFile->Seek(0, EFSO_END);
	*puiPrevious = uiPosition;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The images' palette, then the images, as a container.  An image with a
// transparent pixel is a transparent image (its runs merged as
// SaveSFTTransparentPixels' uiMergeGap says), any other an opaque one.
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTQuantised(CSFTQuantised* pcQuantised, char* szFilename, lsize uiMergeGap)
{
	CFileBasic			cFile;
	CFileBasic			cMemoryBasic;
	CMemoryFile			cMemoryFile;
	SSFTContainer		sContainer;
	SSFTImage			sImage;
	SSFTIndexedImage*	psImage;
	filePos				uiPrevious;
	bool*				pabOpaque;
	bool				bResult;
	size				i;
	lsize				j;
	lsize				uiPixels;

	if ((szFilename == NULL) || StrEmpty(szFilename))
	{
		return false;
	}
	cFile.Init(DiskFile(szFilename));
	if (!cFile.Open(EFM_Write_Replace))
	{
		cFile.Kill();
		return false;
	}

	sContainer.Init((uint16)(pcQuantised->muiNumImages + 1));
	bResult = sContainer.Write(&cFile);
	uiPrevious = 0;
	for (i = 0; (i <= pcQuantised->muiNumImages) && bResult; i++)
	{
		cMemoryFile.Init();
		cMemoryBasic.Init(&cMemoryFile);
		cMemoryBasic.Open(EFM_ReadWrite_Create);
		if (i == 0)
		{
			bResult = SaveSFTPalette(&cMemoryBasic, pcQuantised->msColours.uiFirst, pcQuantised->msColours.uiCount,
									 &pcQuantised->msColours.auiRed[pcQuantised->msColours.uiFirst], &pcQuantised->msColours.auiGreen[pcQuantised->msColours.uiFirst],
									 &pcQuantised->msColours.auiBlue[pcQuantised->msColours.uiFirst]);
		}
		else
		{
			psImage = &pcQuantised->masImages[i - 1];
			uiPixels = (lsize)psImage->uiWidth * psImage->uiHeight;
			if (psImage->bTransparent)
			{
				pabOpaque = (bool*)malloc(sizeof(bool) * (uiPixels + 1));
				for (j = 0; j < uiPixels; j++)
				{
					pabOpaque[j] = (psImage->puiPixels[j] != SFT_PIXEL_TRANSPARENT);
				}
				bResult = SaveSFTTransparentPixels(&cMemoryBasic, psImage->uiWidth, psImage->uiHeight, psImage->puiPixels, pabOpaque, uiMergeGap);
				free(pabOpaque);
			}
			else
			{
				sImage.Init(SFT_TYPE_OPAQUE, psImage->uiWidth, psImage->uiHeight);
				bResult = sImage.Write(&cMemoryBasic) && (cMemoryBasic.Write(psImage->puiPixels, 1, uiPixels) == uiPixels);
			}
		}
		if (bResult)
		{
			bResult = AppendSFTContained(&cFile, cMemoryFile.GetBufferPointer(), cMemoryFile.GetBufferSize(), &uiPrevious, i == 0);
		}
		cMemoryBasic.Close();
		cMemoryBasic.Kill();
		cMemoryFile.Kill();
	}
	cFile.Close();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTCelArray(Ptr<CArrayImageCel> pImageCels, char* szFilename)
{
	ValidatePtr(pImageCels);

	CFileBasic			cDiskBasic;
	bool				bResult;

	if ((szFilename == NULL) || pImageCels.IsNull())
	{
		return false;
	}

	cDiskBasic.Init(DiskFile(szFilename));
	if (cDiskBasic.Open(EFM_Write_Replace))
	{
		bResult = SaveSFTContainer(pImageCels, &cDiskBasic);

		cDiskBasic.Close();
		cDiskBasic.Kill();
		return bResult;
	}

	cDiskBasic.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTContainer(Ptr<CArrayImageCel> pImageCels, CFileBasic* pcDiskBasic)
{
	CImageR3G3B2A	cRGBTo8bit;
	size			i;
	size			uiNumCels;
	CMapPtrPtr		cMap;
	Ptr<CImage>		pImage;
	Ptr<CImage>		pConverted;
	Ptr<CImageCel>	pImageCel;
	Ptr<CImage>		pExisting;
	CSubImage*		pcSub;
	uint16			uiCelWidth;
	uint16			uiCelHeight;
	uint16			uiCelLeft;
	uint16			uiCelTop;
	CFileBasic		cMemoryBasic;
	CMemoryFile		cMemoryFile;
	filePos			uiPrevContainedPos;
	SSFTContainer	sContainer;
	CArrayImage		aConveted;

	if (!pcDiskBasic->IsOpen())
	{
		return false;
	}

	aConveted.Init();  // Necessary to stop the images being destroyed on pConverted reassignment.
	cMap.Init();
	uiNumCels = pImageCels->NumElements();
	for (i = 0; i < uiNumCels; i++)
	{
		pImageCel = pImageCels->Get(i);
		pImage = pImageCel->GetSourceImage();
		pExisting = (CImage*)cMap.Get(&pImage);

		if (pExisting.IsNull())
		{
			cRGBTo8bit.Init();
			pConverted = cRGBTo8bit.Modify(&pImage);
			if (pConverted.IsNull())
			{
				cMap.Kill();
				//You should kill the converted images to.  Or use a map that will do that for you.
				return false;
			}
			aConveted.Add(pConverted);
			cRGBTo8bit.Kill();

			cMap.Put(&pImage, &pConverted);
		}
		pExisting = NULL;
	}

	sContainer.Init((uint16)uiNumCels);
	if (!sContainer.Write(pcDiskBasic))
	{
		pcDiskBasic->Close();
		pcDiskBasic->Kill();
		return false;
	}

	pConverted = NULL;

	uiPrevContainedPos = 0;
	for (i = 0; i < uiNumCels; i++)
	{
		cMemoryFile.Init();
		cMemoryBasic.Init(&cMemoryFile);
		cMemoryBasic.Open(EFM_ReadWrite_Create);

		pImageCel = pImageCels->Get(i);
		pImage = pImageCel->GetSourceImage();
		pExisting = (CImage*)cMap.Get(&pImage);

		pcSub = pImageCel->GetSubImage();
		uiCelWidth = pcSub->GetImageWidth();
		uiCelHeight = pcSub->GetImageHeight();
		uiCelLeft = pcSub->mcImageRect.GetLeft();
		uiCelTop = pcSub->mcImageRect.GetTop();

		if (HasTransparency(pExisting, uiCelLeft, uiCelTop, uiCelWidth, uiCelHeight))
		{
			SaveSFTTransparent(pExisting, &cMemoryBasic, uiCelLeft, uiCelTop, uiCelWidth, uiCelHeight);
		}
		else
		{
			SaveSFTOpaque(pExisting, &cMemoryBasic, uiCelLeft, uiCelTop, uiCelWidth, uiCelHeight);
		}

		AppendSFTContained(pcDiskBasic, cMemoryFile.GetBufferPointer(), cMemoryFile.GetBufferSize(), &uiPrevContainedPos, i == 0);

		cMemoryFile.Close();
		cMemoryFile.Kill();
		cMemoryBasic.Kill();
	}

	cMap.Kill();
	aConveted.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTPalette(CFileBasic* pcFile, uint16 uiFirst, uint16 uiCount, const uint8* puiRed, const uint8* puiGreen, const uint8* puiBlue)
{
	SSFTPalette	sPalette;

	sPalette.Init(uiFirst, uiCount, SFTPaletteHash(uiFirst, uiCount, puiRed, puiGreen, puiBlue));
	if (!pcFile->IsOpen() || !sPalette.IsValid())
	{
		return false;
	}
	return	sPalette.Write(pcFile) &&
			pcFile->WriteData(puiRed, uiCount) &&
			pcFile->WriteData(puiGreen, uiCount) &&
			pcFile->WriteData(puiBlue, uiCount);
}
