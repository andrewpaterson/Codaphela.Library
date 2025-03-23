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
#include "ImageRecolour.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRecolour::Init(CImageColour* pcOldColour, CImageColour* pcNewColour, CRectangle* pcRectangle)
{
	macOldColour.Init();
	macNewColour.Init();

	if (pcRectangle)
	{
		mcRectangle = *pcRectangle;
		mbWholeImage = false;
	}
	else
	{
		mbWholeImage = true;
	}


	AddRecolour(pcOldColour, pcNewColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRecolour::Kill(void)
{	
	macOldColour.Kill();
	macNewColour.Kill();

	CImageModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRecolour::AddRecolour(CImageColour* pcOldColour, CImageColour* pcNewColour)
{
	macOldColour.Add(pcOldColour);
	macNewColour.Add(pcNewColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageRecolour::Modify(Ptr<CImage> pcImage)
{
	CImageAccessor*		pcAccessorOld;
	CImageAccessor*		pcAccessorNew;
	int					x;
	int					y;
	SImageColour		sColourOld[4];
	SImageColour		sColourNew[4];
	size				i;
	CImageColour*		pcColourOld;
	CImageColour*		pcColourNew;
	SImageColour*		psColourCurrent;

	if (mbWholeImage)
	{
		mcRectangle.Init(0, 0, pcImage->GetWidth(), pcImage->GetHeight());
	}

	for (i = 0; i < macOldColour.NumElements(); i++)
	{
		pcColourOld = (CImageColour*)macOldColour.Get(i);
		pcColourNew = (CImageColour*)macNewColour.Get(i);

		pcAccessorOld = CImageAccessorCreator::Create(pcImage, pcColourOld);
		sColourOld[0].Zero();
		pcAccessorOld->MakeColour(sColourOld, pcColourOld);

		pcAccessorNew = CImageAccessorCreator::Create(pcImage, pcColourNew);
		sColourNew[0].Zero();
		pcAccessorNew->MakeColour(sColourNew, pcColourNew);

		for (y = mcRectangle.miTop; y < mcRectangle.miBottom; y++)
		{
			for (x = mcRectangle.miLeft; x < mcRectangle.miRight; x++)
			{
				psColourCurrent = (SImageColour *)pcAccessorOld->Get(x, y);
				if (psColourCurrent->Equals(&sColourOld, pcAccessorOld->GetBufferSize()))
				{
					pcAccessorNew->Set(x, y, &sColourNew);
				}
			}
		}
		pcAccessorNew->Kill();
		pcAccessorOld->Kill();
	}

	return pcImage;
}

