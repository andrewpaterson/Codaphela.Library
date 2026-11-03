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
#include "ImageAccessorCreator.h"
#include "ImageAccessor.h"
#include "ImageGreyToRGB.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageGreyToRGB::Init(void)
{
	PreInit();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageGreyToRGB::Modify(Ptr<CImage> pImage)
{
	CImageAccessor*		pcRed;
	CImageAccessor*		pcGreen;
	CImageAccessor*		pcBlue;
	CChannel*			psGreyChannel;
	size				x;
	size				y;
	int 				iWidth;
	int 				iHeight;
	void*				pvRed;
	EPrimitiveType		eChannelType;
	bool				bSuccess;

	psGreyChannel = pImage->GetChannel(IMAGE_DIFFUSE_GREY);
	if (psGreyChannel)
	{
		bSuccess = true;
		eChannelType = psGreyChannel->eType;
		bSuccess &= pImage->RenameChannel(IMAGE_DIFFUSE_GREY, IMAGE_DIFFUSE_RED);
		pImage->BeginChange();
		pImage->AddChannel(IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, eChannelType);
		bSuccess &= pImage->EndChange();

		if (!bSuccess)
		{
			return NULL;
		}

		pcRed = CImageAccessorCreator::Create(&pImage, eChannelType, IMAGE_DIFFUSE_RED, CHANNEL_STOP);
		pcGreen = CImageAccessorCreator::Create(&pImage, eChannelType, IMAGE_DIFFUSE_GREEN, CHANNEL_STOP);
		pcBlue = CImageAccessorCreator::Create(&pImage, eChannelType, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);

		iHeight = pImage->GetHeight();
		iWidth = pImage->GetWidth();
		for (y = 0; y < (size)iHeight; y++)
		{
			for (x = 0; x < (size)iWidth; x++)
			{

				pvRed = pcRed->Get((int32)x, (int32)y);
				pcGreen->Set((int32)x, (int32)y, pvRed);
				pcBlue->Set((int32)x, (int32)y, pvRed);
			}
		}
		
		pcRed->Kill();
		pcGreen->Kill();
		pcBlue->Kill();

		return pImage;
	}

	return NULL;
}

