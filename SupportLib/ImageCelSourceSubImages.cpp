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
#include "ImageCel.h"
#include "ImageCelSourceSubImages.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceSubImages::Init(CArraySubImage* pacSubImages, SImageColour* psTransparentColour, BOOL bIgnoreEmpty, BOOL bCropTransparentBorders)
{
	CImageCelSource::Init(psTransparentColour, bIgnoreEmpty, bCropTransparentBorders);
	if (pacSubImages)
	{
		macSubImages.Init(pacSubImages);
	}
	else
	{
		macSubImages.Init(8);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceSubImages::AddSubImage(CSubImage* pcSubImage)
{
	macSubImages.Add(pcSubImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceSubImages::Divide(CImage* pcImage, CArrayUnknown* pcDestImageCels, CImage* pcMask)
{
	CImageDivider	cDivider;
	CArrayRectangle	acRects;
	int				i;
	CSubImage*		pcSubImage;
	CImageCel*		pcCel;

	acRects.Init(macSubImages.NumElements());
	for (i = 0; i < macSubImages.NumElements(); i++)
	{
		pcSubImage = macSubImages.Get(i);
		acRects.Add(&pcSubImage->mcImageRect);
	}

	cDivider.Init(pcImage, mpsTransparentColour, mbIgnoreEmpty, mbCropTransparentBorders);
	cDivider.GenerateFromRectangles(&acRects);
	cDivider.CopyCellsTo(pcDestImageCels);
	cDivider.Kill();
	acRects.Kill();

	for (i = 0; i < macSubImages.NumElements(); i++)
	{
		pcSubImage = macSubImages.Get(i);
		pcCel = (CImageCel*)pcDestImageCels->UnsafeGet(i);
		pcSubImage->AdjustImageRect(pcCel->GetSubImage());
		pcCel->SetSubImage(pcSubImage);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceSubImages::Kill(void)
{
	macSubImages.Kill();
	CImageCelSource::Kill();
}

