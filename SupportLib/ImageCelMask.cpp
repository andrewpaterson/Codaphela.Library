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
#include "FillRectangle.h"
#include "ImageCelMask.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMask::Init(CImage* pcSourceImage, CImage* pcMaskImage, CFillRectangle* pcRect)
{
	Init(pcSourceImage, pcMaskImage, pcRect->miMask, pcRect->IsFillRectangular(), pcRect->miLeft, pcRect->miTop, pcRect->miRight, pcRect->miBottom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMask::Init(CImage* pcSourceImage, CImage* pcMaskImage, short iMask, bool bFilled, int iLeft, int iTop, int iRight, int iBottom)
{
	CImageCel::Init(pcSourceImage, iLeft, iTop, iRight, iBottom);
	mpcMaskImage = pcMaskImage;
	miMask = iMask;
	mbFilled = bFilled;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMask::Kill(void)
{
	mpcMaskImage = NULL;
	CImageCel::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMask::CropTransparentBorders(void)
{
	CPixelOpacityMask cOpacity;

	cOpacity.Init(mpcSourceImage, mpcMaskImage, miMask);
	CImageCel::CropTransparentBorders(&cOpacity);
	cOpacity.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMask::GetAllChannels(CArrayChannel* pasChannels)
{
	int			iOpacity;
	SChannel	sTransparent;

	if (mbFilled)
	{
		//If the borders defined a rectangle without bits cut out then we don't need to worry about wether
		//the image has opacity or not.
		mpcSourceImage->GetAllChannels(pasChannels);
	}
	else
	{
		//If the borders are irregular then opacity is implied.
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelMask::MustFixDrawOpacity(void)
{
	if (mbFilled && mpcSourceImage->HasChannel(IMAGE_OPACITY))
	{
		//If we already have an opacity channel and then border was full rectangular then it has been used correctly
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelMask::FixDrawOpacity(CImageAccessor* pcDestOpacity, int iDestTop, int iDestLeft)
{
	CPixelOpacityMask	cOpacity;

	cOpacity.Init(mpcSourceImage, mpcMaskImage, miMask);
	CImageCel::FixDrawOpacity(&cOpacity, pcDestOpacity, iDestLeft, iDestTop);
	cOpacity.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
short CImageCelMask::GetMask(void) {return miMask;}
