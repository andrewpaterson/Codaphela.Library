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
#include "BaseLib/HashMapTemplate.h"
#include "BaseLib/PrimitiveTypes.h"
#include "StandardLib/ChannelsAccessorCreator.h"
#include "ImageChannel.h"
#include "SFTQuantiser.h"


#define SFT_QUANTISE_TRANSPARENT	0xFFFFFFFF		//A pixel's colour while it is read: not drawn.
#define SFT_QUANTISE_OPAQUE			128				//Opacity from which a pixel is drawn.


//A colour the images have (red low byte first), how many pixels have it, and its palette index.
struct SSFTColourCount
{
	uint32	uiColour;
	uint32	uiCount;
	uint16	uiIndex;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint32 SFTHashColour(uint32 uiColour)
{
	uiColour ^= uiColour >> 13;
	uiColour *= 0x5BD1E995;
	return uiColour ^ (uiColour >> 15);
}


struct SHashOfSFTColour
{
	static uint32		Hash(const uint32* puiColour) { return SFTHashColour(*puiColour); }
	static HashEquals	Equals(void) { return NULL; }
};


//The colours, and each one's place in the list of them.
struct SSFTColourTable
{
	CHashMapTemplate<uint32, uint32, SHashOfSFTColour, SArrayLargeSize>	cPlaces;
	SSFTColourCount*	pasColours;
	uint32				uiNumColours;
	uint32				uiMaxColours;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSFTQuantised::Init(void)
{
	msColours.Init();
	masImages = NULL;
	muiNumImages = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSFTQuantised::Kill(void)
{
	size	i;

	for (i = 0; i < muiNumImages; i++)
	{
		free(masImages[i].puiPixels);
	}
	free(masImages);
	masImages = NULL;
	muiNumImages = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 SFTChannel(uint32 uiColour, size uiChannel)
{
	return (uint8)(uiColour >> (uiChannel * 8));
}


//////////////////////////////////////////////////////////////////////////
//
// The place of uiColour in the table's list, which it is put in (with a
// count of 0) if it isn't.
//
//////////////////////////////////////////////////////////////////////////
static uint32 SFTFindColour(SSFTColourTable* psTable, uint32 uiColour)
{
	uint32*	puiPlace;
	bool	bAdded;

	puiPlace = psTable->cPlaces.FindOrAdd(&uiColour, &bAdded);
	if (bAdded)
	{
		*puiPlace = psTable->uiNumColours;
		psTable->pasColours[psTable->uiNumColours].uiColour = uiColour;
		psTable->pasColours[psTable->uiNumColours].uiCount = 0;
		psTable->pasColours[psTable->uiNumColours].uiIndex = 0;
		psTable->uiNumColours++;
	}
	return *puiPlace;
}


//////////////////////////////////////////////////////////////////////////
//
// The colours from uiStart to before uiEnd in order of their uiChannel
// (0 red, 1 green, 2 blue; 3 the whole colour), then of the whole colour
// so that the order is the same however they started.
//
//////////////////////////////////////////////////////////////////////////
static bool SFTColourBefore(const SSFTColourCount* ps1, const SSFTColourCount* ps2, size uiChannel)
{
	uint8	ui1;
	uint8	ui2;

	if (uiChannel < 3)
	{
		ui1 = SFTChannel(ps1->uiColour, uiChannel);
		ui2 = SFTChannel(ps2->uiColour, uiChannel);
		if (ui1 != ui2)
		{
			return ui1 < ui2;
		}
	}
	return ps1->uiColour < ps2->uiColour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SFTSortColours(SSFTColourCount* pasColours, uint32 uiStart, uint32 uiEnd, size uiChannel)
{
	SSFTColourCount	sPivot;
	SSFTColourCount	sSwap;
	uint32			uiLow;
	uint32			uiHigh;
	uint32			i;

	while (uiEnd - uiStart > 1)
	{
		//The middle one as the pivot, then Lomuto's partition; the smaller side is sorted by calling, the larger by
		//going round again, so the depth is at most log2 of the count.
		sSwap = pasColours[(uiStart + uiEnd) / 2];
		pasColours[(uiStart + uiEnd) / 2] = pasColours[uiEnd - 1];
		pasColours[uiEnd - 1] = sSwap;
		sPivot = pasColours[uiEnd - 1];
		uiLow = uiStart;
		for (i = uiStart; i < uiEnd - 1; i++)
		{
			if (SFTColourBefore(&pasColours[i], &sPivot, uiChannel))
			{
				sSwap = pasColours[i];
				pasColours[i] = pasColours[uiLow];
				pasColours[uiLow] = sSwap;
				uiLow++;
			}
		}
		pasColours[uiEnd - 1] = pasColours[uiLow];
		pasColours[uiLow] = sPivot;
		uiHigh = uiLow + 1;
		if (uiLow - uiStart < uiEnd - uiHigh)
		{
			SFTSortColours(pasColours, uiStart, uiLow, uiChannel);
			uiStart = uiHigh;
		}
		else
		{
			SFTSortColours(pasColours, uiHigh, uiEnd, uiChannel);
			uiEnd = uiLow;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The channel the colours from uiStart to before uiEnd spread over most,
// and in *puiRange how far.
//
//////////////////////////////////////////////////////////////////////////
static size SFTWidestChannel(const SSFTColourCount* pasColours, uint32 uiStart, uint32 uiEnd, uint32* puiRange)
{
	uint8	auiLeast[3];
	uint8	auiMost[3];
	uint8	uiValue;
	uint32	i;
	size	c;
	size	uiWidest;

	for (c = 0; c < 3; c++)
	{
		auiLeast[c] = 255;
		auiMost[c] = 0;
	}
	for (i = uiStart; i < uiEnd; i++)
	{
		for (c = 0; c < 3; c++)
		{
			uiValue = SFTChannel(pasColours[i].uiColour, c);
			if (uiValue < auiLeast[c])
			{
				auiLeast[c] = uiValue;
			}
			if (uiValue > auiMost[c])
			{
				auiMost[c] = uiValue;
			}
		}
	}
	uiWidest = 0;
	for (c = 1; c < 3; c++)
	{
		if ((auiMost[c] - auiLeast[c]) > (auiMost[uiWidest] - auiLeast[uiWidest]))
		{
			uiWidest = c;
		}
	}
	*puiRange = (uint32)(auiMost[uiWidest] - auiLeast[uiWidest]);
	return uiWidest;
}


//////////////////////////////////////////////////////////////////////////
//
// uiBoxes boxes of the colours (their starts in puiStarts, the last's end
// the count): the box spread widest is split at the weighted median of
// that channel until there are enough or none can be.  Returns how many.
//
//////////////////////////////////////////////////////////////////////////
static uint32 SFTMedianCut(SSFTColourCount* pasColours, uint32 uiNumColours, uint32 uiBoxes, uint32* puiStarts)
{
	uint32	uiNumBoxes;
	uint32	uiBox;
	uint32	uiBest;
	uint32	uiBestRange;
	uint32	uiRange;
	uint32	uiStart;
	uint32	uiEnd;
	uint32	uiTotal;
	uint32	uiSoFar;
	uint32	uiSplit;
	size	uiChannel;
	uint32	i;

	puiStarts[0] = 0;
	uiNumBoxes = 1;
	while (uiNumBoxes < uiBoxes)
	{
		uiBest = uiNumBoxes;
		uiBestRange = 0;
		for (uiBox = 0; uiBox < uiNumBoxes; uiBox++)
		{
			uiStart = puiStarts[uiBox];
			uiEnd = (uiBox + 1 < uiNumBoxes) ? puiStarts[uiBox + 1] : uiNumColours;
			if (uiEnd - uiStart < 2)
			{
				continue;
			}
			SFTWidestChannel(pasColours, uiStart, uiEnd, &uiRange);
			if ((uiBest == uiNumBoxes) || (uiRange > uiBestRange))
			{
				uiBest = uiBox;
				uiBestRange = uiRange;
			}
		}
		if (uiBest == uiNumBoxes)
		{
			break;
		}

		uiStart = puiStarts[uiBest];
		uiEnd = (uiBest + 1 < uiNumBoxes) ? puiStarts[uiBest + 1] : uiNumColours;
		uiChannel = SFTWidestChannel(pasColours, uiStart, uiEnd, &uiRange);
		SFTSortColours(pasColours, uiStart, uiEnd, uiChannel);
		uiTotal = 0;
		for (i = uiStart; i < uiEnd; i++)
		{
			uiTotal += pasColours[i].uiCount;
		}
		uiSoFar = 0;
		uiSplit = uiStart + 1;
		for (i = uiStart; i < uiEnd - 1; i++)
		{
			uiSoFar += pasColours[i].uiCount;
			uiSplit = i + 1;
			if (uiSoFar * 2 >= uiTotal)
			{
				break;
			}
		}

		//The boxes after it move up one, and the second half is the box after it.
		for (uiBox = uiNumBoxes; uiBox > uiBest + 1; uiBox--)
		{
			puiStarts[uiBox] = puiStarts[uiBox - 1];
		}
		puiStarts[uiBest + 1] = uiSplit;
		uiNumBoxes++;
	}
	return uiNumBoxes;
}


//////////////////////////////////////////////////////////////////////////
//
// The palette entry nearest uiColour: the least squared distance, the
// lower index if two are as near.
//
//////////////////////////////////////////////////////////////////////////
static uint16 SFTNearest(const SSFTColours* psColours, uint32 uiColour)
{
	uint32	uiBest;
	uint32	uiDistance;
	int32	iRed;
	int32	iGreen;
	int32	iBlue;
	uint16	uiBestIndex;
	uint16	i;

	uiBest = 0xFFFFFFFF;
	uiBestIndex = psColours->uiFirst;
	for (i = psColours->uiFirst; i < psColours->uiFirst + psColours->uiCount; i++)
	{
		iRed = (int32)psColours->auiRed[i] - SFTChannel(uiColour, 0);
		iGreen = (int32)psColours->auiGreen[i] - SFTChannel(uiColour, 1);
		iBlue = (int32)psColours->auiBlue[i] - SFTChannel(uiColour, 2);
		uiDistance = (uint32)(iRed * iRed + iGreen * iGreen + iBlue * iBlue);
		if (uiDistance < uiBest)
		{
			uiBest = uiDistance;
			uiBestIndex = i;
		}
	}
	return uiBestIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// The image's pixels as colours (SFT_QUANTISE_TRANSPARENT for one not
// drawn) in puiColours, and whether any is transparent.  False if it
// hasn't red, green and blue.
//
//////////////////////////////////////////////////////////////////////////
static bool SFTReadColours(Ptr<CImage> pImage, uint32* puiColours, bool* pbTransparent)
{
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;
	uint8*						puiPixel;
	bool						bOpacity;
	uint32						uiPixels;
	uint32						i;

	if (!pImage->HasChannel(IMAGE_DIFFUSE_RED) || !pImage->HasChannel(IMAGE_DIFFUSE_GREEN) || !pImage->HasChannel(IMAGE_DIFFUSE_BLUE))
	{
		return false;
	}
	bOpacity = pImage->HasChannel(IMAGE_OPACITY);
	cCreator.Init(pImage->GetChannels());
	cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	if (bOpacity)
	{
		cCreator.AddAccess(IMAGE_OPACITY, PT_uint8);
	}
	pcAccessor = cCreator.CreateAndKill();
	if (!pcAccessor)
	{
		return false;
	}

	*pbTransparent = false;
	uiPixels = pImage->GetWidth() * pImage->GetHeight();
	for (i = 0; i < uiPixels; i++)
	{
		puiPixel = (uint8*)pcAccessor->Get(i);
		if (bOpacity && (puiPixel[3] < SFT_QUANTISE_OPAQUE))
		{
			puiColours[i] = SFT_QUANTISE_TRANSPARENT;
			*pbTransparent = true;
		}
		else
		{
			puiColours[i] = (uint32)puiPixel[0] | ((uint32)puiPixel[1] << 8) | ((uint32)puiPixel[2] << 16);
		}
	}
	pcAccessor->Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool QuantiseSFT(CArrayImage* pacImages, uint16 uiMaxColours, CSFTQuantised* pcResult)
{
	SSFTColourTable		sTable;
	Ptr<CImage>			pImage;
	uint32**			papuiColours;
	uint32*				puiStarts;
	uint32				uiNumImages;
	uint32				uiPixels;
	uint32				uiAllPixels;
	uint32				uiLimit;
	uint32				uiNumBoxes;
	uint32				uiStart;
	uint32				uiEnd;
	uint32				uiPlace;
	uint32				auiSums[3];
	uint32				uiCount;
	uint32				i;
	uint32				j;
	size				c;
	bool				bTransparent;
	bool				bAnyTransparent;
	bool				bResult;
	uint16				uiFirst;
	SSFTIndexedImage*	psImage;

	pcResult->Init();
	uiNumImages = (uint32)pacImages->NumElements();
	if (uiNumImages == 0)
	{
		return false;
	}
	pcResult->masImages = (SSFTIndexedImage*)malloc(sizeof(SSFTIndexedImage) * uiNumImages);
	papuiColours = (uint32**)malloc(sizeof(uint32*) * uiNumImages);
	memset(pcResult->masImages, 0, sizeof(SSFTIndexedImage) * uiNumImages);
	memset(papuiColours, 0, sizeof(uint32*) * uiNumImages);
	pcResult->muiNumImages = uiNumImages;

	//Each image's colours.
	bResult = true;
	bAnyTransparent = false;
	uiAllPixels = 0;
	for (i = 0; (i < uiNumImages) && bResult; i++)
	{
		pImage = pacImages->Get(i);
		if (pImage.IsNull() || (pImage->GetWidth() == 0) || (pImage->GetHeight() == 0) || (pImage->GetWidth() > 0xFFFF) || (pImage->GetHeight() > 0xFFFF))
		{
			bResult = false;
			break;
		}
		uiPixels = pImage->GetWidth() * pImage->GetHeight();
		papuiColours[i] = (uint32*)malloc(sizeof(uint32) * uiPixels);
		bResult = SFTReadColours(pImage, papuiColours[i], &bTransparent);
		psImage = &pcResult->masImages[i];
		psImage->uiWidth = (uint16)pImage->GetWidth();
		psImage->uiHeight = (uint16)pImage->GetHeight();
		psImage->bTransparent = bTransparent;
		psImage->puiPixels = (uint8*)malloc(uiPixels);
		bAnyTransparent = bAnyTransparent || bTransparent;
		uiAllPixels += uiPixels;
	}

	//The colours there are and how many pixels have each.
	sTable.cPlaces.Init();
	sTable.pasColours = NULL;
	if (bResult)
	{
		sTable.pasColours = (SSFTColourCount*)malloc(sizeof(SSFTColourCount) * (uiAllPixels + 1));
		sTable.uiNumColours = 0;
		sTable.uiMaxColours = uiAllPixels;
		for (i = 0; i < uiNumImages; i++)
		{
			uiPixels = (uint32)pcResult->masImages[i].uiWidth * pcResult->masImages[i].uiHeight;
			for (j = 0; j < uiPixels; j++)
			{
				if (papuiColours[i][j] != SFT_QUANTISE_TRANSPARENT)
				{
					sTable.pasColours[SFTFindColour(&sTable, papuiColours[i][j])].uiCount++;
				}
			}
		}

		//The palette: the colours themselves if there are few enough, else median cut's boxes' means.
		uiFirst = bAnyTransparent ? 1 : 0;
		uiLimit = (uint32)SFT_PALETTE_COLOURS - uiFirst;
		if ((uiMaxColours > 0) && (uiMaxColours < uiLimit))
		{
			uiLimit = uiMaxColours;
		}
		pcResult->msColours.uiFirst = uiFirst;
		if (sTable.uiNumColours == 0)
		{
			//Nothing is drawn: one black colour, so that the palette isn't empty.
			pcResult->msColours.uiCount = 1;
			pcResult->msColours.auiRed[uiFirst] = 0;
			pcResult->msColours.auiGreen[uiFirst] = 0;
			pcResult->msColours.auiBlue[uiFirst] = 0;
		}
		else if (sTable.uiNumColours <= uiLimit)
		{
			pcResult->msColours.uiCount = (uint16)sTable.uiNumColours;
			for (i = 0; i < sTable.uiNumColours; i++)
			{
				uiPlace = uiFirst + i;
				pcResult->msColours.auiRed[uiPlace] = SFTChannel(sTable.pasColours[i].uiColour, 0);
				pcResult->msColours.auiGreen[uiPlace] = SFTChannel(sTable.pasColours[i].uiColour, 1);
				pcResult->msColours.auiBlue[uiPlace] = SFTChannel(sTable.pasColours[i].uiColour, 2);
				sTable.pasColours[i].uiIndex = (uint16)uiPlace;
			}
		}
		else
		{
			//Median cut moves the colours about in the list, so the table's places are found again after.
			SFTSortColours(sTable.pasColours, 0, sTable.uiNumColours, 3);
			puiStarts = (uint32*)malloc(sizeof(uint32) * (uiLimit + 1));
			uiNumBoxes = SFTMedianCut(sTable.pasColours, sTable.uiNumColours, uiLimit, puiStarts);
			pcResult->msColours.uiCount = (uint16)uiNumBoxes;
			for (i = 0; i < uiNumBoxes; i++)
			{
				uiStart = puiStarts[i];
				uiEnd = (i + 1 < uiNumBoxes) ? puiStarts[i + 1] : sTable.uiNumColours;
				auiSums[0] = auiSums[1] = auiSums[2] = 0;
				uiCount = 0;
				for (j = uiStart; j < uiEnd; j++)
				{
					for (c = 0; c < 3; c++)
					{
						auiSums[c] += SFTChannel(sTable.pasColours[j].uiColour, c) * sTable.pasColours[j].uiCount;
					}
					uiCount += sTable.pasColours[j].uiCount;
				}
				uiPlace = uiFirst + i;
				pcResult->msColours.auiRed[uiPlace] = (uint8)((auiSums[0] + uiCount / 2) / uiCount);
				pcResult->msColours.auiGreen[uiPlace] = (uint8)((auiSums[1] + uiCount / 2) / uiCount);
				pcResult->msColours.auiBlue[uiPlace] = (uint8)((auiSums[2] + uiCount / 2) / uiCount);
			}
			free(puiStarts);
			for (i = 0; i < sTable.uiNumColours; i++)
			{
				sTable.pasColours[i].uiIndex = SFTNearest(&pcResult->msColours, sTable.pasColours[i].uiColour);
			}
			sTable.cPlaces.ReInit();
			sTable.cPlaces.Reserve(sTable.uiNumColours);
			for (i = 0; i < sTable.uiNumColours; i++)
			{
				sTable.cPlaces.Put(&sTable.pasColours[i].uiColour, &i);
			}
		}

		//Each pixel its colour's index.
		for (i = 0; i < uiNumImages; i++)
		{
			psImage = &pcResult->masImages[i];
			uiPixels = (uint32)psImage->uiWidth * psImage->uiHeight;
			for (j = 0; j < uiPixels; j++)
			{
				psImage->puiPixels[j] = (papuiColours[i][j] == SFT_QUANTISE_TRANSPARENT) ? (uint8)SFT_PIXEL_TRANSPARENT :
										(uint8)sTable.pasColours[SFTFindColour(&sTable, papuiColours[i][j])].uiIndex;
			}
		}
	}

	sTable.cPlaces.Kill();
	free(sTable.pasColours);
	for (i = 0; i < uiNumImages; i++)
	{
		free(papuiColours[i]);
	}
	free(papuiColours);
	if (!bResult)
	{
		pcResult->Kill();
	}
	return bResult;
}
