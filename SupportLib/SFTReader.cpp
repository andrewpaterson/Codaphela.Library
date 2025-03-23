/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
	uint16			uiType;
	bool			bResult;
	Ptr<CImage>		pcImage;

	cFile.Init(DiskFile(szFilename));
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		return NULL;
	}

	bResult = cFile.ReadInt16(&uiType);
	if (!bResult)
	{
		cFile.Close();
		cFile.Kill();
		return NULL;
	}

	if (uiType == SFT_TYPE_OPAQUE)
	{
		cFile.Seek(0);
		pcImage = LoadSFTOpaque(&cFile, bAddDebug);
		cFile.Close();
		cFile.Kill();

		return pcImage;
	}
	else if (uiType == SFT_TYPE_TRANSPARENT)
	{
		cFile.Seek(0);
		pcImage = LoadSFTTransparent(&cFile, bAddDebug);
		cFile.Close();
		cFile.Kill();

		return pcImage;
	}
	else if (uiType == SFT_TYPE_CONTAINER)
	{
		cFile.Seek(0);
		pcImage = LoadSFTContainer(&cFile, bAddDebug);
		cFile.Close();
		cFile.Kill();

		return pcImage;
	}
	else
	{
		cFile.Close();
		cFile.Kill();
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 Convert8BitColourTo32BitColour(uint8 uiColour8)
{
	uint8				r;
	uint8				g;
	uint8				b;
	uint8				a;
	uint32				uiColour32;

	//b = (uiColour8 >> 0) & 0x3;
	//g = (uiColour8 >> 2) & 0x7;
	//r = (uiColour8 >> 5) & 0x7;
	b = (uiColour8 >> 6) & 0x3;
	g = (uiColour8 >> 3) & 0x7;
	r = (uiColour8 >> 0) & 0x7;

	b = (b << 6) + (b << 4) + (b << 2) + b;
	g = (g << 5) + (g << 2) + (g >> 1);
	r = (r << 5) + (r << 2) + (r >> 1);
	a = 0xff;

	uiColour32 = (r << 0) | (g << 8) | (b << 16) | (a << 24);
	return uiColour32;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadSFTOpaque(CFileBasic* pcFile, bool bAddDebug)
{
	SSFTImage	sStruct;
	size		iResult;
	size		i;
	uint8		uiColour8;
	uint32		uiColour32;
	uint8*		puiImage;
	size		uiSize;
	uint8*		puiData;
	size		iPos;

	iResult = pcFile->Read(&sStruct, sizeof(SSFTImage), 1);
	if (iResult != 1)
	{
		return NULL;
	}

	if (sStruct.uiType != SFT_TYPE_OPAQUE)
	{
		return NULL;
	}

	Ptr<CImage> pcImage = OMalloc<CImage>(sStruct.uiImageWidth, sStruct.uiImageHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO);
	if (pcImage.IsNull())
	{
		return false;
	}
	puiImage = (uint8*)pcImage->GetData();

	if (bAddDebug)
	{
		pcImage->SetChannelDebugNames(IMAGE_DIFFUSE_RED);
		pcImage->SetChannelDebugNames(IMAGE_DIFFUSE_GREEN);
		pcImage->SetChannelDebugNames(IMAGE_DIFFUSE_BLUE);
	}

	uiSize = sStruct.uiImageWidth * sStruct.uiImageHeight;
	puiData = (uint8*)malloc(uiSize);
	iResult = pcFile->Read(puiData, 1, uiSize);
	if (iResult != uiSize)
	{
		return false;
	}

	for (i = 0, iPos = 0; i < uiSize; i++)
	{
		uiColour8 = puiData[i];
		uiColour32 = Convert8BitColourTo32BitColour(uiColour8);

		puiImage[iPos] = (uiColour32 >> 0) & 0xff;  iPos++;
		puiImage[iPos] = (uiColour32 >> 8) & 0xff;  iPos++;
		puiImage[iPos] = (uiColour32 >> 16) & 0xff;  iPos++;
	}

	free(puiData);
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadSFTTransparent(CFileBasic* pcFile, bool bAddDebug)
{
	SSFTImage			sStruct;
	size				iResult;
	size				x;
	size				y;
	SSFTCelRun			sCelRun;
	CArrayChar			sMemory;
	size				x2;
	size				i;
	uint8				uiColour8;
	uint32				uiColour32;
	uint32*				puiImage;
	size				uiWidth;
	void*				pvResult;

	iResult = pcFile->Read(&sStruct, sizeof(SSFTImage), 1);
	if (iResult != 1)
	{
		return NULL;
	}

	if (sStruct.uiType != SFT_TYPE_TRANSPARENT)
	{
		return NULL;
	}

	uiWidth = sStruct.uiImageWidth;
	Ptr<CImage> pcImage = OMalloc<CImage>(sStruct.uiImageWidth, sStruct.uiImageHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_ZERO);
	if (pcImage.IsNull())
	{
		return false;
	}

	puiImage = (uint32*)pcImage->GetData();
	memset(puiImage, 0, pcImage->GetByteSize());

	if (bAddDebug)
	{
		pcImage->SetChannelDebugNames(IMAGE_DIFFUSE_RED);
		pcImage->SetChannelDebugNames(IMAGE_DIFFUSE_GREEN);
		pcImage->SetChannelDebugNames(IMAGE_DIFFUSE_BLUE);
		pcImage->SetChannelDebugNames(IMAGE_OPACITY);
	}

	x = 0;
	y = 0;
	sMemory.Init();
	for (;;)
	{
		iResult = pcFile->Read(&sCelRun, sizeof(SSFTCelRun), 1);
		if (iResult != 1)
		{
			return NULL;
		}

		if (sCelRun.IsDone())
		{
			break;
		}

		if (sCelRun.IsStartOfRun())
		{
			y++;
			x = 0;
		}

		x += sCelRun.GetSkipLength();

		pvResult = sMemory.GrowToAtLeastNumElements(sCelRun.GetRunLength());
		if (pvResult == NULL)
		{
			return NULL;
		}

		iResult = pcFile->Read(sMemory.GetData(), 1, sCelRun.GetRunLength());
		if (iResult != sCelRun.GetRunLength())
		{
			return NULL;
		}

		x2 = x + sCelRun.GetRunLength();
		for (i = 0; x < x2; x++, i++)
		{
			uiColour8 = sMemory.GetValue(i);
			uiColour32 = Convert8BitColourTo32BitColour(uiColour8);

			puiImage[x + y * uiWidth] = uiColour32;
		}
	}
	sMemory.Kill();

	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadSFTContainer(CFileBasic* pcFile, bool bAddDebug)
{
	SSFTContainer			sContainer;
	size					iResult;
	SSFTContained			sContained;
	bool					bResult;
	uint16					uiType;
	filePos					uiFilePos;
	Ptr<CImage>				pcImage;
	size					i;
	CImageCelsSource		cSource;
	CImageCombiner			cCombiner;
	CImageCelSourceSingle	cSingle;
	Ptr<CImage>				pcDestImage;

	cSingle.Init(0, 0, 0, 0, NULL, false, false);
	cSource.Init();

	iResult = pcFile->Read(&sContainer, sizeof(SSFTContainer), 1);
	if (iResult != 1)
	{
		return NULL;
	}

	if (sContainer.uiType != SFT_TYPE_CONTAINER)
	{
		return NULL;
	}

	for (i = 0;; i++)
	{
		iResult = pcFile->Read(&sContained, sizeof(SSFTContained), 1);
		if (iResult != 1)
		{
			return NULL;
		}

		uiFilePos = pcFile->Tell();
		bResult = pcFile->ReadInt16(&uiType);
		bResult &= pcFile->Seek(uiFilePos);
		if (!bResult)
		{
			return NULL;
		}

		pcImage = NULL;
		if (uiType == SFT_TYPE_OPAQUE)
		{
			pcImage = LoadSFTOpaque(pcFile, bAddDebug);
			if (pcImage.IsNull())
			{
				return NULL;
			}
		}
		else if (uiType == SFT_TYPE_TRANSPARENT)
		{
			pcImage = LoadSFTTransparent(pcFile, bAddDebug);
			if (pcImage.IsNull())
			{
				return NULL;
			}
		}
		else
		{
			return NULL;
		}

		cSource.AddMemorySource(pcImage, &cSingle);

		if (sContained.uiSkipToNext != 0)
		{
			bResult = pcFile->Seek(uiFilePos - sizeof(SSFTContained));
			bResult &= pcFile->Seek(sContained.uiSkipToNext, EFSO_CURRENT);
			if (!bResult)
			{
				return NULL;
			}
		}
		else
		{
			break;
		}
	}

	cSource.Load();

	cCombiner.Init(ICL_Best, ICS_PowerOf2, ICC_FromCels);
	cCombiner.AddCels(cSource.GetCels());
	pcDestImage = cCombiner.Combine();

	cCombiner.Kill();
	cSingle.Kill();
	cSource.Kill();


	return pcDestImage;
}

