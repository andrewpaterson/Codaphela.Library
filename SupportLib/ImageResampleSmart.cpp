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
#include "ImageResampleChannel.h"
#include "ImageResampleBilinear.h"
#include "ImageResampleSmart.h"


#define SMART_CORNERS		4
#define SMART_TIE			1e-12


//All the channels of the source as floats and what's needed to compare pixels.
struct SSmartSource
{
	size		uiNumChannels;
	float64**	papafValues;	//One array of pixels per channel.
	float64*	pafScales;		//Multiplies a difference to make it a fraction of the channel's range.
	bool*		pabCompared;	//Compared on its own; false for red, green, blue and ignored channels.
	size		uiRed;
	size		uiGreen;
	size		uiBlue;
	bool		bRGB;

	bool		Init(Ptr<CImage> pcSource);
	void		Kill(void);
	bool		IsSimilar(size uiPixel1, size uiPixel2);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SSmartSource::Init(Ptr<CImage> pcSource)
{
	size					uiChannel;
	CImageResampleChannel	cChannel;
	EChannel				eChannel;
	size					uiNumPixels;

	uiNumChannels = pcSource->NumChannels();
	uiNumPixels = (size)pcSource->GetWidth() * (size)pcSource->GetHeight();
	papafValues = (float64**)malloc(sizeof(float64*) * (uiNumChannels + 1));
	pafScales = (float64*)malloc(sizeof(float64) * (uiNumChannels + 1));
	pabCompared = (bool*)malloc(sizeof(bool) * (uiNumChannels + 1));

	uiRed = pcSource->GetChannels()->GetIndexOfChannel(IMAGE_DIFFUSE_RED);
	uiGreen = pcSource->GetChannels()->GetIndexOfChannel(IMAGE_DIFFUSE_GREEN);
	uiBlue = pcSource->GetChannels()->GetIndexOfChannel(IMAGE_DIFFUSE_BLUE);
	bRGB = (uiRed != ARRAY_ELEMENT_NOT_FOUND) && (uiGreen != ARRAY_ELEMENT_NOT_FOUND) && (uiBlue != ARRAY_ELEMENT_NOT_FOUND);

	for (uiChannel = 0; uiChannel < uiNumChannels; uiChannel++)
	{
		papafValues[uiChannel] = (float64*)malloc(sizeof(float64) * (uiNumPixels + 1));
		cChannel.Init(pcSource, uiChannel);
		cChannel.GetFloats(papafValues[uiChannel]);

		//Floats are assumed to run from 0 to 1.
		pafScales[uiChannel] = cChannel.IsFloat() ? 1.0 : 1.0 / (cChannel.GetMaximum() - cChannel.GetMinimum());
		cChannel.Kill();

		eChannel = pcSource->GetChannelAtIndex(uiChannel)->iChannel;
		pabCompared[uiChannel] = (eChannel != IMAGE_IGNORED);
		if (bRGB && ((uiChannel == uiRed) || (uiChannel == uiGreen) || (uiChannel == uiBlue)))
		{
			pabCompared[uiChannel] = false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSmartSource::Kill(void)
{
	size	uiChannel;

	for (uiChannel = 0; uiChannel < uiNumChannels; uiChannel++)
	{
		free(papafValues[uiChannel]);
	}
	free(papafValues);
	free(pafScales);
	free(pabCompared);
}


//////////////////////////////////////////////////////////////////////////
//
// Red, green and blue are compared in YUV, as the hq filters do.
//
//////////////////////////////////////////////////////////////////////////
bool SSmartSource::IsSimilar(size uiPixel1, size uiPixel2)
{
	size	uiChannel;
	float64	fRed;
	float64	fGreen;
	float64	fBlue;
	float64	fDifference;

	if (uiPixel1 == uiPixel2)
	{
		return true;
	}

	if (bRGB)
	{
		fRed = (papafValues[uiRed][uiPixel1] - papafValues[uiRed][uiPixel2]) * pafScales[uiRed];
		fGreen = (papafValues[uiGreen][uiPixel1] - papafValues[uiGreen][uiPixel2]) * pafScales[uiGreen];
		fBlue = (papafValues[uiBlue][uiPixel1] - papafValues[uiBlue][uiPixel2]) * pafScales[uiBlue];

		if ((fabs(0.299 * fRed + 0.587 * fGreen + 0.114 * fBlue) > SMART_THRESHOLD_Y) ||
			(fabs(-0.169 * fRed - 0.331 * fGreen + 0.5 * fBlue) > SMART_THRESHOLD_U) ||
			(fabs(0.5 * fRed - 0.419 * fGreen - 0.081 * fBlue) > SMART_THRESHOLD_V))
		{
			return false;
		}
	}

	for (uiChannel = 0; uiChannel < uiNumChannels; uiChannel++)
	{
		if (pabCompared[uiChannel])
		{
			fDifference = (papafValues[uiChannel][uiPixel1] - papafValues[uiChannel][uiPixel2]) * pafScales[uiChannel];
			if (fabs(fDifference) > SMART_THRESHOLD_CHANNEL)
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Groups the four corners and returns the group (the lowest corner in
// it) with the most weight.  On a tie the group with the heaviest corner
// wins, then the first.
//
//////////////////////////////////////////////////////////////////////////
int SmartChooseGroup(SSmartSource* psSource, size* puiPixels, float64* pafWeights, int* paiGroups)
{
	int		i;
	int		j;
	int		k;
	int		iOld;
	float64	afMembership[SMART_CORNERS];
	int		iBest;
	float64	fBestMembership;
	float64	fBestWeight;
	float64	fMembership;

	for (i = 0; i < SMART_CORNERS; i++)
	{
		paiGroups[i] = i;
		afMembership[i] = 0.0;
	}

	for (i = 0; i < SMART_CORNERS; i++)
	{
		for (j = i + 1; j < SMART_CORNERS; j++)
		{
			if ((paiGroups[i] != paiGroups[j]) && psSource->IsSimilar(puiPixels[i], puiPixels[j]))
			{
				iOld = paiGroups[j];
				for (k = 0; k < SMART_CORNERS; k++)
				{
					if (paiGroups[k] == iOld)
					{
						paiGroups[k] = paiGroups[i];
					}
				}
			}
		}
	}

	for (i = 0; i < SMART_CORNERS; i++)
	{
		afMembership[paiGroups[i]] += pafWeights[i];
	}

	iBest = paiGroups[0];
	fBestMembership = afMembership[iBest];
	fBestWeight = pafWeights[0];
	for (i = 1; i < SMART_CORNERS; i++)
	{
		fMembership = afMembership[paiGroups[i]];
		if ((fMembership > fBestMembership + SMART_TIE) || ((fMembership > fBestMembership - SMART_TIE) && (pafWeights[i] > fBestWeight + SMART_TIE)))
		{
			iBest = paiGroups[i];
			fBestMembership = fMembership;
			fBestWeight = pafWeights[i];
		}
	}
	return iBest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ResampleSmart(Ptr<CImage> pcSource, Ptr<CImage> pcDest)
{
	SBilinearAxis			sX;
	SBilinearAxis			sY;
	SSmartSource			sSource;
	CImageResampleChannel	cDestChannel;
	size					uiChannel;
	uint32					uiSourceWidth;
	uint32					uiDestWidth;
	uint32					uiDestHeight;
	float64**				papafDest;
	uint32					x;
	uint32					y;
	size					auiPixels[SMART_CORNERS];
	float64					afWeights[SMART_CORNERS];
	int						aiGroups[SMART_CORNERS];
	float64					fX;
	float64					fY;
	int						iGroup;
	int						i;
	float64					fTotal;
	float64					fValue;
	size					uiDestPixel;
	bool					bResult;

	if (!CImageResampleChannel::CanResample(pcSource, pcDest))
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
	sSource.Init(pcSource);

	papafDest = (float64**)malloc(sizeof(float64*) * (sSource.uiNumChannels + 1));
	for (uiChannel = 0; uiChannel < sSource.uiNumChannels; uiChannel++)
	{
		papafDest[uiChannel] = (float64*)malloc(sizeof(float64) * uiDestWidth * uiDestHeight);
	}

	for (y = 0; y < uiDestHeight; y++)
	{
		fY = (float64)sY.paiFraction[y] / BILINEAR_ONE;
		for (x = 0; x < uiDestWidth; x++)
		{
			fX = (float64)sX.paiFraction[x] / BILINEAR_ONE;

			auiPixels[0] = (size)sY.paiFirst[y] * uiSourceWidth + sX.paiFirst[x];
			auiPixels[1] = (size)sY.paiFirst[y] * uiSourceWidth + sX.paiSecond[x];
			auiPixels[2] = (size)sY.paiSecond[y] * uiSourceWidth + sX.paiFirst[x];
			auiPixels[3] = (size)sY.paiSecond[y] * uiSourceWidth + sX.paiSecond[x];
			afWeights[0] = (1.0 - fX) * (1.0 - fY);
			afWeights[1] = fX * (1.0 - fY);
			afWeights[2] = (1.0 - fX) * fY;
			afWeights[3] = fX * fY;

			iGroup = SmartChooseGroup(&sSource, auiPixels, afWeights, aiGroups);

			fTotal = 0.0;
			for (i = 0; i < SMART_CORNERS; i++)
			{
				if (aiGroups[i] == iGroup)
				{
					fTotal += afWeights[i];
				}
			}

			uiDestPixel = (size)y * uiDestWidth + x;
			for (uiChannel = 0; uiChannel < sSource.uiNumChannels; uiChannel++)
			{
				fValue = 0.0;
				for (i = 0; i < SMART_CORNERS; i++)
				{
					if (aiGroups[i] == iGroup)
					{
						fValue += sSource.papafValues[uiChannel][auiPixels[i]] * afWeights[i];
					}
				}
				papafDest[uiChannel][uiDestPixel] = fValue / fTotal;
			}
		}
	}

	bResult = true;
	for (uiChannel = 0; uiChannel < sSource.uiNumChannels; uiChannel++)
	{
		bResult &= cDestChannel.Init(pcDest, uiChannel);
		cDestChannel.SetFloats(papafDest[uiChannel]);
		cDestChannel.Kill();
		free(papafDest[uiChannel]);
	}
	free(papafDest);

	sSource.Kill();
	sY.Kill();
	sX.Kill();
	return bResult;
}

