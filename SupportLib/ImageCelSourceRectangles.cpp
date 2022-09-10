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
#include "BaseLib/SystemAllocator.h"
#include "ImageCelSourceRectangles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceRectangles::Init(CArrayRectangle* pcRectangles, SImageColour* psTransparentColour, bool bIgnoreEmpty, bool bCropTransparentBorders)
{
	CImageCelSource::Init(psTransparentColour, bIgnoreEmpty, bCropTransparentBorders);
	if (pcRectangles)
	{
		mcRectangles.Init(&gcSystemAllocator);
		mcRectangles.Copy(pcRectangles);
	}
	else
	{
		mcRectangles.Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceRectangles::AddRectangle(CRectangle* pcRect)
{
	mcRectangles.Add(pcRect);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceRectangles::Divide(CImage* pcImage, CArrayUnknown* pcDestImageCels, CImage* pcMask)
{
	CImageDivider	cDivider;

	cDivider.Init(pcImage, mpsTransparentColour, mbIgnoreEmpty, mbCropTransparentBorders);
	cDivider.GenerateFromRectangles(&mcRectangles);
	cDivider.CopyCellsTo(pcDestImageCels);
	cDivider.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceRectangles::Kill(void)
{
	mcRectangles.Kill();
	CImageCelSource::Kill();
}

