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
#ifndef __SFT_WRITER_H__
#define __SFT_WRITER_H__
#include "Image.h"


#define SFT_TYPE_OPAQUE				(uint16)0xe457
#define SFT_TYPE_OPAQUE_CEL			(uint16)0xe434
#define SFT_TYPE_TRANSPARENT_CEL	(uint16)0xe4f9
#define SFT_TYPE_CONTAINER			(uint16)0xe4ef


struct SSFT_Opaque
{
	uint16	uiType;
	uint16	uiImageWidth;
	uint16	uiImageHeight;

	void	Init(uint16 uiImageWidth, uint16 uiImageHeight);
};


struct SSFT_Opaque_Cel
{
	uint16	uiType;
	uint16	uiImageWidth;
	uint16	uiImageHeight;
	int16	iCelLeft;
	int16	iCelTop;
	uint16	uiCelWidth;
	uint16	uiCelHeight;

	void	Init(uint16 uiImageWidth, uint16 uiImageHeight, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight);
};


struct SSFT_Transparent_Cel
{
	uint16	uiType;
	uint16	uiImageWidth;
	uint16	uiImageHeight;
	int16	iCelLeft;
	int16	iCelTop;
	uint16	uiCelWidth;
	uint16	uiCelHeight;

	void	Init(uint16 uiImageWidth, uint16 uiImageHeight, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight);
};


struct SSFT_Cel_Run
{
	uint16	uiFlags;  //0x8000: Start of run flag.  //0x7fff: run skip mask.  or how many pixels to add before the current run starts drawing
	uint16	uiRunLength;

	void Init(bool bStartOfRun);
	void Set(uint16 uiRunLength, uint16 uiSkipLength);
};


struct SSFT_Container
{
	uint16	uiType;
	uint16	uiCount;
};


bool SaveSFT(Ptr<CImage> pcImage, char* szPathName);
bool SaveSFTOpaque(Ptr<CImage> pcImage, char* szFileName);
bool SaveSFTOpaqueCel(Ptr<CImage> pcImage, char* szFileName, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight);
bool SaveSFTTransparentCel(Ptr<CImage> pcImage, char* szFileName, int16 iCelLeft, int16 iCelTop, uint16 uiCelWidth, uint16 uiCelHeight);


#endif // __SFT_WRITER_H__

