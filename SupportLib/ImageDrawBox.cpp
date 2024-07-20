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
#include "ImageAccessorCreator.h"
#include "ImageDrawBox.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDrawBox::Init(CRectangle* pcRectangle, CImageColour* pcColour, bool bFilled)
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
void CImageDrawBox::Kill(void)
{
	CImageModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageDrawBox::Modify(Ptr<CImage> pcImage)
{
	CImageAccessor*		pcAccessor;
	int					x;
	int					y;
	SImageColour		sColour[4];

	pcAccessor = CImageAccessorCreator::Create(pcImage, mpcColour);
	pcAccessor->MakeColour(sColour, mpcColour);

	if (mbWholeImage)
	{
		mcRectangle.Init(0, 0, pcImage->GetWidth(), pcImage->GetHeight());
	}

	if (mbFilled)
	{
		for (y = mcRectangle.miTop; y < mcRectangle.miBottom; y++)
		{
			for (x = mcRectangle.miLeft; x < mcRectangle.miRight; x++)
			{
				pcAccessor->Set(x, y, sColour);
			}
		}
	}
	else
	{
		for (x = mcRectangle.miLeft; x < mcRectangle.miRight; x++)
		{
			pcAccessor->Set(x, mcRectangle.miTop, sColour);
			pcAccessor->Set(x, mcRectangle.miBottom-1, sColour);
		}

		for (y = mcRectangle.miTop; y < mcRectangle.miBottom; y++)
		{
			pcAccessor->Set(mcRectangle.miLeft, y, sColour);
			pcAccessor->Set(mcRectangle.miRight-1, y, sColour);
		}
	}

	pcAccessor->Kill();

	return pcImage;
}

