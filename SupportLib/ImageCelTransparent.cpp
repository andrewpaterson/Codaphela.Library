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
#include "ImageCelTransparent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelTransparent::Init(CImage* pcSourceImage, SImageColour* psTransparent)
{
	Init(pcSourceImage, psTransparent, 0, 0, pcSourceImage->miWidth, pcSourceImage->miHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelTransparent::Init(CImage* pcSourceImage, SImageColour* psTransparent, CRectangle* pcRect)
{
	Init(pcSourceImage, psTransparent, pcRect->miLeft, pcRect->miTop, pcRect->miRight, pcRect->miBottom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelTransparent::Init(CImage* pcSourceImage, SImageColour* psTransparent, int iLeft, int iTop, int iRight, int iBottom)
{
	CImageCel::Init(pcSourceImage, iLeft, iTop, iRight, iBottom);
	memcpy(&msTransparentColour, psTransparent, sizeof(SImageColour));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelTransparent::CropTransparentBorders(void)
{
	CPixelOpacityTransparentColour cOpacity;

	cOpacity.Init(&mpcSourceImage, &msTransparentColour);
	CImageCel::CropTransparentBorders(&cOpacity);
	cOpacity.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelTransparent::GetAllChannels(CArrayChannel* pasChannels)
{
	int			iOpacity;
	SChannel	sTransparent;

	iOpacity = IMAGE_OPACITY;
	mpcSourceImage->GetAllChannels(pasChannels);
	if (!mpcSourceImage->HasChannel(iOpacity))
	{
		//Just use the first type.
		sTransparent.eType = pasChannels->Get(0)->eType;
		sTransparent.iChannel = iOpacity;
		pasChannels->InsertAt(&sTransparent, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelTransparent::MustFixDrawOpacity(void)
{
	//If we have a transparent colour then always fix the opacity.
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelTransparent::FixDrawOpacity(CImageAccessor* pcDestOpacity, int iDestTop, int iDestLeft)
{
	CPixelOpacityTransparentColour cOpacity;

	cOpacity.Init(&mpcSourceImage, &msTransparentColour);
	CImageCel::FixDrawOpacity(&cOpacity, pcDestOpacity, iDestLeft, iDestTop);
	cOpacity.Kill();
}
