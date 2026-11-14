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
#include "VulkanRenderMesh.h"


#define VULKAN_WHITE	0xFFFFFFFF


//////////////////////////////////////////////////////////////////////////
//
// A mesh the renderer couldn't make buffers for isn't drawn.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderMesh::Init(Ptr<CVulkanRenderer> pRenderer, CCookedMesh* pcCooked)
{
	PreInit();

	CRenderMesh::Init(pRenderer);
	memset(&msPositions, 0, sizeof(SVulkanBuffer));
	memset(&msNormals, 0, sizeof(SVulkanBuffer));
	memset(&msColours, 0, sizeof(SVulkanBuffer));
	memset(&msIndices, 0, sizeof(SVulkanBuffer));
	masSubmeshes.Init();
	mbMade = pRenderer->IsReady() && (pcCooked->NumTriangles() > 0) && Make(&pRenderer, pcCooked);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderMesh::Class(void)
{
	CRenderMesh::Class();
	U_Data(SVulkanBuffer, msPositions);
	U_Data(SVulkanBuffer, msNormals);
	U_Data(SVulkanBuffer, msColours);
	U_Data(SVulkanBuffer, msIndices);
	U_Unknown(CArrayVulkanSubmesh, masSubmeshes);
	U_Bool(mbMade);
}


//////////////////////////////////////////////////////////////////////////
//
// Its renderer is freed after it (CRenderMesh), so it's there to destroy
// the buffers with.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderMesh::Free(void)
{
	Ptr<CVulkanRenderer>	pRenderer;

	pRenderer = mpRenderer;
	if (pRenderer.IsNotNull())
	{
		pRenderer->DestroyBuffer(&msIndices);
		pRenderer->DestroyBuffer(&msColours);
		pRenderer->DestroyBuffer(&msNormals);
		pRenderer->DestroyBuffer(&msPositions);
	}
	masSubmeshes.Kill();
	CRenderMesh::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderMesh::Make(CVulkanRenderer* pcRenderer, CCookedMesh* pcCooked)
{
	SVulkanSubmesh*	psSubmesh;
	SCookedSubmesh*	psCooked;
	uint32*			puiColours;
	size			uiNumVertices;
	size			i;

	uiNumVertices = pcCooked->NumVertices();
	if (!pcRenderer->CreateBuffer(&msPositions, uiNumVertices * sizeof(SFloat32Vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ||
		!pcRenderer->CreateBuffer(&msNormals, uiNumVertices * sizeof(SFloat32Vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ||
		!pcRenderer->CreateBuffer(&msColours, uiNumVertices * sizeof(uint32), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ||
		!pcRenderer->CreateBuffer(&msIndices, pcCooked->mauiIndices.NumElements() * sizeof(uint32), VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
	{
		return false;
	}

	memcpy(msPositions.pvMapped, pcCooked->masPositions.GetData(), uiNumVertices * sizeof(SFloat32Vec3));
	memcpy(msNormals.pvMapped, pcCooked->masNormals.GetData(), uiNumVertices * sizeof(SFloat32Vec3));
	memcpy(msIndices.pvMapped, pcCooked->mauiIndices.GetData(), pcCooked->mauiIndices.NumElements() * sizeof(uint32));
	puiColours = (uint32*)msColours.pvMapped;
	for (i = 0; i < uiNumVertices; i++)
	{
		puiColours[i] = pcCooked->HasColours() ? pcCooked->mauiColours.GetValue(i) : VULKAN_WHITE;
	}

	for (i = 0; i < pcCooked->masSubmeshes.NumElements(); i++)
	{
		psCooked = pcCooked->masSubmeshes.Get(i);
		psSubmesh = masSubmeshes.Add();
		psSubmesh->uiFirstIndex = psCooked->uiFirstIndex;
		psSubmesh->uiNumIndices = psCooked->uiNumIndices;
		psSubmesh->uiColour = pcCooked->HasColours() ? VULKAN_WHITE : psCooked->uiColour;
		psSubmesh->bTransparent = psCooked->bTransparent;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Its buffers, for the frame's draws.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderMesh::Bind(void)
{
	Ptr<CVulkanRenderer>	pRenderer;
	VkBuffer				ahBuffers[3];
	VkDeviceSize			auiOffsets[3];

	pRenderer = mpRenderer;
	ahBuffers[0] = msPositions.hBuffer;
	ahBuffers[1] = msNormals.hBuffer;
	ahBuffers[2] = msColours.hBuffer;
	auiOffsets[0] = 0;
	auiOffsets[1] = 0;
	auiOffsets[2] = 0;
	pRenderer->GetFunctions()->vkCmdBindVertexBuffers(pRenderer->GetCommands(), 0, 3, ahBuffers, auiOffsets);
	pRenderer->GetFunctions()->vkCmdBindIndexBuffer(pRenderer->GetCommands(), msIndices.hBuffer, 0, VK_INDEX_TYPE_UINT32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderMesh::IsDrawable(void) { return mbMade; }
size CVulkanRenderMesh::NumSubmeshes(void) { return masSubmeshes.NumElements(); }
SVulkanSubmesh* CVulkanRenderMesh::GetSubmesh(size uiIndex) { return masSubmeshes.Get(uiIndex); }

