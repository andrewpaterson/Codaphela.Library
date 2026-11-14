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
#ifndef __VULKAN_RENDER_MESH_H__
#define __VULKAN_RENDER_MESH_H__
#include "BaseLib/ArrayTemplate.h"
#include "RenderLib/CookedMesh.h"
#include "RenderLib/RenderMesh.h"
#include "VulkanRenderer.h"


//Some of a mesh's triangles, drawn in one colour (0xAARRGGBB) times their vertices', and transparent or not.
struct SVulkanSubmesh
{
	uint32	uiFirstIndex;
	uint32	uiNumIndices;
	uint32	uiColour;
	bool	bTransparent;
};


typedef CArrayTemplate<SVulkanSubmesh>	CArrayVulkanSubmesh;


//A mesh as the Vulkan renderer draws it, made from a cooked mesh: buffers of its positions, normals, colours and indices, and its
//submeshes.  A mesh with no vertex colours has white ones, and its submeshes' colours are its materials'.  One with vertex colours
//draws them, its submeshes white.  Its buffers are destroyed when it's freed, as nothing is in flight between frames (see
//CVulkanRenderer).
class CVulkanRenderMesh : public CRenderMesh
{
CONSTRUCTABLE(CVulkanRenderMesh);
DESTRUCTABLE(CVulkanRenderMesh);
protected:
	SVulkanBuffer		msPositions;
	SVulkanBuffer		msNormals;
	SVulkanBuffer		msColours;
	SVulkanBuffer		msIndices;
	CArrayVulkanSubmesh	masSubmeshes;
	bool				mbMade;

public:
	void			Init(Ptr<CVulkanRenderer> pRenderer, CCookedMesh* pcCooked);
	void			Class(void) override;
	void			Free(void) override;

	bool			IsDrawable(void);
	void			Bind(void);
	size			NumSubmeshes(void);
	SVulkanSubmesh*	GetSubmesh(size uiIndex);

protected:
	bool			Make(CVulkanRenderer* pcRenderer, CCookedMesh* pcCooked);
};


#endif // __VULKAN_RENDER_MESH_H__

