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
#ifndef __VULKAN_FRAME_H__
#define __VULKAN_FRAME_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Float4x4.h"
#include "BaseLib/IndexedGeneral.h"
#include "StandardLib/Array.h"
#include "SupportLib/Camera.h"
#include "RenderLib/RenderScene.h"
#include "VulkanRenderMesh.h"


//A mesh somewhere: what its draws push, and whether it's mirrored.  A skinned one's skin, and where its palette is in the frame's.
//The frame holds the mesh.
struct SVulkanFrameInstance
{
	CVulkanRenderMesh*	pcMesh;
	SVulkanDraw			sDraw;
	VkDescriptorSet		hSkin;
	uint32				uiPaletteOffset;
	bool				bMirrored;
};


typedef CArrayTemplate<SVulkanFrameInstance>	CArrayVulkanFrameInstance;
typedef CArrayTemplatePrimitive<OIndex>			CArrayVulkanOIndex;


//A frame of a scene as the Vulkan renderer draws it, made by Extract from a CRenderScene whose renderer is a CVulkanRenderer,
//after Extract updates the render scene, and drawn offscreen by Draw into pixels (0xAARRGGBB, a row of the width after another).
//
//Each rigid mesh of each instance is placed by its connection's world matrix, and its normals are turned by that matrix's inverse
//transpose.  A skinned mesh is posed in the vertex shader by its palette: each matrix its inverse matrix times the world matrix of
//the instance mesh's connection for it.  It isn't drawn if a connection is missing, which binding it logged, or if a matrix
//mirrors, which is an error logged the first time this frame finds it for that instance mesh.
//
//The camera is the scene camera, looking along its connection's -z with y up, its field of view across the target's width, as
//CRasterFrame's is.  With no camera, or a camera that isn't placed, nothing is drawn.
//
//The lights are the scene's, as CRasterFrame has them, in the scene's units: ambient lights add up, a directional light is
//towards its connection's +z, and omni and spot lights are point lights, a spot's cone not being used yet.  A light's amount is
//its strength of the most a shade can be.  A scene with no lights has the light the rasteriser has always had.
class CVulkanFrame : public CObject
{
CONSTRUCTABLE(CVulkanFrame);
DESTRUCTABLE(CVulkanFrame);
protected:
	Ptr<CRenderScene>			mpRenderScene;
	Ptr<CCamera>				mpCamera;
	CArrayVulkanFrameInstance	masInstances;
	CArray<CVulkanRenderMesh>	mapMeshes;
	SVulkanFrameData			msData;
	CArrayFloat4x4				masPalette;			//The skinned mesh being posed's matrices.
	CArrayVulkanOIndex			maoiMirrorReported;	//The instance meshes a bone mirroring has been logged for.

public:
	void	Init(Ptr<CRenderScene> pRenderScene, Ptr<CCamera> pCamera);
	void	Class(void) override;
	void	Free(void) override;

	bool	Extract(int32 iWidth, int32 iHeight);
	bool	Draw(uint32* puiPixels, int32 iWidth, int32 iHeight, uint32 uiBackground);

	size	NumInstances(void);

protected:
	void	ExtractInstances(void);
	bool	ExtractSkinned(CInstance* pcInstance, CInstanceMesh* pcInstanceMesh, CVulkanRenderMesh* pcMesh, SVulkanFrameInstance* psInstance);
	bool	IsMirrorReported(OIndex oiInstanceMesh);
	bool	ExtractCamera(int32 iWidth, int32 iHeight);
	void	ExtractLights(void);
	void	DrawSubmeshes(CVulkanRenderer* pcRenderer, bool bTransparent);
};


#endif // __VULKAN_FRAME_H__

