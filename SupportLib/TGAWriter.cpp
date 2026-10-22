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
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "ImageCopier.h"
#include "RLEPacketEncoder.h"
#include "TGAWriter.h"


#define TGA_TRUE_COLOUR			2
#define TGA_GREY				3
#define TGA_RLE					8


//////////////////////////////////////////////////////////////////////////
//
// Writes the header with no ID or colour map.  Rows are stored bottom up,
// left to right.
//
//////////////////////////////////////////////////////////////////////////
bool TGAWriteHeader(CFileWriter* pcFile, int iImageType, int iWidth, int iHeight, int iPixelBytes, bool bAlpha)
{
	return	pcFile->WriteInt8((uint8)0) &&						//ID length.
			pcFile->WriteInt8((uint8)0) &&						//No colour map.
			pcFile->WriteInt8((uint8)iImageType) &&
			pcFile->WriteLittleEndianInt16((uint16)0) &&					//Colour map first entry.
			pcFile->WriteLittleEndianInt16((uint16)0) &&					//Colour map length.
			pcFile->WriteInt8((uint8)0) &&						//Colour map entry bits.
			pcFile->WriteLittleEndianInt16((uint16)0) &&					//X origin.
			pcFile->WriteLittleEndianInt16((uint16)0) &&					//Y origin.
			pcFile->WriteLittleEndianInt16((uint16)iWidth) &&
			pcFile->WriteLittleEndianInt16((uint16)iHeight) &&
			pcFile->WriteInt8((uint8)(iPixelBytes * 8)) &&
			pcFile->WriteInt8((uint8)(bAlpha ? 8 : 0));		//Alpha bits.
}


//////////////////////////////////////////////////////////////////////////
//
// TGA 2.0 footer with no extension or developer areas.
//
//////////////////////////////////////////////////////////////////////////
bool TGAWriteFooter(CFileWriter* pcFile)
{
	return	pcFile->WriteLittleEndianInt32((uint32)0) &&					//Extension area offset.
			pcFile->WriteLittleEndianInt32((uint32)0) &&					//Developer directory offset.
			pcFile->WriteData("TRUEVISION-XFILE.", 18);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveTGA(Ptr<CImage> pcImage, char* szPathName, bool bRLE)
{
	int					iWidth;
	int					iHeight;
	bool				bGrey;
	bool				bAlpha;
	int					iPixelBytes;
	size				uiRowBytes;
	size				uiMaxDataSize;
	size				uiDataSize;
	uint8*				pvPixels;
	uint8*				pvData;
	uint8*				puiRow;
	uint8*				puiSource;
	uint8*				puiDest;
	CImage				cImageExport;
	CImageCopier		cCopier;
	CFileBasic			cFile;
	CRLEPacketEncoder	cEncoder;
	int					x;
	int					y;
	bool				bResult;

	if (pcImage.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Cannot write NULL image [", szPathName, "].", NULL);
		return false;
	}

	iWidth = pcImage->GetWidth();
	iHeight = pcImage->GetHeight();
	if ((iWidth <= 0) || (iHeight <= 0) || (iWidth > 0xFFFF) || (iHeight > 0xFFFF))
	{
		gcLogger.Error2(__METHOD__, " Cannot write a [", IntToString(iWidth), " x ", IntToString(iHeight), "] image as a Targa [", szPathName, "].", NULL);
		return false;
	}

	if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP))
	{
		bGrey = false;
	}
	else if (pcImage->HasChannels(IMAGE_DIFFUSE_GREY, CHANNEL_STOP))
	{
		bGrey = true;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Image has no RGB or grey channels.  Cannot write [", szPathName, "].", NULL);
		return false;
	}
	bAlpha = pcImage->HasChannels(IMAGE_OPACITY, CHANNEL_STOP);
	iPixelBytes = (bGrey ? 1 : 3) + (bAlpha ? 1 : 0);

	//Export in Targa's byte order: blue, green, red, alpha or grey, alpha.
	pvPixels = (uint8*)malloc((size)iWidth * iHeight * iPixelBytes);
	if (pvPixels == NULL)
	{
		gcLogger.Error2(__METHOD__, " Could not allocate memory to write [", szPathName, "].", NULL);
		return false;
	}
	if (bGrey && bAlpha)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, CHANNEL_STOP);
	}
	else if (bGrey)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	}
	else if (bAlpha)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_STOP);
	}
	else
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_STOP);
	}
	cCopier.Init(pcImage, &cImageExport);
	cCopier.Copy(0, 0);
	cCopier.Kill();
	cImageExport.Kill();

	cEncoder.Init(iPixelBytes);
	uiRowBytes = (size)iWidth * iPixelBytes;
	uiMaxDataSize = bRLE ? cEncoder.GetMaxEncodedSize(iWidth) * iHeight : uiRowBytes * iHeight;
	pvData = (uint8*)malloc(uiMaxDataSize);
	if (pvData == NULL)
	{
		cEncoder.Kill();
		free(pvPixels);
		gcLogger.Error2(__METHOD__, " Could not allocate memory to write [", szPathName, "].", NULL);
		return false;
	}
	puiDest = pvData;
	for (y = iHeight - 1; y >= 0; y--)
	{
		puiRow = &pvPixels[y * uiRowBytes];
		if (bRLE)
		{
			//Packets never cross rows.
			puiDest += cEncoder.Encode(puiRow, iWidth, puiDest);
		}
		else
		{
			for (x = 0; x < iWidth; x++)
			{
				puiSource = &puiRow[x * iPixelBytes];
				memcpy(puiDest, puiSource, iPixelBytes);
				puiDest += iPixelBytes;
			}
		}
	}
	cEncoder.Kill();
	free(pvPixels);

	uiDataSize = puiDest - pvData;

	cFile.Init(DiskFile(szPathName));
	bResult = cFile.Open(EFM_Write_Create);
	if (bResult)
	{
		bResult =	TGAWriteHeader(&cFile, (bGrey ? TGA_GREY : TGA_TRUE_COLOUR) | (bRLE ? TGA_RLE : 0), iWidth, iHeight, iPixelBytes, bAlpha) &&
					cFile.WriteData(pvData, uiDataSize) &&
					TGAWriteFooter(&cFile);
		cFile.Close();
	}
	cFile.Kill();
	free(pvData);

	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not write [", szPathName, "].", NULL);
	}
	return bResult;
}
