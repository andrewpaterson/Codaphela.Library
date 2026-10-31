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

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "CatmullRom.h"
#include "ImageResampleChannel.h"
#include "ImageResampleBicubic.h"


#define BICUBIC_TAPS		4


//The four source pixels and their weights for each dest pixel along one axis.
struct SBicubicAxis
{
	int32*		paiPixels;		//BICUBIC_TAPS per dest pixel.
	float64*	pafWeights;		//BICUBIC_TAPS per dest pixel.

	void	Init(uint32 uiSourceSize, uint32 uiDestSize);
	void	Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
// Dest pixel d samples source position (d + 0.5) * source / dest - 0.5.
// The taps are the pixels before, at, after and two after its floor.
//
//////////////////////////////////////////////////////////////////////////
void SBicubicAxis::Init(uint32 uiSourceSize, uint32 uiDestSize)
{
	uint32		d;
	float64		fPosition;
	float64		fFloor;
	float64		t;
	int32		iFloor;
	int32		iPixel;
	int32		iLast;
	uint32		i;
	int32*		paiTaps;
	float64*	pafTaps;

	paiPixels = (int32*)malloc(sizeof(int32) * BICUBIC_TAPS * (uiDestSize + 1));
	pafWeights = (float64*)malloc(sizeof(float64) * BICUBIC_TAPS * (uiDestSize + 1));

	iLast = (int32)uiSourceSize - 1;
	for (d = 0; d < uiDestSize; d++)
	{
		fPosition = ((float64)d + 0.5) * (float64)uiSourceSize / (float64)uiDestSize - 0.5;
		fFloor = floor(fPosition);
		t = fPosition - fFloor;
		iFloor = (int32)fFloor;

		paiTaps = &paiPixels[d * BICUBIC_TAPS];
		pafTaps = &pafWeights[d * BICUBIC_TAPS];
		for (i = 0; i < BICUBIC_TAPS; i++)
		{
			iPixel = iFloor - 1 + (int32)i;
			if (iPixel < 0)
			{
				iPixel = 0;
			}
			else if (iPixel > iLast)
			{
				iPixel = iLast;
			}
			paiTaps[i] = iPixel;
		}

		GetCatmullRomWeights(t, pafTaps);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SBicubicAxis::Kill(void)
{
	free(paiPixels);
	free(pafWeights);
}


//////////////////////////////////////////////////////////////////////////
//
// Resamples the rows to the dest width, then the columns of that to the
// dest height.
//
//////////////////////////////////////////////////////////////////////////
void ResampleBicubicFloats(float64* pafSource, uint32 uiSourceWidth, uint32 uiSourceHeight, float64* pafDest, uint32 uiDestWidth, uint32 uiDestHeight, SBicubicAxis* psX, SBicubicAxis* psY)
{
	float64*	pafRows;
	float64*	pafSourceRow;
	float64*	pafRow;
	int32*		paiTaps;
	float64*	pafTaps;
	uint32		x;
	uint32		y;

	pafRows = (float64*)malloc(sizeof(float64) * uiDestWidth * uiSourceHeight);
	for (y = 0; y < uiSourceHeight; y++)
	{
		pafSourceRow = &pafSource[(lsize)y * uiSourceWidth];
		pafRow = &pafRows[(lsize)y * uiDestWidth];
		for (x = 0; x < uiDestWidth; x++)
		{
			paiTaps = &psX->paiPixels[x * BICUBIC_TAPS];
			pafTaps = &psX->pafWeights[x * BICUBIC_TAPS];
			pafRow[x] =	pafSourceRow[paiTaps[0]] * pafTaps[0] +
						pafSourceRow[paiTaps[1]] * pafTaps[1] +
						pafSourceRow[paiTaps[2]] * pafTaps[2] +
						pafSourceRow[paiTaps[3]] * pafTaps[3];
		}
	}

	for (y = 0; y < uiDestHeight; y++)
	{
		paiTaps = &psY->paiPixels[y * BICUBIC_TAPS];
		pafTaps = &psY->pafWeights[y * BICUBIC_TAPS];
		pafRow = &pafDest[(lsize)y * uiDestWidth];
		for (x = 0; x < uiDestWidth; x++)
		{
			pafRow[x] =	pafRows[(size)paiTaps[0] * uiDestWidth + x] * pafTaps[0] +
						pafRows[(size)paiTaps[1] * uiDestWidth + x] * pafTaps[1] +
						pafRows[(size)paiTaps[2] * uiDestWidth + x] * pafTaps[2] +
						pafRows[(size)paiTaps[3] * uiDestWidth + x] * pafTaps[3];
		}
	}
	free(pafRows);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ResampleBicubic(Ptr<CImage> pcSource, Ptr<CImage> pcDest)
{
	SBicubicAxis			sX;
	SBicubicAxis			sY;
	CImageResampleChannel	cSourceChannel;
	CImageResampleChannel	cDestChannel;
	size					uiChannel;
	size					uiNumChannels;
	uint32					uiSourceWidth;
	uint32					uiSourceHeight;
	uint32					uiDestWidth;
	uint32					uiDestHeight;
	float64*				pafSource;
	float64*				pafDest;
	bool					bResult;

	if (!CImageResampleChannel::CanResample(pcSource, pcDest))
	{
		return false;
	}

	uiSourceWidth = pcSource->GetWidth();
	uiSourceHeight = pcSource->GetHeight();
	uiDestWidth = pcDest->GetWidth();
	uiDestHeight = pcDest->GetHeight();
	if ((uiSourceWidth == 0) || (uiSourceHeight == 0) || (uiDestWidth == 0) || (uiDestHeight == 0))
	{
		return true;
	}

	sX.Init(uiSourceWidth, uiDestWidth);
	sY.Init(uiSourceHeight, uiDestHeight);

	bResult = true;
	uiNumChannels = pcSource->NumChannels();
	for (uiChannel = 0; bResult && (uiChannel < uiNumChannels); uiChannel++)
	{
		cSourceChannel.Init(pcSource, uiChannel);
		bResult = cDestChannel.Init(pcDest, uiChannel);
		if (bResult)
		{
			pafSource = (float64*)malloc(sizeof(float64) * cSourceChannel.NumPixels());
			pafDest = (float64*)malloc(sizeof(float64) * cDestChannel.NumPixels());
			cSourceChannel.GetFloats(pafSource);
			ResampleBicubicFloats(pafSource, uiSourceWidth, uiSourceHeight, pafDest, uiDestWidth, uiDestHeight, &sX, &sY);
			cDestChannel.SetFloats(pafDest);
			free(pafDest);
			free(pafSource);
		}
		cDestChannel.Kill();
		cSourceChannel.Kill();
	}

	sY.Kill();
	sX.Kill();
	return bResult;
}

