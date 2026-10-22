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
#include "BaseLib/Logger.h"
#include "ImageAccessorCreator.h"
#include "FontColours.h"


//////////////////////////////////////////////////////////////////////////
//
// pWhite's image must be white with the glyphs' coverage in its opacity.
//
//////////////////////////////////////////////////////////////////////////
void CFontColours::Init(Ptr<CFont> pWhite)
{
	PreInit();

	mpWhite = pWhite;
	mapFonts.Init();
	masColours.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontColours::Class(void)
{
	M_Pointer(mpWhite);
	M_Embedded(mapFonts);
	U_Data(CArrayARGB32, masColours);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontColours::Free(void)
{
	masColours.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontColours::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontColours::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Opaque white is the white font itself.  Any other colour is made the
// first time it is asked for and kept.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFontColours::GetFont(ARGB32 sColour)
{
	size		i;
	Ptr<CFont>	pFont;

	if (sColour == FONT_COLOURS_WHITE)
	{
		return mpWhite;
	}

	for (i = 0; i < masColours.NumElements(); i++)
	{
		if (*masColours.Get(i) == sColour)
		{
			return mapFonts.Get(i);
		}
	}

	pFont = Recolour(sColour);
	if (pFont.IsNotNull())
	{
		mapFonts.Add(pFont);
		masColours.Add(&sColour);
	}
	return pFont;
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the white font's image with every pixel's red, green and blue
// set to the colour.  The accessors write whatever channel order and
// types the image has.  An alpha less than opaque scales the opacity.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFontColours::Recolour(ARGB32 sColour)
{
	Ptr<CImage>				pWhiteImage;
	Ptr<CImage>				pImage;
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcColour;
	CImageAccessor*			pcOpacity;
	uint8					auiColour[3];
	uint8					uiAlpha;
	uint8*					puiOpacity;
	uint8					uiOpacity;
	int32					x;
	int32					y;
	int32					iWidth;
	int32					iHeight;

	pWhiteImage = mpWhite->GetImage();
	if (pWhiteImage.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Font [", mpWhite->GetFontName(), "] has no image to recolour.", NULL);
		return NULL;
	}

	pImage = OMalloc<CImage>();
	pImage->CopyIntoInitialised(pWhiteImage);
	iWidth = (int32)pImage->GetWidth();
	iHeight = (int32)pImage->GetHeight();

	auiColour[0] = Get8BitRedColour(sColour);
	auiColour[1] = Get8BitGreenColour(sColour);
	auiColour[2] = Get8BitBlueColour(sColour);
	cCreator.Init(&pImage);
	cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	pcColour = cCreator.CreateAndKill();
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			pcColour->Set(x, y, auiColour);
		}
	}
	pcColour->Kill();

	uiAlpha = Get8BitAlphaColour(sColour);
	if ((uiAlpha != 0xFF) && pImage->HasChannel(IMAGE_OPACITY))
	{
		cCreator.Init(&pImage);
		cCreator.AddAccess(IMAGE_OPACITY, PT_uint8);
		pcOpacity = cCreator.CreateAndKill();
		for (y = 0; y < iHeight; y++)
		{
			for (x = 0; x < iWidth; x++)
			{
				puiOpacity = (uint8*)pcOpacity->Get(x, y);
				uiOpacity = (uint8)(((uint32)*puiOpacity * uiAlpha + 127) / 255);
				pcOpacity->Set(x, y, &uiOpacity);
			}
		}
		pcOpacity->Kill();
	}

	return OMalloc<CFont>(mpWhite->GetDescription(), pImage);
}


//////////////////////////////////////////////////////////////////////////
//
// The number of colours made, not counting white.
//
//////////////////////////////////////////////////////////////////////////
size CFontColours::NumColours(void)
{
	return masColours.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFontColours::GetWhite(void)
{
	return mpWhite;
}

