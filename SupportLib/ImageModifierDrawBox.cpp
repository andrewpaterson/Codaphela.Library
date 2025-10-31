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
#include "ImageAccessorCreator.h"
#include "ImageDraw.h"
#include "ImageModifierDrawBox.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageModifierDrawBox::Init(CRectangle* pcRectangle, CImageColour* pcColour, bool bFilled)
{
	mpcColour = pcColour;
	mbFilled = bFilled;
	if (pcRectangle)
	{
		mcRectangle = *pcRectangle;
		mbWholeImage = false;
	}
	else
	{
		mbWholeImage = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageModifierDrawBox::Kill(void)
{
	CImageModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageModifierDrawBox::Modify(Ptr<CImage> pImage)
{
	CImageDraw			cDraw;

	cDraw.Init(pImage);
	cDraw.SetColour(mpcColour);
	if (mbWholeImage)
	{
		cDraw.DrawBox(mbFilled);
	}
	else
	{
		cDraw.DrawBox(&mcRectangle, mbFilled);
	}
	cDraw.Kill();

	return pImage;
}

