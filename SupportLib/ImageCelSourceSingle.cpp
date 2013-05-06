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
#include "ImageCelSourceSingle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceSingle::Init(int iLeftOffset, int iTopOffset, int iRightOffset, int iBottomOffset, SImageColour* psTransparentColour, BOOL bIgnoreEmpty, BOOL bCropTransparentBorders)
{
	CImageCelSource::Init(psTransparentColour, bIgnoreEmpty, bCropTransparentBorders);
	miLeftOffset = iLeftOffset;
	miTopOffset = iTopOffset;
	miRightOffset = iRightOffset;
	miBottomOffset = iBottomOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceSingle::Divide(CImage* pcImage, CArrayUnknown* pcDestImageCels, CImage* pcMask)
{
	CImageDividerNumbers	cNumbers;
	CImageDivider			cDivider;

	cDivider.Init(pcImage, mpsTransparentColour, mbIgnoreEmpty, mbCropTransparentBorders);
	cNumbers.InitFromSingle(miLeftOffset, miTopOffset, miRightOffset, miBottomOffset);
	cDivider.GenerateFromNumbers(&cNumbers);
	cDivider.CopyCellsTo(pcDestImageCels);
	cDivider.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelSourceSingle::Kill(void)
{
	CImageCelSource::Kill();
}

