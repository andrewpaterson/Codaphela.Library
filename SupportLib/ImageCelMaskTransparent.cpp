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
#include "StandardLib/ClassDefines.h"
#include "ImageCelMaskTransparent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMaskTransparent::Init(Ptr<CImage> pcSourceImage, SImageColour* psTransparent, Ptr<CImage> pcMaskImage, CFillRectangle* pcRect)
{
	CImageCelMask::Init(pcSourceImage, pcMaskImage, pcRect);
	memcpy(&msTransparentColour, psTransparent, sizeof(SImageColour));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMaskTransparent::Init(Ptr<CImage> pcSourceImage, SImageColour* psTransparent, Ptr<CImage> pcMaskImage, short iMask, bool bFilled, int iLeft, int iTop, int iRight, int iBottom)
{
	CImageCelMask::Init(pcSourceImage, pcMaskImage, iMask, bFilled, iLeft, iTop, iRight, iBottom);
	memcpy(&msTransparentColour, psTransparent, sizeof(SImageColour));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMaskTransparent::Class(void)
{
	CImageCelMask::Class();

	U_Data(SImageColour, msTransparentColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMaskTransparent::Free(void)
{
	CImageCelMask::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelMaskTransparent::Save(CObjectWriter* pcFile)
{
	return CImageCelMask::Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelMaskTransparent::Load(CObjectReader* pcFile)
{
	return CImageCelMask::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//

//////////////////////////////////////////////////////////////////////////
void CImageCelMaskTransparent::CropTransparentBorders(void)
{
	CPixelOpacityMaskTransparentColour cOpacity;

	cOpacity.Init(&mpcSourceImage, &msTransparentColour, &mpcMaskImage, miMask);
	CImageCel::CropTransparentBorders(&cOpacity);
	cOpacity.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMaskTransparent::GetAllChannels(CArrayChannel* pasChannels)
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
bool CImageCelMaskTransparent::MustFixDrawOpacity(void)
{
	//If we have a transparent colour then always fix the opacity.
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMaskTransparent::FixDrawOpacity(CImageAccessor* pcDestOpacity, int iDestTop, int iDestLeft)
{
	CPixelOpacityMaskTransparentColour	cOpacity;

	cOpacity.Init(&mpcSourceImage, &msTransparentColour, &mpcMaskImage, miMask);
	CImageCel::FixDrawOpacity(&cOpacity, pcDestOpacity, iDestLeft, iDestTop);
	cOpacity.Kill();
}

