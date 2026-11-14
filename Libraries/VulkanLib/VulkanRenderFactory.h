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
#ifndef __VULKAN_RENDER_FACTORY_H__
#define __VULKAN_RENDER_FACTORY_H__
#include "RenderLib/RenderFactory.h"


//Makes Vulkan renderers, and the meshes they draw.  A renderer that finds no Vulkan isn't ready, and its meshes aren't drawn.
class CVulkanRenderFactory : public CRenderFactory
{
CONSTRUCTABLE(CVulkanRenderFactory);
public:
	void				Init(void);
	void				Kill(void);

	Ptr<CRenderer>		CreateRenderer(void) override;
	Ptr<CRenderMesh>	CreateMesh(Ptr<CRenderer> pRenderer, CCookedMesh* pcCooked) override;
};


#endif // __VULKAN_RENDER_FACTORY_H__

