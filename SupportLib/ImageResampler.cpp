/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "ImageResampler.h"
#include "Image.h"
#include "BaseLib/FastFunctions.h"
#include "ImageAccessorCreator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::Init(EImageResample eResample, EPow2Style eStyle)
{
	Init(eResample, -1, -1);
	meStyle = eStyle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::Init(EImageResample eResample, int iWidth, int iHeight)
{
	meResample = eResample;
	miWidth = iWidth;
	miHeight = iHeight;
	meStyle = RS_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::Kill(void)
{
	CImageModifier::Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::Modify(CImage* pcImage)
{
	CImage			cDest;

	if (meStyle != RS_Unknown)
	{
		GetPowerOf2Size(meStyle, pcImage->GetWidth(), pcImage->GetHeight(), &miWidth, &miHeight);
	}

	if ((pcImage->GetWidth() == miWidth) && (pcImage->GetHeight() == miHeight))
	{
		return;
	}

	ResampleTo(&cDest, pcImage);
	pcImage->Copy(&cDest);
	cDest.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::ResampleTo(CImage* pcDest, CImage* pcSource)
{
	float					fdx;
	float					fdy;
	float					fWidth;
	float					fHeight;
	CImageAccessor*			pcSourceAccessor;
	CImageAccessor*			pcDestAccessor;
	float					fsx;
	float					fsy;
	float					fSourceWidth;
	float					fSourceHeight;
	float					fScalex;
	float					fScaley;
	float					fHScalex;
	float					fHScaley;
	float					fTotalArea;
	SImageColour*			psColour;

	pcDest->Init();
	pcDest->BeginChange();
	pcDest->AddChannels(pcSource);
	pcDest->SetSize(miWidth, miHeight);
	pcDest->EndChange();

	if ((pcSource->GetWidth() == miWidth) && (pcSource->GetHeight() == miHeight))
	{
		pcDest->Copy(pcSource);
	}
	else
	{
		//This assumes the channels in dest and source are the same.
		//Which is reasonable as dest has just been recreated.
		pcSourceAccessor = CImageAccessorCreator::Create(pcSource, pcSource);
		pcDestAccessor = CImageAccessorCreator::Create(pcDest, pcDest);

		fWidth = (float)pcDest->GetWidth();
		fHeight = (float)pcDest->GetHeight();

		fSourceWidth = (float)pcSource->GetWidth();
		fSourceHeight = (float)pcSource->GetHeight();

		fScalex = fSourceWidth/fWidth;
		fScaley = fSourceHeight/fHeight;
		fHScalex = fScalex / 2.0f;
		fHScaley = fScaley / 2.0f;
		fTotalArea = fScalex*fScaley;  //In terms of the source.

		if (meResample == IR_NearestNeighbour)
		{
			for (fdy = 0.5f; fdy < fHeight; fdy++)
			{
				fsy = fScaley * fdy;
				for (fdx = 0.5f; fdx < fWidth; fdx++)
				{
					fsx = fScalex * fdx;

					psColour = (SImageColour*)pcSourceAccessor->Get((int)fsx, (int)fsy);
					pcDestAccessor->Set((int)fdx, (int)fdy, psColour);
				}
			}
		}

		pcSourceAccessor->Kill();
		pcDestAccessor->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::GetPowerOf2Size(EPow2Style eStyle, int iSourceWidth, int iSourceHeight, int* piDestWidth, int* piDestHeight)
{
	if (eStyle == RS_Up)
	{
		*piDestWidth = GetPowerOf2SizeUp(iSourceWidth);
		*piDestHeight = GetPowerOf2SizeUp(iSourceHeight);
	}
	else if (eStyle == RS_Down)
	{
		*piDestWidth = GetPowerOf2SizeDown(iSourceWidth);
		*piDestHeight = GetPowerOf2SizeDown(iSourceHeight);
	}
	else if (eStyle == RS_Auto)
	{
		*piDestWidth = GetPowerOf2SizeAuto(iSourceWidth);
		*piDestHeight = GetPowerOf2SizeAuto(iSourceHeight);
	}
}


