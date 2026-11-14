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
	memset(&msUVs, 0, sizeof(SVulkanBuffer));
	memset(&msIndices, 0, sizeof(SVulkanBuffer));
	memset(&msSkinVertices, 0, sizeof(SVulkanBuffer));
	memset(&msSkinWeights, 0, sizeof(SVulkanBuffer));
	mhSkin = VK_NULL_HANDLE;
	muiNumMatrices = 0;
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
	U_Data(SVulkanBuffer, msUVs);
	U_Data(SVulkanBuffer, msIndices);
	U_Data(SVulkanBuffer, msSkinVertices);
	U_Data(SVulkanBuffer, msSkinWeights);
	U_Pointer(mhSkin);
	U_UInt32(muiNumMatrices);
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
		pRenderer->FreeSkin(mhSkin);
		pRenderer->DestroyBuffer(&msSkinWeights);
		pRenderer->DestroyBuffer(&msSkinVertices);
		pRenderer->DestroyBuffer(&msIndices);
		pRenderer->DestroyBuffer(&msUVs);
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
	Ptr<CMaterial>	pMaterial;
	uint32*			puiColours;
	size			uiNumVertices;
	size			i;

	uiNumVertices = pcCooked->NumVertices();
	if (!pcRenderer->CreateBuffer(&msPositions, uiNumVertices * sizeof(SFloat32Vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ||
		!pcRenderer->CreateBuffer(&msNormals, uiNumVertices * sizeof(SFloat32Vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ||
		!pcRenderer->CreateBuffer(&msColours, uiNumVertices * sizeof(uint32), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ||
		!pcRenderer->CreateBuffer(&msUVs, uiNumVertices * sizeof(SFloat32Vec2), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ||
		!pcRenderer->CreateBuffer(&msIndices, pcCooked->mauiIndices.NumElements() * sizeof(uint32), VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
	{
		return false;
	}

	memcpy(msPositions.pvMapped, pcCooked->masPositions.GetData(), uiNumVertices * sizeof(SFloat32Vec3));
	memcpy(msNormals.pvMapped, pcCooked->masNormals.GetData(), uiNumVertices * sizeof(SFloat32Vec3));
	memcpy(msIndices.pvMapped, pcCooked->mauiIndices.GetData(), pcCooked->mauiIndices.NumElements() * sizeof(uint32));
	if (pcCooked->HasUVs())
	{
		memcpy(msUVs.pvMapped, pcCooked->masUVs.GetData(), uiNumVertices * sizeof(SFloat32Vec2));
	}
	else
	{
		memset(msUVs.pvMapped, 0, uiNumVertices * sizeof(SFloat32Vec2));
	}
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
		pMaterial = pcCooked->mapMaterials.Get(i);
		psSubmesh->hTexture = pcRenderer->GetTexture(pMaterial.IsNull() ? NULL : &pMaterial->GetImage(MI_Diffuse)->mpImage);
	}
	return !pcCooked->IsSkinned() || MakeSkin(pcRenderer, pcCooked);
}


//////////////////////////////////////////////////////////////////////////
//
// Each vertex's first weight and how many, and each weight's matrix and
// amount, as SCookedSkinVertex and SCookedSkinWeight are and the shader
// reads them.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderMesh::MakeSkin(CVulkanRenderer* pcRenderer, CCookedMesh* pcCooked)
{
	size	uiVerticesSize;
	size	uiWeightsSize;

	uiVerticesSize = pcCooked->masSkinVertices.NumElements() * sizeof(SCookedSkinVertex);
	uiWeightsSize = pcCooked->masSkinWeights.NumElements() * sizeof(SCookedSkinWeight);
	if (!pcRenderer->CreateBuffer(&msSkinVertices, uiVerticesSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) ||
		!pcRenderer->CreateBuffer(&msSkinWeights, uiWeightsSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
	{
		return false;
	}
	memcpy(msSkinVertices.pvMapped, pcCooked->masSkinVertices.GetData(), uiVerticesSize);
	if (uiWeightsSize > 0)
	{
		memcpy(msSkinWeights.pvMapped, pcCooked->masSkinWeights.GetData(), uiWeightsSize);
	}
	mhSkin = pcRenderer->AllocateSkin(&msSkinVertices, &msSkinWeights);
	muiNumMatrices = pcCooked->muiNumMatrices;
	return mhSkin != VK_NULL_HANDLE;
}


//////////////////////////////////////////////////////////////////////////
//
// Its buffers, for the frame's draws.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderMesh::Bind(void)
{
	Ptr<CVulkanRenderer>	pRenderer;
	VkBuffer				ahBuffers[4];
	VkDeviceSize			auiOffsets[4];

	pRenderer = mpRenderer;
	ahBuffers[0] = msPositions.hBuffer;
	ahBuffers[1] = msNormals.hBuffer;
	ahBuffers[2] = msColours.hBuffer;
	ahBuffers[3] = msUVs.hBuffer;
	memset(auiOffsets, 0, sizeof(auiOffsets));
	pRenderer->GetFunctions()->vkCmdBindVertexBuffers(pRenderer->GetCommands(), 0, 4, ahBuffers, auiOffsets);
	pRenderer->GetFunctions()->vkCmdBindIndexBuffer(pRenderer->GetCommands(), msIndices.hBuffer, 0, VK_INDEX_TYPE_UINT32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderMesh::IsDrawable(void) { return mbMade; }
bool CVulkanRenderMesh::IsSkinned(void) { return mhSkin != VK_NULL_HANDLE; }
uint32 CVulkanRenderMesh::NumMatrices(void) { return muiNumMatrices; }
VkDescriptorSet CVulkanRenderMesh::GetSkin(void) { return mhSkin; }
size CVulkanRenderMesh::NumSubmeshes(void) { return masSubmeshes.NumElements(); }
SVulkanSubmesh* CVulkanRenderMesh::GetSubmesh(size uiIndex) { return masSubmeshes.Get(uiIndex); }

