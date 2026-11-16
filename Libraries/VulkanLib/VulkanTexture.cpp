/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela VulkanLib

Codaphela VulkanLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela VulkanLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela VulkanLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "StandardLib/ClassDefines.h"
#include "VulkanTexture.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanTexture::Init(Ptr<CVulkanRenderer> pRenderer, OIndex oiImage, SVulkanTexture* psTexture)
{
	PreInit();
	mpRenderer = pRenderer;
	moiImage = oiImage;
	msTexture = *psTexture;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanTexture::Class(void)
{
	Nes(M_Pointer(mpRenderer));
	U_Data(SVulkanTexture, msTexture);
	U_Int64(moiImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanTexture::Free(void)
{
	Ptr<CVulkanRenderer>	pRenderer;

	pRenderer = mpRenderer;
	if (pRenderer.IsNotNull())
	{
		pRenderer->DestroyTexture(&msTexture);
	}
	memset(&msTexture, 0, sizeof(SVulkanTexture));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanTexture::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanTexture::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
VkDescriptorSet CVulkanTexture::GetSet(void) { return msTexture.hSet; }
OIndex CVulkanTexture::GetImageIndex(void) { return moiImage; }

