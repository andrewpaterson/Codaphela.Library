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
#include "ImageAccessor.h"
#include "ImageGreyToRGB.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageGreyToRGB::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageGreyToRGB::Kill(void)
{
	CImageModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageGreyToRGB::Modify(CImage* pcImage)
{
	CImageAccessor*			pcRed;
	CImageAccessor*			pcGreen;
	CImageAccessor*			pcBlue;
	CChannel*				psGreyChannel;
	int						x;
	int						y;
	int 					iWidth;
	int 					iHeight;
	void*					pvRed;
	EPrimitiveType			eChannelType;

	psGreyChannel = pcImage->GetChannel(IMAGE_DIFFUSE_GREY);
	if (psGreyChannel)
	{
		eChannelType = psGreyChannel->eType;
		pcImage->RenameChannel(IMAGE_DIFFUSE_GREY, IMAGE_DIFFUSE_RED);
		pcImage->BeginChange();
		pcImage->AddChannel(IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, eChannelType);
		pcImage->EndChange();

		pcRed = CImageAccessorCreator::Create(pcImage, eChannelType, IMAGE_DIFFUSE_RED, CHANNEL_ZERO);
		pcGreen = CImageAccessorCreator::Create(pcImage, eChannelType, IMAGE_DIFFUSE_GREEN, CHANNEL_ZERO);
		pcBlue = CImageAccessorCreator::Create(pcImage, eChannelType, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO);

		iHeight = pcImage->GetHeight();
		iWidth = pcImage->GetWidth();
		for (y = 0; y < iHeight; y++)
		{
			for (x = 0; x < iWidth; x++)
			{

				pvRed = pcRed->Get(x, y);
				pcGreen->Set(x, y, pvRed);
				pcBlue->Set(x, y, pvRed);
			}
		}
		
		pcRed->Kill();
		pcGreen->Kill();
		pcBlue->Kill();
	}
}

