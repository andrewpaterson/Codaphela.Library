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
#include "ImageCopier.h"
#include "ImageToR3G3B2A.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageR3G3B2A::Init(void)
{
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageR3G3B2A::Kill(void)
{
	CImageModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageR3G3B2A::Modify(Ptr<CImage> pcImage)
{
	if (!pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO))
	{
		return false;
	}

	Ptr<CImage>			pcDest;
	CChannelsAccessor*	pcAccessor;
	size				uiNumElements;
	size				i;
	size				uiNumOpaque;
	size				uiNumTransparent;
	uint8				uiAlpha;

	pcDest = OMalloc<CImage>();
	pcDest->BeginChange();
	pcDest->SetSize(pcImage->GetWidth(), pcImage->GetHeight());
	pcDest->AddChannel(pcImage, IMAGE_DIFFUSE_RED, PT_tribble);
	pcDest->AddChannel(pcImage, IMAGE_DIFFUSE_GREEN, PT_tribble);
	pcDest->AddChannel(pcImage, IMAGE_DIFFUSE_BLUE, PT_crumb);
	if (pcImage->HasChannel(IMAGE_OPACITY))
	{
		pcDest->AddChannel(pcImage, IMAGE_OPACITY, PT_uint8);
	}
	pcDest->EndChange();
	
	CImageCopier::Copy(pcImage, pcDest);

	if (pcDest->HasChannel(IMAGE_OPACITY))
	{
		uiNumOpaque = 0;
		uiNumTransparent = 0;

		pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pcImage->GetChannels(), IMAGE_OPACITY);
		uiNumElements = pcImage->GetPixelSize();
		for (i = 0; i < uiNumElements; i++)
		{
			uiAlpha = *((uint8*)pcAccessor->Get(i));
			if (uiAlpha >= 128)
			{
				uiAlpha = 255;
				uiNumOpaque++;
			}
			else
			{
				uiAlpha = 0;
				uiNumTransparent++;
			}
			pcAccessor->Set(i, &uiAlpha);
		}
		pcAccessor->Kill();

		if ((uiNumOpaque == uiNumElements) || (uiNumTransparent == uiNumElements))
		{
			pcDest->BeginChange();
			pcDest->RemoveChannel(IMAGE_OPACITY);
			pcDest->EndChange();
		}
	}

	return pcDest;
}

