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
		return SaveSFTTransparentCel(pcImage, szFilename, 0, 0, pcImage->GetWidth(), pcImage->GetHeight());
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
		bResult = SaveSFTOpaque(pcImage, &cFile);

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
bool SaveSFTOpaque(Ptr<CImage> pcImage, CFileBasic* pcFile)
{
	SSFTOpaque	sStruct;
	size		iResult;

	if (pcFile->IsOpen())
	{
		sStruct.Init(pcImage->GetWidth(), pcImage->GetHeight());
		iResult = pcFile->Write(&sStruct, sizeof(SSFTOpaque), 1);
		if (iResult != 1)
		{
			return false;
		}

		iResult = pcFile->Write(pcImage->GetData(), 1, pcImage->GetByteSize());

		return iResult == pcImage->GetByteSize();
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
bool SaveSFTOpaqueCel(Ptr<CImage> pcImage, char* szFilename, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	CFileBasic		cFile;
	bool			bResult;

	if (StrEmpty(szFilename))
	{
		return false;
	}

	cFile.Init(DiskFile(szFilename));
	if (cFile.Open(EFM_Write_Create))
	{
		bResult = SaveSFTOpaqueCel(pcImage, &cFile, iCelLeft, iCelTop, uiCelWidth, uiCelHeight);

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
bool SaveSFTOpaqueCel(Ptr<CImage> pcImage, CFileBasic* pcFile, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	SSFTOpaqueCel	sStruct;
	size			iResult;

	if (pcFile->IsOpen())
	{
		sStruct.Init(pcImage->GetWidth(), pcImage->GetHeight(), iCelLeft, iCelTop, uiCelWidth, uiCelHeight);
		iResult = pcFile->Write(&sStruct, sizeof(SSFTOpaqueCel), 1);
		if (iResult != 1)
		{
			return false;
		}

		iResult = pcFile->Write(pcImage->GetData(), 1, pcImage->GetByteSize());
		if (iResult != pcImage->GetByteSize())
		{
			return false;
		}

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
bool SaveSFTTransparentCel(Ptr<CImage> pcImage, char* szFilename, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
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
		bResult = SaveSFTTransparentCel(pcImage, &cFile, iCelLeft, iCelTop, uiCelWidth, uiCelHeight);

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
bool SaveSFTTransparentCel(Ptr<CImage> pcImage, CFileBasic* pcFile, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight)
{
	SSFTTransparentCel	sStruct;
	CChannelsAccessor*	pcOppacityAccessor;
	CChannelsAccessor*	pcDiffueseAccessor;
	size				x;
	size				y;
	size				uiWidth;
	size				uiHeight;
	bool				bOpaque;
	uint8				uiColour;
	SSFTCelRun			sCelRun;
	bool				bLastOpaque;
	uint16				uiSkipLength;
	uint16				uiRunLength;
	CArrayChar			auiRow;
	size				iResult;

	if (pcFile->IsOpen())
	{
		uiWidth = pcImage->GetWidth();
		uiHeight = pcImage->GetHeight();
		sStruct.Init((uint16)uiWidth, (uint16)uiHeight, iCelLeft, iCelTop, uiCelWidth, uiCelHeight);
		iResult = pcFile->Write(&sStruct, sizeof(SSFTTransparentCel), 1);
		if (iResult != 1)
		{
			return false;
		}

		pcOppacityAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pcImage->GetChannels(), IMAGE_OPACITY, PT_bit);
		CChannelsAccessorCreator cCreator;
		cCreator.Init(pcImage->GetChannels());
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE);
		pcDiffueseAccessor = cCreator.CreateAndKill();

		auiRow.Init();
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

					CChars	sz;
					sz.Init();
					sz.AppendData2(auiRow.GetData(), auiRow.NumElements());
					sz.AppendNewLine();
					sz.DumpKill();

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
		}

		pcOppacityAccessor->Kill();
		pcDiffueseAccessor->Kill();
		auiRow.Kill();

		return true;
	}
	else
	{
		return false;
	}
}


