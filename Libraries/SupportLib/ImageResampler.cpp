/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/IntegerHelper.h"
#include "Image.h"
#include "ImageAccessorCreator.h"
#include "ImageResampleBilinear.h"
#include "ImageResampleBicubic.h"
#include "ImageResampleSmart.h"
#include "ImageResampler.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::Init(EImageResample eResample, EPow2Style eStyle)
{
	PreInit();

	Init(eResample, -1, -1);
	meStyle = eStyle;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::Init(EImageResample eResample, int iWidth, int iHeight)
{
	PreInit();

	meResample = eResample;
	miWidth = iWidth;
	miHeight = iHeight;
	meStyle = RS_Unknown;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::Class(void)
{
	CImageModifier::Class();
	U_Enum(meResample);
	U_Int32(miWidth);
	U_Int32(miHeight);
	U_Enum(meStyle);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageResampler::Modify(Ptr<CImage> pImage)
{
	Ptr<CImage>		pDest;

	if (meStyle != RS_Unknown)
	{
		GetPowerOf2Size(meStyle, pImage->GetWidth(), pImage->GetHeight(), &miWidth, &miHeight);
	}

	if ((pImage->GetWidth() == (uint32)miWidth) && (pImage->GetHeight() == (uint32)miHeight))
	{
		return pImage;
	}

	pDest = ResampleTo(pImage);

	return pDest;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageResampler::ResampleTo(Ptr<CImage> pSource)
{
	float32				fdx;
	float32				fdy;
	float32				fWidth;
	float32				fHeight;
	CImageAccessor*		pcSourceAccessor;
	CImageAccessor*		pcDestAccessor;
	float32				fsx;
	float32				fsy;
	float32				fSourceWidth;
	float32				fSourceHeight;
	float32				fScalex;
	float32				fScaley;
	SImageColour*		psColour;
	Ptr<CImage>			pDest;
	bool				bResampled;

	pDest = OMalloc<CImage>();
	pDest->BeginChange();
	pDest->AddChannels(&pSource);
	pDest->SetSize(miWidth, miHeight);
	pDest->EndChange();

	pcSourceAccessor = CImageAccessorCreator::Create(&pSource, &pSource);
	pcDestAccessor = CImageAccessorCreator::Create(&pDest, &pDest);

	if ((pSource->GetWidth() == (uint32)miWidth) && (pSource->GetHeight() == (uint32)miHeight))
	{
		gcLogger.Error2(__METHOD__, " Implement this.  Copy does not work.");
		//pDest->Copy(pSource);
	}
	else
	{
		//This assumes the channels in dest and source are the same.
		//Which is reasonable as dest has just been recreated.
		fWidth = (float32)pDest->GetWidth();
		fHeight = (float32)pDest->GetHeight();

		fSourceWidth = (float32)pSource->GetWidth();
		fSourceHeight = (float32)pSource->GetHeight();

		fScalex = fSourceWidth/fWidth;
		fScaley = fSourceHeight/fHeight;

		bResampled = false;
		if (meResample == IR_Bilinear)
		{
			bResampled = ResampleBilinear(pSource, pDest);
		}
		else if (meResample == IR_Bicubic)
		{
			bResampled = ResampleBicubic(pSource, pDest);
		}
		else if (meResample == IR_Smart)
		{
			bResampled = ResampleSmart(pSource, pDest);
		}

		//Channels the other methods can't resample (reversed channels) are resampled by nearest neighbour.
		if (!bResampled)
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
	}

	pcSourceAccessor->Kill();
	pcDestAccessor->Kill();

	return pDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageResampler::GetPowerOf2Size(EPow2Style eStyle, int32 iSourceWidth, int32 iSourceHeight, int32* piDestWidth, int32* piDestHeight)
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


