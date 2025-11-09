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
#include "ImageCopier.h"
#include "ImageReaderHelper.h"
#include "PNGReader.h"




/* Read a PNG file.  You may want to return an error code if the read
* fails (depending upon the failure).  There are two "prototypes" given
* here - one where we are given the filename, and we need to open the
* file, and the other where we are given an open file (possibly with
* some or all of the magic bytes read - see comments above).
*/
Ptr<CImage> LoadPNG(char* szFilename, bool bAddDebug)
{
	png_structp			png_ptr;
	png_infop			info_ptr;
	uint32				sig_read = 0;
	FILE*				fp;
	png_bytep*			row_pointers;
	int					iWidth;
	int					iHeight;
	int					i;
	EPrimitiveType		eSourceType;
	bool				bReverse;

	if ((fp = fopen(szFilename, "rb")) == NULL)
	{
		return false;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fclose(fp);
		return false;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return false;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	* the normal method of doing things with libpng).  REQUIRED unless you
	* set up your own error handlers in the png_create_read_struct() earlier.
	*/

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		fclose(fp);
		/* If we get here, we had a problem reading the file */
		return false;
	}

	/* Set up the input control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* If we have already read some of the signature */
	png_set_sig_bytes(png_ptr, sig_read);

	/*
	* If you have enough memory to read in the entire image at once,
	* and you need to specify only transforms that can be controlled
	* with one of the PNG_TRANSFORM_* bits (this presently excludes
	* dithering, filling, setting background, and doing gamma
	* adjustment), then you can read the entire image (including
	* pixels) into the info structure with this call:
	*/


	png_read_png(png_ptr, info_ptr, 0, png_voidp_NULL);

	/* close the file */
	fclose(fp);

	row_pointers = png_get_rows(png_ptr, info_ptr);
	iWidth = info_ptr->width;
	iHeight = info_ptr->height;

	if ((iWidth == 0) || (iHeight == 0))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		return false;
	}

	bReverse = false;
	if (info_ptr->bit_depth == 1)
	{
		eSourceType = PT_bit;
		bReverse = true;  //Eh.  This seems to be the standard though.
	}
	else if (info_ptr->bit_depth == 2)
	{
		eSourceType = PT_crumb;
		//bReverse = true;
	}
	else if (info_ptr->bit_depth == 4)
	{
		eSourceType = PT_nybble;
		//bReverse = true;
	}
	else if (info_ptr->bit_depth == 8)
	{
		eSourceType = PT_uint8;
	}
	else if (info_ptr->bit_depth == 16)
	{
		eSourceType = PT_uint16;
		bReverse = true;
	}

	Ptr<CImage> pcImage = OMalloc<CImage>();

	CImage cImageImport;

	pcImage->BeginChange();
	cImageImport.Init();
	cImageImport.BeginChange();

	if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_GREY, eSourceType, bReverse);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_GREY, bAddDebug);
	}
	else 	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
	{
		CChars::Dump("PNG_COLOR_TYPE_PALETTE\n");
		//pcImage->AddChannel(IC_Index, PT_uint8, 0);
		//cSourceRow.Add(eSourceType, IC_Index);
		//pcImage->AddPalette(256, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO);
	}
	else if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, eSourceType, bReverse);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_RED, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_GREEN, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_BLUE, bAddDebug);
	}
	else if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, eSourceType, bReverse);
		AddDebugChannel(pcImage, eSourceType, IMAGE_OPACITY, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_RED, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_GREEN, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_BLUE, bAddDebug);
	}
	else if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		cImageImport.AddChannel(IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, eSourceType, bReverse);
		AddDebugChannel(pcImage, eSourceType, IMAGE_OPACITY, bAddDebug);
		AddDebugChannel(pcImage, eSourceType, IMAGE_DIFFUSE_GREY, bAddDebug);
	}

	pcImage->ByteAlignChannels();
	pcImage->SetSize(iWidth, iHeight);
	pcImage->EndChange();

	cImageImport.SetData(row_pointers[0]);
	cImageImport.SetSize(iWidth, iHeight);
	cImageImport.EndChange();

	CImageCopier		cCopier;

	cCopier.Init(&cImageImport, pcImage);
	for (i = 0; i < iHeight; i++)
	{
		cImageImport.SetData(row_pointers[i]);
		cCopier.Copy(0, i, 0, 0, iWidth, 1);
	}
	cCopier.Kill();

	cImageImport.Kill();
	/* At this point you have read the entire image */

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	return pcImage;
}

