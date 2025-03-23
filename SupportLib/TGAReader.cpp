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
#include "ImageCopier.h"
#include "ColourARGB.h"
#include "TGAReader.h"


struct STGAFileHeader
{
	uint8  iIDLength;
	uint8  iColourMapType;
	uint8  iImageType;
	unsigned short iFirstEntryIndex;
	unsigned short iColourMapLength;
	uint8  iColourMapEntrySize;
	unsigned short iOriginX;
	unsigned short iOriginY;
};


struct STGAImageHeader
{
	unsigned short iImageWidth;
	unsigned short iImageHeight;
	uint8  iPixelDepth;
	uint8  iPixelDescriptor;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadUncompressedTrueColourTGA(char *szFilename, bool bAddDebug)
{
	STGAImageHeader 	sTGAImageHeader;
	STGAFileHeader  	sTGAFileHeader;				// Used To Store Our File Header
	CFileBasic			sFile;
	int   				iWidth;
	int   				iHeight;
	int   				iBitsPerPixel;
	int   				iBytesPerPixel;
	int   				iImageSize;
	uint8*				pvMem;
	CImage				cImageImport;
	CImageCopier		cCopier;
	filePos				iRead;
	int					i;
	int					iStride;

	sFile.Init(DiskFile(szFilename));
	if (!sFile.Open(EFM_Read))
	{
		sFile.Kill();
		return false;
	}

	//Error check please.
	memset(&sTGAFileHeader, 0, sizeof(STGAFileHeader));
	sFile.Read(&sTGAFileHeader.iIDLength, 1, 1);
	sFile.Read(&sTGAFileHeader.iColourMapType, 1, 1);
	sFile.Read(&sTGAFileHeader.iImageType, 1, 1);
	sFile.Read(&sTGAFileHeader.iFirstEntryIndex, 2, 1);
	sFile.Read(&sTGAFileHeader.iColourMapLength, 2, 1);
	sFile.Read(&sTGAFileHeader.iColourMapEntrySize, 1, 1);
	sFile.Read(&sTGAFileHeader.iOriginX, 2, 1);
	sFile.Read(&sTGAFileHeader.iOriginY, 2, 1);

	//Error check please.
	memset(&sTGAImageHeader, 0, sizeof(STGAImageHeader));
	sFile.Read(&sTGAImageHeader.iImageWidth, 2, 1);
	sFile.Read(&sTGAImageHeader.iImageHeight, 2, 1);
	sFile.Read(&sTGAImageHeader.iPixelDepth, 1, 1);
	sFile.Read(&sTGAImageHeader.iPixelDescriptor, 1, 1);


	//----------------------------------------------------------------------
	// get the TGA dimensions
	//----------------------------------------------------------------------
	iWidth        = (sTGAImageHeader.iImageWidth);
	iHeight       = (sTGAImageHeader.iImageHeight);
	iBitsPerPixel = sTGAImageHeader.iPixelDepth;

	//----------------------------------------------------------------------
	// Make Sure All Information Is Valid
	//----------------------------------------------------------------------
	if ((iWidth <= 0) || (iHeight <= 0))
	{
		sFile.Close();
		sFile.Kill();
		return false;
	}

	switch (iBitsPerPixel)
	{
	case 16:
		iBytesPerPixel = 2;
		break;
	case 24:
		iBytesPerPixel = 3;
		break;
	case 32:
		iBytesPerPixel = 4;
		break;
	default:
		sFile.Close();
		sFile.Kill();
		return false;
	}

	iBytesPerPixel = (iBitsPerPixel / 8);				  // Calculate The BYTES Per Pixel
	iImageSize     = (iBytesPerPixel * iWidth * iHeight); // Calculate Memory Needed To Store Image
	iStride = iWidth * iBytesPerPixel;

	pvMem = (uint8*)malloc(iImageSize);
	iRead = sFile.Read(pvMem, iImageSize, 1);
	if (iRead != 1)
	{
		free(pvMem);
		sFile.Close();
		sFile.Kill();
		return false;	
	}
	sFile.Close();
	sFile.Kill();


	cImageImport.Init();
	cImageImport.BeginChange();
	Ptr<CImage> pcImage = OMalloc<CImage>();
	pcImage->BeginChange();
	if (iBytesPerPixel == 3)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, PT_uint8);
		pcImage->AddChannel(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	}
	else if (iBytesPerPixel == 4)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, PT_uint8);
		pcImage->AddChannel(IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	}
	else if (iBytesPerPixel == 2)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, PT_nickle);
		cImageImport.AddChannel(IMAGE_IGNORED, PT_bit);
		pcImage->AddChannel(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	}
	cImageImport.SetSize(iWidth, iHeight);
	pcImage->SetSize(iWidth, iHeight);
	pcImage->EndChange();
	cImageImport.SetData(pvMem);
	cImageImport.EndChange();

	cCopier.Init(&cImageImport, pcImage);
	for (i = 0; i < iHeight; i++)
	{
		cImageImport.SetData(&pvMem[iStride* ((iHeight-1) - i)]);
		cCopier.Copy(0, i, 0, 0, iWidth, 1);
	}
	cCopier.Kill();
	cImageImport.Kill();

	free(pvMem);
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadTGA(char* szFilename, bool bAddDebug)
{
	static char uTGAcompare[12] = {0,0, 2,0,0,0,0,0,0,0,0,0}; // Uncompressed True Colour TGA Header
	//static char cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0}; // Compressed True Colour TGA Header
	STGAFileHeader tgaheader;				

	CFileBasic sFile;


	//----------------------------------------------------------------------
	// open the tga file.
	//----------------------------------------------------------------------
	sFile.Init(DiskFile(szFilename));
	if (!sFile.Open(EFM_Read))
	{
		sFile.Kill();
		return false;
	}

	//----------------------------------------------------------------------
	// read the header
	//----------------------------------------------------------------------
	if (sFile.Read(&tgaheader, 1, 12) != 12)
	{
		sFile.Close();
		sFile.Kill();
		return false;
	}

	sFile.Close();
	sFile.Kill();

	//----------------------------------------------------------------------
	// check whether the TGA Header matches a compressed one or not
	//----------------------------------------------------------------------
	if (memcmp(uTGAcompare, &tgaheader, 12) == 0)
	{
		return LoadUncompressedTrueColourTGA(szFilename, bAddDebug);
	}
	/*
	else 
	if (memcmp(cTGAcompare, &tgaheader, 10) == 0)
	{											
	return LoadCompressedTrueColourTGA(pcImage, filename);
	}
	*/

	//----------------------------------------------------------------------
	// if no header is matched, this is not a supported TGA format.
	//----------------------------------------------------------------------
	return NULL;
}

