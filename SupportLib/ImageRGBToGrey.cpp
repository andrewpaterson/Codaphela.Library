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
#include "ImageRGBToGrey.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRGBToGrey::Init(ERGBToGreyStyle eStyle)
{
	meStyle = eStyle;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRGBToGrey::Kill(void)
{
	CImageModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRGBToGrey::Modify(CImage* pcImage)
{
	BOOL				bDiffuseChannelsSame;
	ERGBToGreyStyle		eStyle;

	//This will preserve opacity.
	eStyle = meStyle;
	if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO))
	{
		if (eStyle == RGBTGS_OnlyIfChannelsSame)
		{
			bDiffuseChannelsSame = AreChannelsSame(pcImage, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE);
			if (bDiffuseChannelsSame)
			{
				eStyle = RGBTGS_UseRed;
			}
		}
		if (eStyle == RGBTGS_UseRed)
		{
			pcImage->BeginChange();
			pcImage->RemoveChannel(IMAGE_DIFFUSE_GREEN);
			pcImage->RemoveChannel(IMAGE_DIFFUSE_BLUE);
			pcImage->EndChange();
			pcImage->RenameChannel(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREY);
		}
		if (eStyle == RGBTGS_UseAll)
		{
			//Implement me later
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CImageRGBToGrey::AreChannelsSame(CImage* pcImage, EChannel eChannel1, EChannel eChannel2, EChannel eChannel3)
{
	int						x;
	int						y;
	CImageAccessor*			pcAccessor1;
	CImageAccessor*			pcAccessor2;
	CImageAccessor*			pcAccessor3;
	int						iWidth;
	int						iHeight;
	int						iSize;
	SImageColour*			psColour1;
	SImageColour*			psColour2;
	SImageColour*			psColour3;

	pcAccessor1 = CImageAccessorCreator::Create(pcImage, eChannel1, CHANNEL_ZERO);
	pcAccessor2 = CImageAccessorCreator::Create(pcImage, eChannel2, CHANNEL_ZERO);
	pcAccessor3 = CImageAccessorCreator::Create(pcImage, eChannel3, CHANNEL_ZERO);
	if ((pcAccessor1->GetBitSize() != pcAccessor2->GetBitSize()) && (pcAccessor1->GetBitSize() != pcAccessor3->GetBitSize()))
	{
		return FALSE;
	}

	iSize = pcAccessor1->GetByteSize();

	iWidth = pcImage->GetWidth();
	iHeight = pcImage->GetHeight();
	for (y = 0; y < iWidth; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			psColour1 = (SImageColour*)pcAccessor1->Get(x, y);
			psColour2 = (SImageColour*)pcAccessor2->Get(x, y);
			psColour3 = (SImageColour*)pcAccessor3->Get(x, y);

			if ((!psColour1->Equals(psColour2, iSize)) || (!psColour1->Equals(psColour3, iSize)))
			{
				pcAccessor1->Kill();
				pcAccessor2->Kill();
				pcAccessor3->Kill();
				return FALSE;
			}
		}
	}
	pcAccessor1->Kill();
	pcAccessor2->Kill();
	pcAccessor3->Kill();
	return TRUE;
}

