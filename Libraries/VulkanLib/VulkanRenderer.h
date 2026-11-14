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
#include "RenderLib/Renderer.h"
#include "VulkanFunctions.h"


class CImage;


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


//A texture made from a CImage, found by its object index.
struct SVulkanTextureEntry
{
	OIndex			oiImage;
	SVulkanTexture	sTexture;
};


typedef CArrayTemplate<SVulkanTextureEntry>	CArrayVulkanTextureEntry;


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
//and a dynamic front face need, and is ready (IsReady) if it does: a program can go on without one.  A Debug build uses the
//validation layer if it's installed, and logs what it reports.
//
//A frame is BeginFrame, then the meshes' draws, then EndFrame, which waits for the GPU and gives the pixels.  So nothing is in
//flight between frames, and a mesh's buffers are destroyed as soon as it's freed.  Every buffer is in memory the CPU sees and
//the GPU reads, which is simple and fast enough for now.
//
//A skinned mesh has its skin in storage buffers (AllocateSkin), and each frame its instances' palettes are added to the frame's
//(AddPalette), which a draw finds by its offset (UseSkin).  A rigid mesh's draw uses a skin with nothing in it.
//
//GetTexture gives the texture of a CImage, made the first time it's asked for, as many meshes share an image, and kept until the
//renderer is freed.  A texture has no mipmaps yet.  An image it can't make one of, or no image, is white.  At most
//VULKAN_MAX_TEXTURES are made.
class CVulkanRenderer : public CRenderer
{
CONSTRUCTABLE(CVulkanRenderer);
DESTRUCTABLE(CVulkanRenderer);
protected:
	CVulkanFunctions					mcVulkan;
	VkInstance							mhInstance;
	VkDebugUtilsMessengerEXT			mhMessenger;
	PFN_vkDestroyDebugUtilsMessengerEXT	mpfDestroyMessenger;
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
	CArrayVulkanTextureEntry			masTextures;
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
	void				Init(void);
	void				Class(void) override;
	void				Free(void) override;

	bool				IsReady(void);
	const char*			GetDeviceName(void);
	CVulkanFunctions*	GetFunctions(void);
	VkDevice			GetDevice(void);

	bool				CreateBuffer(SVulkanBuffer* psBuffer, VkDeviceSize uiSize, VkBufferUsageFlags uiUsage);
	void				DestroyBuffer(SVulkanBuffer* psBuffer);
	VkDescriptorSet		GetTexture(CImage* pcImage);
	VkDescriptorSet		AllocateSkin(SVulkanBuffer* psVertices, SVulkanBuffer* psWeights);
	void				FreeSkin(VkDescriptorSet hSkin);
	void				ClearPalette(void);
	bool				AddPalette(float32* paf4x4s, uint32 uiNumMatrices, uint32* puiOffset);

	bool				BeginFrame(int32 iWidth, int32 iHeight, uint32 uiBackground, SVulkanFrameData* psFrame);
	VkCommandBuffer		GetCommands(void);
	void				UseOpaque(void);
	void				UseTransparent(void);
	void				UseTexture(VkDescriptorSet hTexture);
	void				UseSkin(VkDescriptorSet hSkin, uint32 uiPaletteOffset);
	void				UseNoSkin(void);
	void				Draw(SVulkanDraw* psDraw, bool bMirrored, uint32 uiFirstIndex, uint32 uiNumIndices);
	bool				EndFrame(uint32* puiPixels);

protected:
	bool				CreateInstance(void);
	bool				ChooseDevice(void);
	bool				CreateDevice(void);
	bool				CreateCommands(void);
	bool				CreateDescriptors(void);
	bool				CreateTextureDescriptors(void);
	bool				CreateSkinDescriptors(void);
	bool				CreateTexture(SVulkanTexture* psTexture, int32 iWidth, int32 iHeight, uint8* puiRGBA);
	void				DestroyTexture(SVulkanTexture* psTexture);
	bool				Upload(SVulkanImage* psImage, SVulkanBuffer* psPixels, int32 iWidth, int32 iHeight);
	bool				CreatePipelines(void);
	VkPipeline			CreatePipeline(VkShaderModule hVertex, VkShaderModule hFragment, bool bTransparent);
	VkShaderModule		CreateShader(const uint32* puiCode, size uiSize);
	bool				CreateTarget(int32 iWidth, int32 iHeight);
	void				DestroyTarget(void);
	bool				CreateImage(SVulkanImage* psImage, int32 iWidth, int32 iHeight, VkFormat eFormat, VkImageUsageFlags uiUsage, VkImageAspectFlags uiAspect);
	void				DestroyImage(SVulkanImage* psImage);
	bool				AllocateMemory(VkDeviceMemory* phMemory, VkMemoryRequirements* psRequirements, VkMemoryPropertyFlags uiProperties);
	void				Barrier(VkImage hImage, VkImageAspectFlags uiAspect, VkImageLayout eFrom, VkImageLayout eTo, VkAccessFlags uiFromAccess, VkAccessFlags uiToAccess, VkPipelineStageFlags uiFromStage, VkPipelineStageFlags uiToStage);
};


#endif // __VULKAN_RENDERER_H__

