/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFT(Ptr<CImage> pcImage, char* szFilename)
{
	CImageR3G3B2A	cRGBTo8bit;
	CChannel*		pcAlpha;

	if (szFilename == NULL)
	{
		return false;
	}

	cRGBTo8bit.Init();
	pcImage = cRGBTo8bit.Modify(&pcImage);
	cRGBTo8bit.Kill();

	if (pcImage.IsNull())
	{
		return false;
	}


	CFileBasic	cFile;

	pcAlpha = pcImage->GetChannel(IMAGE_OPACITY);
	if (pcAlpha)
	{
		return SaveSFTTransparent(pcImage, szFilename);
	}
	else
	{
		return SaveSFTOpaque(pcImage, szFilename);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTOpaque(Ptr<CImage> pcImage, char* szFilename)
{
	CFileBasic	cFile;
	bool		bResult;

	if (StrEmpty(szFilename))
	{
		return false;
	}

	cFile.Init(DiskFile(szFilename));
	if (cFile.Open(EFM_Write_Create))
	{
		bResult = SaveSFTOpaque(pcImage, &cFile, 0, 0, pcImage->GetWidth(), pcImage->GetHeight());

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
bool SaveSFTTransparent(Ptr<CImage> pcImage, char* szFilename)
{
	CFileBasic			cFile;
	bool				bResult;

	if (StrEmpty(szFilename))
	{
		return false;
	}

	cFile.Init(DiskFile(szFilename));
	if (cFile.Open(EFM_Write_Create))
	{
		bResult = SaveSFTTransparent(pcImage, &cFile, 0, 0, pcImage->GetWidth(), pcImage->GetHeight());

		cFile.Close();
		cFile.Kill();
		return bResult;
	}
	else
	{
		cFile.Kill();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTOpaque(Ptr<CImage> pcImage, CFileBasic* pcFile, size uiImageLeftOffset, size uiImageTopOffset, uint16 uiCelWidth, uint16 uiCelHeight)
{
	SSFTImage	sStruct;
	size		iResult;
	size		uiImageWidth;
	size		uiImageHeight;
	uint8*		puiData;
	size		x;
	size		y;
	size		x2;
	size		y2;
	size		i;
	uint8*		puiImage;

	if (pcFile->IsOpen())
	{
		uiImageWidth = pcImage->GetWidth();
		uiImageHeight = pcImage->GetHeight();

		sStruct.Init(SFT_TYPE_OPAQUE, uiCelWidth, uiCelHeight);
		iResult = pcFile->Write(&sStruct, sizeof(SSFTImage), 1);
		if (iResult != 1)
		{
			return false;
		}

		puiImage = (uint8*)pcImage->GetData();
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
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTTransparent(Ptr<CImage> pcImage, CFileBasic* pcFile, size uiImageLeftOffset, size uiImageTopOffset, uint16 uiCelWidth, uint16 uiCelHeight)
{
	SSFTImage			sStruct;
	CChannelsAccessor*	pcOppacityAccessor;
	CChannelsAccessor*	pcDiffueseAccessor;
	size				x;
	size				y;
	size				uiImageWidth;
	size				uiImageHeight;
	bool				bOpaque;
	uint8				uiColour;
	SSFTCelRun			sCelRun;
	bool				bLastOpaque;
	uint16				uiSkipLength;
	uint16				uiRunLength;
	CArrayChar			auiRow;
	size				iResult;
	size				y2;
	size				x2;

	if (pcFile->IsOpen())
	{
		uiImageWidth = pcImage->GetWidth();
		uiImageHeight = pcImage->GetHeight();
		sStruct.Init(SFT_TYPE_TRANSPARENT, uiCelWidth, uiCelHeight);
		iResult = pcFile->Write(&sStruct, sizeof(SSFTImage), 1);
		if (iResult != 1)
		{
			return false;
		}

		pcOppacityAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pcImage->GetChannels(), IMAGE_OPACITY, PT_bit);
		CChannelsAccessorCreator cCreator;
		cCreator.Init(pcImage->GetChannels());
		cCreator.AddAccess(IMAGE_DIFFUSE_BLUE, PT_crumb);
		cCreator.AddAccess(IMAGE_DIFFUSE_GREEN, PT_tribble);
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, PT_tribble);
		pcDiffueseAccessor = cCreator.CreateAndKill();

		auiRow.Init();
		sCelRun.Init(false);
		y2 = uiImageTopOffset + uiCelHeight;
		x2 = uiImageLeftOffset + uiCelWidth;
		for (y = uiImageTopOffset; y < y2; y++)
		{
			bLastOpaque = false;
			uiSkipLength = 0;
			uiRunLength = 0;
			for (x = uiImageLeftOffset; x < x2; x++)
			{
				bOpaque = *((bool*)pcOppacityAccessor->Get(x + y * uiImageWidth));
				uiColour = *((uint8*)pcDiffueseAccessor->Get(x + y * uiImageWidth));

				if (!bOpaque && bLastOpaque)
				{
					sCelRun.Set(uiRunLength, uiSkipLength);
					iResult = pcFile->Write(&sCelRun, sizeof(SSFTCelRun), 1);
					if (iResult != 1)
					{
						auiRow.Kill();
						return false;
					}

					iResult = pcFile->Write(auiRow.GetData(), 1, auiRow.NumElements());
					if (iResult != auiRow.NumElements())
					{
						auiRow.Kill();
						return false;
					}

					auiRow.ReInit();
					sCelRun.Init(false);
					uiSkipLength = 0;
					uiRunLength = 0;
				}

				if (!bOpaque)
				{
					uiSkipLength++;
				}
				else
				{
					uiRunLength++;
					auiRow.Add(uiColour);
				}

				bLastOpaque = bOpaque;
			}

			if (uiRunLength != 0)
			{
				sCelRun.Set(uiRunLength, uiSkipLength);
				iResult = pcFile->Write(&sCelRun, sizeof(SSFTCelRun), 1);
				if (iResult != 1)
				{
					auiRow.Kill();
					return false;
				}

				iResult = pcFile->Write(auiRow.GetData(), 1, auiRow.NumElements());
				if (iResult != auiRow.NumElements())
				{
					auiRow.Kill();
					return false;
				}

				auiRow.ReInit();
			}

			sCelRun.Init(true);
		}

		pcOppacityAccessor->Kill();
		pcDiffueseAccessor->Kill();
		auiRow.Kill();

		sCelRun.Init(false);
		iResult = pcFile->Write(&sCelRun, sizeof(SSFTCelRun), 1);
		return iResult == 1;
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
bool SaveSFT(CArrayUnknown* pacImageCels, char* szFilename)
{
	CImageR3G3B2A	cRGBTo8bit;
	//CChannel*		pcAlpha;
	size			i;
	size			uiNumCels;
	CMapPtrPtr		cMap;
	Ptr<CImage>		pcImage;
	Ptr<CImage>		pcConverted;
	CImageCel*		pcImageCel;
	Ptr<CImage>		pcExisting;
	CFileBasic		cFile;
	CSubImage*		pcSub;
	uint16			uiCelWidth;
	uint16			uiCelHeight;
	uint16			uiCelLeft;
	uint16			uiCelTop;

	if (szFilename == NULL)
	{
		return false;
	}

	cFile.Init(DiskFile(szFilename));
	if (cFile.Open(EFM_Write_Create))
	{
		cMap.Init();
		uiNumCels = pacImageCels->NumElements();
		for (i = 0; i < uiNumCels; i++)
		{
			pcImageCel = (CImageCel*)pacImageCels->Get(i);
			pcImage = pcImageCel->GetSourceImage();
			pcExisting = (CImage*)cMap.Get(&pcImage);

			if (pcExisting.IsNull())
			{
				cRGBTo8bit.Init();
				pcConverted = cRGBTo8bit.Modify(&pcImage);
				if (pcConverted.IsNull())
				{
					cMap.Kill();
					//You should kill the converted images to.  Or use a map that will do that for you.
					return false;
				}
				cRGBTo8bit.Kill();
			
				cMap.Put(&pcImage, &pcConverted);
			}
		}

		pcConverted = NULL;
		pcExisting = NULL;


		for (i = 0; i < uiNumCels; i++)
		{
			pcImageCel = (CImageCel*)pacImageCels->Get(i);
			pcImage = pcImageCel->GetSourceImage();
			pcExisting = (CImage*)cMap.Get(&pcImage);

			pcSub = pcImageCel->GetSubImage();
			uiCelWidth = pcSub->GetImageWidth();
			uiCelHeight = pcSub->GetImageHeight();
			uiCelLeft = pcSub->mcImageRect.GetLeft();
			uiCelTop = pcSub->mcImageRect.GetTop();

			SaveSFTTransparent(pcExisting, &cFile, uiCelLeft, uiCelTop, uiCelWidth, uiCelHeight);
		}

		//pcAlpha = pcImage->GetChannel(IMAGE_OPACITY);
		//if (pcAlpha)
		//{
		//	return SaveSFTTransparent(pcImage, szFilename);
		//}
		//else
		//{
		//	return SaveSFTOpaque(pcImage, szFilename);
		//}

		cFile.Close();
		cFile.Kill();
		return false;
	}

	cFile.Kill();
	return false;

}

