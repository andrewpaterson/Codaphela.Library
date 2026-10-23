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
#include "ImageResampleChannel.h"
#include "ImageResampleBilinear.h"


//////////////////////////////////////////////////////////////////////////
//
// Dest pixel d samples source position (2d + 1) * source / (2 * dest) - 0.5,
// which is ((2d + 1) * source - dest) / (2 * dest).
//
//////////////////////////////////////////////////////////////////////////
void SBilinearAxis::Init(uint32 uiSourceSize, uint32 uiDestSize)
{
	uint32	d;
	int64	lliNumerator;
	int64	lliPosition;
	int32	iFirst;
	int32	iFraction;
	int32	iLast;

	paiFirst = (int32*)malloc(sizeof(int32) * (uiDestSize + 1));
	paiSecond = (int32*)malloc(sizeof(int32) * (uiDestSize + 1));
	paiFraction = (int32*)malloc(sizeof(int32) * (uiDestSize + 1));

	iLast = (int32)uiSourceSize - 1;
	for (d = 0; d < uiDestSize; d++)
	{
		lliNumerator = (int64)(2 * (int64)d + 1) * (int64)uiSourceSize - (int64)uiDestSize;
		if (lliNumerator <= 0)
		{
			iFirst = 0;
			iFraction = 0;
		}
		else
		{
			lliPosition = (lliNumerator * BILINEAR_ONE) / (2 * (int64)uiDestSize);
			iFirst = (int32)(lliPosition / BILINEAR_ONE);
			iFraction = (int32)(lliPosition % BILINEAR_ONE);
		}

		if (iFirst >= iLast)
		{
			iFirst = iLast;
			iFraction = 0;
		}
		paiFirst[d] = iFirst;
		paiSecond[d] = iFirst < iLast ? iFirst + 1 : iLast;
		paiFraction[d] = iFraction;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SBilinearAxis::Kill(void)
{
	free(paiFirst);
	free(paiSecond);
	free(paiFraction);
}


//////////////////////////////////////////////////////////////////////////
//
// Values are at most 16 bits so a row blend is at most 32 bits and the
// column blend of two rows is at most 48 bits.
//
//////////////////////////////////////////////////////////////////////////
void ResampleBilinearIntegers(int32* paiSource, uint32 uiSourceWidth, int32* paiDest, uint32 uiDestWidth, uint32 uiDestHeight, SBilinearAxis* psX, SBilinearAxis* psY)
{
	uint32	x;
	uint32	y;
	int32*	paiTop;
	int32*	paiBottom;
	int64	lliFractionY;
	int64	lliFractionX;
	int64	lliTop;
	int64	lliBottom;
	int64	lliValue;
	int32	iFirst;
	int32	iSecond;
	int32*	paiRow;

	for (y = 0; y < uiDestHeight; y++)
	{
		paiTop = &paiSource[(size)psY->paiFirst[y] * uiSourceWidth];
		paiBottom = &paiSource[(size)psY->paiSecond[y] * uiSourceWidth];
		lliFractionY = psY->paiFraction[y];
		paiRow = &paiDest[(size)y * uiDestWidth];

		for (x = 0; x < uiDestWidth; x++)
		{
			iFirst = psX->paiFirst[x];
			iSecond = psX->paiSecond[x];
			lliFractionX = psX->paiFraction[x];

			lliTop = (int64)paiTop[iFirst] * (BILINEAR_ONE - lliFractionX) + (int64)paiTop[iSecond] * lliFractionX;
			lliBottom = (int64)paiBottom[iFirst] * (BILINEAR_ONE - lliFractionX) + (int64)paiBottom[iSecond] * lliFractionX;
			lliValue = lliTop * (BILINEAR_ONE - lliFractionY) + lliBottom * lliFractionY;

			//Divide by BILINEAR_ONE squared, rounding to the nearest value.
			paiRow[x] = (int32)((lliValue + ((int64)1 << 31)) >> 32);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ResampleBilinearFloats(float64* pafSource, uint32 uiSourceWidth, float64* pafDest, uint32 uiDestWidth, uint32 uiDestHeight, SBilinearAxis* psX, SBilinearAxis* psY)
{
	uint32		x;
	uint32		y;
	float64*	pafTop;
	float64*	pafBottom;
	float64		fFractionY;
	float64		fFractionX;
	float64		fTop;
	float64		fBottom;
	int32		iFirst;
	int32		iSecond;
	float64*	pafRow;

	for (y = 0; y < uiDestHeight; y++)
	{
		pafTop = &pafSource[(size)psY->paiFirst[y] * uiSourceWidth];
		pafBottom = &pafSource[(size)psY->paiSecond[y] * uiSourceWidth];
		fFractionY = (float64)psY->paiFraction[y] / BILINEAR_ONE;
		pafRow = &pafDest[(size)y * uiDestWidth];

		for (x = 0; x < uiDestWidth; x++)
		{
			iFirst = psX->paiFirst[x];
			iSecond = psX->paiSecond[x];
			fFractionX = (float64)psX->paiFraction[x] / BILINEAR_ONE;

			fTop = pafTop[iFirst] + (pafTop[iSecond] - pafTop[iFirst]) * fFractionX;
			fBottom = pafBottom[iFirst] + (pafBottom[iSecond] - pafBottom[iFirst]) * fFractionX;
			pafRow[x] = fTop + (fBottom - fTop) * fFractionY;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ResampleBilinear(Ptr<CImage> pcSource, Ptr<CImage> pcDest)
{
	SBilinearAxis			sX;
	SBilinearAxis			sY;
	CImageResampleChannel	cSourceChannel;
	CImageResampleChannel	cDestChannel;
	size					uiChannel;
	size					uiNumChannels;
	uint32					uiSourceWidth;
	uint32					uiDestWidth;
	uint32					uiDestHeight;
	int32*					paiSource;
	int32*					paiDest;
	float64*				pafSource;
	float64*				pafDest;
	bool					bResult;

	if (!CImageResampleChannel::IsSupported(pcSource) || (pcSource->NumChannels() != pcDest->NumChannels()))
	{
		return false;
	}

	uiSourceWidth = pcSource->GetWidth();
	uiDestWidth = pcDest->GetWidth();
	uiDestHeight = pcDest->GetHeight();
	if ((uiSourceWidth == 0) || (pcSource->GetHeight() == 0) || (uiDestWidth == 0) || (uiDestHeight == 0))
	{
		return true;
	}

	sX.Init(uiSourceWidth, uiDestWidth);
	sY.Init(pcSource->GetHeight(), uiDestHeight);

	bResult = true;
	uiNumChannels = pcSource->NumChannels();
	for (uiChannel = 0; bResult && (uiChannel < uiNumChannels); uiChannel++)
	{
		cSourceChannel.Init(pcSource, uiChannel);
		bResult = cDestChannel.Init(pcDest, uiChannel);
		if (bResult)
		{
			if (cSourceChannel.IsSmallInteger())
			{
				paiSource = (int32*)malloc(sizeof(int32) * cSourceChannel.NumPixels());
				paiDest = (int32*)malloc(sizeof(int32) * cDestChannel.NumPixels());
				cSourceChannel.GetIntegers(paiSource);
				ResampleBilinearIntegers(paiSource, uiSourceWidth, paiDest, uiDestWidth, uiDestHeight, &sX, &sY);
				cDestChannel.SetIntegers(paiDest);
				free(paiDest);
				free(paiSource);
			}
			else
			{
				pafSource = (float64*)malloc(sizeof(float64) * cSourceChannel.NumPixels());
				pafDest = (float64*)malloc(sizeof(float64) * cDestChannel.NumPixels());
				cSourceChannel.GetFloats(pafSource);
				ResampleBilinearFloats(pafSource, uiSourceWidth, pafDest, uiDestWidth, uiDestHeight, &sX, &sY);
				cDestChannel.SetFloats(pafDest);
				free(pafDest);
				free(pafSource);
			}
		}
		cDestChannel.Kill();
		cSourceChannel.Kill();
	}

	sY.Kill();
	sX.Kill();
	return bResult;
}

