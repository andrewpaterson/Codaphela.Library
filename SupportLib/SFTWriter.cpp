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
#include "StandardLib/ChannelsAccessorCreator.h"
#include "ImageToR3G3B2A.h"
#include "SFTWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFT(Ptr<CImage> pcImage, char* szFileName)
{
	CImageR3G3B2A	cRGBTo8bit;
	CChannel*		pcAlpha;

	if (szFileName == NULL)
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
		return SaveSFTTransparentCel(pcImage, szFileName, 0, 0, pcImage->GetWidth(), pcImage->GetHeight());
	}
	else
	{
		return SaveSFTOpaque(pcImage, szFileName);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFT_Opaque::Init(uint16 uiImageWidth, uint16 uiImageHeight)
{
	this->uiType = SFT_TYPE_OPAQUE;
	this->uiImageWidth = uiImageWidth;
	this->uiImageHeight = uiImageHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFT_Opaque_Cel::Init(uint16 uiImageWidth, uint16 uiImageHeight, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	this->uiType = SFT_TYPE_OPAQUE_CEL;
	this->uiImageWidth = uiImageWidth;
	this->uiImageHeight = uiImageHeight;
	this->iCelLeft = iCelLeft;
	this->iCelTop = iCelTop;
	this->uiCelWidth = uiCelWidth;
	this->uiCelHeight = uiCelHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFT_Cel_Run::Init(bool bStartOfRun)
{
	if (bStartOfRun)
	{
		uiFlags = 0x8000;
	}
	else
	{
		uiFlags = 0;
	}
	uiRunLength = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFT_Cel_Run::Set(uint16 uiRunLength, uint16 uiSkipLength)
{
	this->uiFlags |= (uiSkipLength & 0x7fff);
	this->uiRunLength = uiRunLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSFT_Transparent_Cel::Init(uint16 uiImageWidth, uint16 uiImageHeight, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	this->uiType = SFT_TYPE_TRANSPARENT_CEL;
	this->uiImageWidth = uiImageWidth;
	this->uiImageHeight = uiImageHeight;
	this->iCelLeft = iCelLeft;
	this->iCelTop = iCelTop;
	this->uiCelWidth = uiCelWidth;
	this->uiCelHeight = uiCelHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTOpaque(Ptr<CImage> pcImage, char* szFileName)
{
	CFileBasic	cFile;
	SSFT_Opaque	sStruct;

	if (StrEmpty(szFileName))
	{
		return false;
	}

	cFile.Init(DiskFile(szFileName));
	if (cFile.Open(EFM_Write_Create))
	{
		sStruct.Init(pcImage->GetWidth(), pcImage->GetHeight());
		cFile.WriteData(&sStruct, sizeof(SSFT_Opaque));
		cFile.WriteData(pcImage->GetData(), pcImage->GetByteSize());
		cFile.Close();
		cFile.Kill();
		return true;
	}

	cFile.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTOpaqueCel(Ptr<CImage> pcImage, char* szFileName, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	CFileBasic		cFile;
	SSFT_Opaque_Cel	sStruct;

	if (StrEmpty(szFileName))
	{
		return false;
	}

	cFile.Init(DiskFile(szFileName));
	if (cFile.Open(EFM_Write_Create))
	{
		sStruct.Init(pcImage->GetWidth(), pcImage->GetHeight(), iCelLeft, iCelTop, uiCelWidth, uiCelHeight);
		cFile.WriteData(&sStruct, sizeof(SSFT_Opaque_Cel));
		cFile.WriteData(pcImage->GetData(), pcImage->GetByteSize());
		cFile.Close();
		cFile.Kill();
		return true;
	}

	cFile.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveSFTTransparentCel(Ptr<CImage> pcImage, char* szFileName, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	CFileBasic				cFile;
	SSFT_Transparent_Cel	sStruct;
	CChannelsAccessor*		pcOppacityAccessor;
	CChannelsAccessor*		pcDiffueseAccessor;
	size					x;
	size					y;
	size					uiWidth;
	size					uiHeight;
	bool					bOpaque;
	uint8					uiColour;
	SSFT_Cel_Run			sCelRun;
	bool					bLastOpaque;
	uint16					uiSkipLength;
	uint16					uiRunLength;
	
	if (StrEmpty(szFileName))
	{
		return false;
	}

	cFile.Init(DiskFile(szFileName));
	if (cFile.Open(EFM_Write_Create))
	{
		uiWidth = pcImage->GetWidth();
		uiHeight = pcImage->GetHeight();
		sStruct.Init((uint16)uiWidth, (uint16)uiHeight, iCelLeft, iCelTop, uiCelWidth, uiCelHeight);
		cFile.WriteData(&sStruct, sizeof(SSFT_Transparent_Cel));

		pcOppacityAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pcImage->GetChannels(), IMAGE_OPACITY, PT_bit);
		CChannelsAccessorCreator cCreator;
		cCreator.Init(pcImage->GetChannels());
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE);
		pcDiffueseAccessor = cCreator.CreateAndKill();

		for (y = 0; y < uiHeight; y++)
		{
			sCelRun.Init(true);
			bLastOpaque = false;
			uiSkipLength = 0;
			uiRunLength = 0;
			for (x = 0; x < uiWidth; x++)
			{
				bOpaque = *((bool*)pcOppacityAccessor->Get(x + y * uiWidth));
				uiColour = *((uint8*)pcDiffueseAccessor->Get(x + y * uiWidth));

				if (!bOpaque && bLastOpaque)
				{
					sCelRun.Set(uiRunLength, uiSkipLength);

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
				}

				bLastOpaque = bOpaque;
			}

			if (uiRunLength != 0)
			{
				sCelRun.Set(uiRunLength, uiSkipLength);
			}
		}

		pcOppacityAccessor->Kill();
		pcDiffueseAccessor->Kill();

		cFile.WriteData(pcImage->GetData(), pcImage->GetByteSize());
		cFile.Close();
		cFile.Kill();
		return true;
	}

	cFile.Kill();
	return false;
}

