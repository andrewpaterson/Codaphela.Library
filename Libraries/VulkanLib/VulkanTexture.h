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
#ifndef __VULKAN_TEXTURE_H__
#define __VULKAN_TEXTURE_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "VulkanRenderer.h"


//A texture on the GPU (its image at every mip level and the descriptor set the shaders sample it by), made by a renderer for a
//material image (CVulkanRenderer::GetTexture).  The render meshes drawn with it hold it; the renderer only finds it, weakly
//(WeakReferencesPlan.txt), so when the last of those meshes goes it's freed and its image and set given back.  It holds its
//renderer (Nes), whose device must outlive it.  It isn't saved.
class CVulkanTexture : public CObject
{
CONSTRUCTABLE(CVulkanTexture);
DESTRUCTABLE(CVulkanTexture);
protected:
	Ptr<CVulkanRenderer>	mpRenderer;
	SVulkanTexture			msTexture;
	OIndex					moiImage;  //What it was made from.

public:
	void			Init(Ptr<CVulkanRenderer> pRenderer, OIndex oiImage, SVulkanTexture* psTexture);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	VkDescriptorSet	GetSet(void) const;
	OIndex			GetImageIndex(void) const;
};


#endif // __VULKAN_TEXTURE_H__

