/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "SupportLib/Image.h"
#include "ImageTexels.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool GetImageTexels(CImage* pcImage, uint32* puiTexels)
{
	CChannel*	pcChannel;
	uint8*		puiImage;
	size		uiNumChannels;
	size		uiNumPixels;
	size		i;
	size		j;
	int			aiShift[16];
	uint32		uiTexel;

	if ((pcImage->GetPrimitiveType() != PT_uint8) || (pcImage->NumChannels() == 0) || (pcImage->NumChannels() > 16))
	{
		return false;
	}
	uiNumChannels = pcImage->NumChannels();
	for (j = 0; j < uiNumChannels; j++)
	{
		pcChannel = pcImage->GetChannelAtIndex(j);
		aiShift[j] = (pcChannel->iChannel == IMAGE_DIFFUSE_RED) ? 16 : ((pcChannel->iChannel == IMAGE_DIFFUSE_GREEN) ? 8 : ((pcChannel->iChannel == IMAGE_DIFFUSE_BLUE) ? 0 : ((pcChannel->iChannel == IMAGE_OPACITY) ? 24 : -1)));
	}

	uiNumPixels = (size)pcImage->GetWidth() * pcImage->GetHeight();
	puiImage = (uint8*)pcImage->GetData();
	for (i = 0; i < uiNumPixels; i++)
	{
		uiTexel = 0xFFFFFFFF;
		for (j = 0; j < uiNumChannels; j++)
		{
			if (aiShift[j] >= 0)
			{
				uiTexel = (uiTexel & ~((uint32)0xFF << aiShift[j])) | ((uint32)puiImage[i * uiNumChannels + j] << aiShift[j]);
			}
		}
		puiTexels[i] = uiTexel;
	}
	return true;
}

