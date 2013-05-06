/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include <olectl.h>
#include <math.h>
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "ColourARGB.h"
#include "PNGReader.h"
#include "TGAReader.h"
#include "RADReader.h"
#include "ImageCopier.h"
#include "ImageReader.h"


//====================================================================================
//=
//= Function Name: LoadPictureIPicture
//= 
//= Parameters   : pcImage   - the descriptor to load the image into
//=                szFilename      - the path of the image file to load from
//= 
//= Returns      : TRUE if successful, FALSE otherwise
//=
//= Description  : Loads BMP, EMF, JPG, ICO, WMF, GIF files into a texture descriptor
//=                by leveraging the power of Windows IPicture routines and OLE
//=
//====================================================================================
BOOL LoadPictureIPicture(CImage *pcImage, char *szFilename)	
{
	HDC			hdcTemp;												// The DC To Hold Our Bitmap
	HBITMAP		hbmpTemp;												// Holds The Bitmap Temporarily
	IPicture	*pPicture;												// IPicture Interface
	OLECHAR		wszPath[MAX_PATH + 1];									// Full Path To Picture (WCHAR)
	long		lWidth;													// Width In Logical Units
	long		lHeight;												// Height In Logical Units
	long		lWidthPixels;											// Width In Pixels
	long		lHeightPixels;											// Height In Pixels
	HRESULT     hr;
	short		sType;
	DWORD		dwAttributes;
	
	//----------------------------------------------------------------------------------------
	// Convert From ASCII To Unicode and load the picture path
	//----------------------------------------------------------------------------------------
	MultiByteToWideChar(CP_ACP, 0, szFilename, -1, wszPath, MAX_PATH);	
	hr = OleLoadPicturePath(wszPath, 0, 0, 0, IID_IPicture, (void**)&pPicture);
	
	if(FAILED(hr))	
	{
		return FALSE;													
	}
	hdcTemp = CreateCompatibleDC(GetDC(0));								
	if(!hdcTemp)														
	{
		pPicture->Release();											
		return FALSE;													
	}
	
	//----------------------------------------------------------------------------------------
	// get the pixel dimensions of the image
	//----------------------------------------------------------------------------------------
	pPicture->get_Attributes(&dwAttributes);
	pPicture->get_Type(&sType);
	pPicture->get_Width(&lWidth);									
	lWidthPixels	= MulDiv(lWidth, GetDeviceCaps(hdcTemp, LOGPIXELSX), 2540);
	pPicture->get_Height(&lHeight);									
	lHeightPixels	= MulDiv(lHeight, GetDeviceCaps(hdcTemp, LOGPIXELSY), 2540);

	//----------------------------------------------------------------------------------------
	//	Create A Temporary Bitmap
	//----------------------------------------------------------------------------------------
	BITMAPINFO		bi = {0};												// The Type Of Bitmap We Request
	unsigned int*	pBits = NULL;												// Pointer To The Bitmap Bits
	
	bi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);				// Set Structure Size
	bi.bmiHeader.biBitCount		= 32;									// 32 Bit
	bi.bmiHeader.biWidth		= lWidthPixels;							// Power Of Two Width
	bi.bmiHeader.biHeight		= lHeightPixels;						// Make Image Top Up (Positive Y-Axis)
	bi.bmiHeader.biCompression	= BI_RGB;								// RGB Encoding
	bi.bmiHeader.biPlanes		= 1;									// 1 Bitplane
	hbmpTemp = CreateDIBSection(hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits, 0, 0);
	
	if(!hbmpTemp)														// Did Creation Fail?
	{
		DeleteDC(hdcTemp);												// Delete The Device Context
		pPicture->Release();											// Decrements IPicture Reference Count
		return FALSE;													// Return False (Failure)
	}

	SelectObject(hdcTemp, hbmpTemp);									// Select Handle To Our Temp DC And Our Temp Bitmap Object

	//----------------------------------------------------------------------------------------
	// Render The IPicture On To The Bitmap
	//----------------------------------------------------------------------------------------
	pPicture->Render(hdcTemp, 0, 0, lWidthPixels, lHeightPixels, 0, lHeight, lWidth, -lHeight, 0);

	//----------------------------------------------------------------------------------------
	// Copy the pixels to our texture descriptor whilst
	// Converting from BGR To RGB format and adding alpha of 255
	//----------------------------------------------------------------------------------------
	int							x, y;
	unsigned int				iAlpha;
	BOOL						bNonZeroAlpha;
	CImage						cImageImport;
	int							i;
	CImageCopier				cCopier;

	bNonZeroAlpha = FALSE;
	for (y = 0; y < lHeightPixels; y++)
	{
		for (x = 0; x < lWidthPixels; x++)
		{
			unsigned int iPixel = pBits[(x + (lHeightPixels - y - 1) * lWidthPixels)];
			iAlpha = iPixel & 0xff000000;
			if (iAlpha != 0)
			{
				bNonZeroAlpha = TRUE;
				break;
			}
		}
	}

	//----------------------------------------------------------------------------------------
	// setup the texture descriptor
	//----------------------------------------------------------------------------------------
	// NOTE: does not set the unique ID correctly
	if (bNonZeroAlpha)
	{
		pcImage->Init(lWidthPixels, lHeightPixels, PT_uchar, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_ZERO);
		cImageImport.Init(lWidthPixels, lHeightPixels, pBits, PT_uchar, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_ZERO);
		cCopier.Init(&cImageImport, pcImage);
		for (i = 0; i < lHeightPixels; i++)
		{
			cImageImport.SetData(&pBits[(lHeightPixels - 1 - i) * lWidthPixels]);
			cCopier.Copy(0, i, 0, 0, lWidthPixels, 1);
		}
		//cCopier.Copy(0, 0, NULL);

		cCopier.Kill();
		cImageImport.Kill();
	}
	else
	{
		pcImage->Init(lWidthPixels, lHeightPixels, PT_uchar, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO);
		cImageImport.Init(lWidthPixels, lHeightPixels, pBits, PT_uchar, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_IGNORED, CHANNEL_ZERO);
		cCopier.Init(&cImageImport, pcImage);
		for (i = 0; i < lHeightPixels; i++)
		{
			cImageImport.SetData(&pBits[(lHeightPixels - 1 - i) * lWidthPixels]);
			cCopier.Copy(0, i, 0, 0, lWidthPixels, 1);
		}
		cCopier.Kill();
		cImageImport.Kill();
	}

	DeleteObject(hbmpTemp);												// Delete The Object
	DeleteDC(hdcTemp);													// Delete The Device Context
	
	pPicture->Release();												// Decrements IPicture Reference Count
	
	return TRUE;														// Return True (All Good)
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL ReadImage(CImage* pcImage, char* szFilename, EImageType eType)
{
	if (eType == IT_Unknown)
	{
		eType = GuessImageType(szFilename);
	}

	//Oi! Check to make sure none of the below expect an initialised image.
	switch (eType)
	{
		case IT_BMP:
		case IT_EMF:
		case IT_GIF:
		case IT_ICO:
		case IT_JPG:
		case IT_WMF:
			// uses OLE and IPicture to load these image formats.
			return (LoadPictureIPicture(pcImage, szFilename));
		case IT_TGA:
			return (LoadTGA(pcImage, szFilename)); // uses NeHe TGA routines.
		case IT_PNG:
			return (LoadPNG(pcImage, szFilename));
		case IT_RAD:
			return (LoadRAD(pcImage, szFilename)); // uses a text-format RAW descriptor (RAD) file to load a raw file.
		case IT_RAW:
			return (LoadRAW(pcImage, szFilename)); // special case assuming an initialised image.
	}
	return FALSE;
}

