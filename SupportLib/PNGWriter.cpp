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
#include <stdio.h>
#include "BaseLib/Logger.h"
#include "ImageLib/png.h"
#include "ImageCopier.h"
#include "PNGWriter.h"


//////////////////////////////////////////////////////////////////////////
//
// PNGs are written with libpng on all platforms so the files are identical.
// The image is copied into 8 bit RGBA, RGB or grey.
//
//////////////////////////////////////////////////////////////////////////
bool SavePNG(Ptr<CImage> pcImage, char* szPathName)
{
	png_structp			png_ptr;
	png_infop			info_ptr;
	FILE*				fp;
	int					iWidth;
	int					iHeight;
	int					iColourType;
	size				uiBytesPerPixel;
	uint8*				pvPixels;
	png_bytep*			apvRows;
	CImage				cImageExport;
	CImageCopier		cCopier;
	int					y;

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
		iColourType = PNG_COLOR_TYPE_RGB_ALPHA;
		uiBytesPerPixel = 4;
	}
	else if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP))
	{
		iColourType = PNG_COLOR_TYPE_RGB;
		uiBytesPerPixel = 3;
	}
	else if (pcImage->HasChannels(IMAGE_DIFFUSE_GREY, CHANNEL_STOP))
	{
		iColourType = PNG_COLOR_TYPE_GRAY;
		uiBytesPerPixel = 1;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Image has no RGB or grey channels.  Cannot write [", szPathName, "].", NULL);
		return false;
	}

	pvPixels = (uint8*)malloc(iWidth * iHeight * uiBytesPerPixel);
	apvRows = (png_bytep*)malloc(iHeight * sizeof(png_bytep));
	for (y = 0; y < iHeight; y++)
	{
		apvRows[y] = &pvPixels[y * iWidth * uiBytesPerPixel];
	}

	if (iColourType == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	}
	else if (iColourType == PNG_COLOR_TYPE_RGB)
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	}
	else
	{
		cImageExport.Init(iWidth, iHeight, pvPixels, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	}
	cCopier.Init(pcImage, &cImageExport);
	cCopier.Copy(0, 0);
	cCopier.Kill();
	cImageExport.Kill();

	fp = fopen(szPathName, "wb");
	if (!fp)
	{
		free(apvRows);
		free(pvPixels);
		gcLogger.Error2(__METHOD__, " Could not open [", szPathName, "] for writing.", NULL);
		return false;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_ptr ? png_create_info_struct(png_ptr) : NULL;
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, png_infopp_NULL);
		fclose(fp);
		free(apvRows);
		free(pvPixels);
		gcLogger.Error2(__METHOD__, " Could not create libpng structures for [", szPathName, "].", NULL);
		return false;
	}

	//libpng reports errors by longjmp-ing back here.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		free(apvRows);
		free(pvPixels);
		gcLogger.Error2(__METHOD__, " libpng failed writing [", szPathName, "].", NULL);
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, iWidth, iHeight, 8, iColourType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, apvRows);
	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	free(apvRows);
	free(pvPixels);
	return true;
}

