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
#ifndef __VULKAN_RENDERER_H__
#define __VULKAN_RENDERER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "BaseLib/IndexedGeneral.h"
#include "StandardLib/ArrayWeakObject.h"
#include "RenderLib/Renderer.h"
#include "VulkanFunctions.h"


class CMaterialImage;
class CVulkanTexture;


#define VULKAN_COLOUR_FORMAT			VK_FORMAT_B8G8R8A8_UNORM  //A pixel is a uint32 0xAARRGGBB, as CRasterTarget's are.
#define VULKAN_DEPTH_FORMAT				VK_FORMAT_D32_SFLOAT
#define VULKAN_MAX_DIRECTIONAL_LIGHTS	8  //As the shaders have.
#define VULKAN_MAX_POINT_LIGHTS			64
#define VULKAN_MAX_TEXTURES				1024
#define VULKAN_MAX_SKINS				1024
#define VULKAN_PALETTE_BYTES			(4 * 1024 * 1024)  //Each frame's skinned instances' matrices, 64 bytes each.


//A buffer the CPU writes and the GPU reads, kept mapped while it lives.
struct SVulkanBuffer
{
	VkBuffer		hBuffer;
	VkDeviceMemory	hMemory;
	void*			pvMapped;
	VkDeviceSize	uiSize;
};


//An image only the GPU uses, and its view.
struct SVulkanImage
{
	VkImage			hImage;
	VkDeviceMemory	hMemory;
	VkImageView		hView;
};


//An image the shaders sample, and the descriptor set that gives it them.
struct SVulkanTexture
{
	SVulkanImage	sImage;
	VkDescriptorSet	hSet;
};


//What the shaders are given for a frame, laid out as they have it (std140).  See Shaders/Mesh.vert.
struct SVulkanFrameData
{
	float32	afViewProjection[16];
	float32	afLights[4];								//Ambient, and how many lights far away and points there are.
	float32	aafDirectional[VULKAN_MAX_DIRECTIONAL_LIGHTS][4];	//Towards each, and its amount.
	float32	aafPoint[VULKAN_MAX_POINT_LIGHTS][8];		//Where each is and its amount, where it starts to fall off, its reach and decay.
};


//What the shaders are given for each draw, as push constants.  A skinned mesh's draw has a 1 in aafNormal[0][3], and its world and
//normal matrices aren't used: its palette puts it in the world.
struct SVulkanDraw
{
	float32	afWorld[16];
	float32	aafNormal[3][4];
	float32	afColour[4];
};


//One Vulkan device and what everything it draws shares: the pipelines, the frame's uniforms, and the target it draws into,
//which is offscreen and read back when a frame ends.  Init finds Vulkan and a device that has version 1.3, which dynamic rendering
//and a dynamic front face need, and is ready (IsReady) if it does: a program can go on without one.  The instance its device is
//made on is the process's (gcVulkanInstance), which every renderer shares and none destroys.
//
//A frame is BeginFrame, then the meshes' draws, then EndFrame, which waits for the GPU and gives the pixels.  So nothing is in
//flight between frames, and a mesh's buffers are destroyed as soon as it's freed.  Every buffer is in memory the CPU sees and
//the GPU reads, which is simple and fast enough for now.
//
//A skinned mesh has its skin in storage buffers (AllocateSkin), and each frame its instances' palettes are added to the frame's
//(AddPalette), which a draw finds by its offset (UseSkin).  A rigid mesh's draw uses a skin with nothing in it.
//
//GetTexture gives the texture of a material image (CVulkanTexture), made the first time it's asked for, as many meshes share an
//image.  The meshes drawn with it hold it and the renderer only finds it (weakly), so it's freed with the last of them.  It has
//every mip level (CTextureLevels: the image's own where it has them, halved where it hasn't), sampled trilinearly.  An image it
//can't make one of, or no image, is drawn white.  At most VULKAN_MAX_TEXTURES are held at once.
class CVulkanRenderer : public CRenderer
{
CONSTRUCTABLE(CVulkanRenderer);
DESTRUCTABLE(CVulkanRenderer);
friend class CVulkanTexture;
protected:
	CVulkanFunctions					mcVulkan;			//The process's instance's functions (copied, not its library) and its own device's.
	VkInstance							mhInstance;			//gcVulkanInstance's.
	VkPhysicalDevice					mhPhysicalDevice;
	VkPhysicalDeviceMemoryProperties	msMemory;
	VkDevice							mhDevice;
	VkQueue								mhQueue;
	uint32								muiQueueFamily;
	VkCommandPool						mhCommandPool;
	VkCommandBuffer						mhCommands;
	VkFence								mhFence;
	VkDescriptorSetLayout				mhSetLayout;
	VkDescriptorPool					mhDescriptorPool;
	VkDescriptorSet						mhDescriptorSet;
	VkDescriptorSetLayout				mhTextureLayout;
	VkDescriptorPool					mhTexturePool;
	VkSampler							mhSampler;
	SVulkanTexture						msWhite;
	CArrayWeakObject					maTextures;  //CVulkanTextures, found by their images; the meshes drawn with them hold them.
	VkDescriptorSetLayout				mhSkinLayout;
	VkDescriptorPool					mhSkinPool;
	SVulkanBuffer						msPalette;
	uint32								muiPaletteUsed;
	uint32								muiPaletteAlignment;
	SVulkanBuffer						msNoSkin;
	VkDescriptorSet						mhNoSkin;
	VkPipelineLayout					mhPipelineLayout;
	VkPipeline							mhOpaquePipeline;
	VkPipeline							mhTransparentPipeline;
	SVulkanBuffer						msFrameData;
	SVulkanImage						msColour;
	SVulkanImage						msDepth;
	SVulkanBuffer						msReadback;
	int32								miWidth;
	int32								miHeight;
	CChars								mszDeviceName;
	bool								mbReady;

public:
	void					Init(void);
	void					Class(void) override;
	void					Free(void) override;

	bool					IsReady(void) const;
	const char*				GetDeviceName(void) const;
	const CVulkanFunctions*	GetFunctions(void) const;
	VkDevice				GetDevice(void) const;

	bool					CreateBuffer(SVulkanBuffer* psBuffer, VkDeviceSize uiSize, VkBufferUsageFlags uiUsage);
	void					DestroyBuffer(SVulkanBuffer* psBuffer);
	Ptr<CVulkanTexture>		GetTexture(CMaterialImage* pcImage);  //NULL for none: the mesh is drawn with GetWhite.
	VkDescriptorSet			GetWhite(void) const;
	size					NumTextures(void);  //Those some mesh still holds.
	VkDescriptorSet			AllocateSkin(SVulkanBuffer* psVertices, SVulkanBuffer* psWeights);
	void					FreeSkin(VkDescriptorSet hSkin);
	void					ClearPalette(void);
	bool					AddPalette(float32* paf4x4s, uint32 uiNumMatrices, uint32* puiOffset);

	bool					BeginFrame(int32 iWidth, int32 iHeight, uint32 uiBackground, SVulkanFrameData* psFrame);
	VkCommandBuffer			GetCommands(void) const;
	void					UseOpaque(void);
	void					UseTransparent(void);
	void					UseTexture(VkDescriptorSet hTexture);
	void					UseSkin(VkDescriptorSet hSkin, uint32 uiPaletteOffset);
	void					UseNoSkin(void);
	void					Draw(SVulkanDraw* psDraw, bool bMirrored, uint32 uiFirstIndex, uint32 uiNumIndices);
	bool					EndFrame(uint32* puiPixels);

protected:
	bool					ChooseDevice(void);
	bool					CreateDevice(void);
	bool					CreateCommands(void);
	bool					CreateDescriptors(void);
	bool					CreateTextureDescriptors(void);
	bool					CreateSkinDescriptors(void);
	bool					CreateTexture(SVulkanTexture* psTexture, int32 iNumLevels, int32* paiWidths, int32* paiHeights, uint32* puiTexels);
	void					DestroyTexture(SVulkanTexture* psTexture);
	bool					Upload(SVulkanImage* psImage, SVulkanBuffer* psPixels, int32 iNumLevels, int32* paiWidths, int32* paiHeights);
	bool					CreatePipelines(void);
	VkPipeline				CreatePipeline(VkShaderModule hVertex, VkShaderModule hFragment, bool bTransparent);
	VkShaderModule			CreateShader(const uint32* puiCode, size uiSize);
	bool					CreateTarget(int32 iWidth, int32 iHeight);
	void					DestroyTarget(void);
	bool					CreateImage(SVulkanImage* psImage, int32 iWidth, int32 iHeight, VkFormat eFormat, VkImageUsageFlags uiUsage, VkImageAspectFlags uiAspect, uint32 uiLevels = 1);
	void					DestroyImage(SVulkanImage* psImage);
	bool					AllocateMemory(VkDeviceMemory* phMemory, VkMemoryRequirements* psRequirements, VkMemoryPropertyFlags uiProperties);
	void					Barrier(VkImage hImage, VkImageAspectFlags uiAspect, VkImageLayout eFrom, VkImageLayout eTo, VkAccessFlags uiFromAccess, VkAccessFlags uiToAccess, VkPipelineStageFlags uiFromStage, VkPipelineStageFlags uiToStage, uint32 uiLevels = 1);
};


#endif // __VULKAN_RENDERER_H__

