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
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Logger.h"
#include "StandardLib/ClassDefines.h"
#include "SupportLib/Image.h"
#include "VulkanShaders.h"
#include "VulkanRenderer.h"


#define VULKAN_VALIDATION_LAYER		"VK_LAYER_KHRONOS_validation"
#define VULKAN_MAX_PHYSICAL_DEVICES	16
#define VULKAN_MAX_QUEUE_FAMILIES	16
#define VULKAN_MAX_LAYERS			64
#define VULKAN_TEXTURE_FORMAT		VK_FORMAT_R8G8B8A8_UNORM  //A texel is bytes of red, green, blue and alpha.


//////////////////////////////////////////////////////////////////////////
//
// What the validation layer reports.  An error is logged as one, which a
// Debug build stops on.
//
//////////////////////////////////////////////////////////////////////////
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanValidationMessage(VkDebugUtilsMessageSeverityFlagBitsEXT eSeverity, VkDebugUtilsMessageTypeFlagsEXT uiType, const VkDebugUtilsMessengerCallbackDataEXT* psData, void* pvUser)
{
	if (eSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		gcLogger.Error2(__METHOD__, " ", psData->pMessage, NULL);
	}
	else if (eSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		gcLogger.Warning2(__METHOD__, " ", psData->pMessage, NULL);
	}
	return VK_FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ClearVulkanBuffer(SVulkanBuffer* psBuffer)
{
	memset(psBuffer, 0, sizeof(SVulkanBuffer));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ClearVulkanImage(SVulkanImage* psImage)
{
	memset(psImage, 0, sizeof(SVulkanImage));
}


//////////////////////////////////////////////////////////////////////////
//
// Makes everything it can, and is ready if it all was.  Whatever it made is
// destroyed when it's freed, as Free looks at what isn't null.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::Init(void)
{
	PreInit();

	CRenderer::Init();
	mcVulkan.Init();
	mhInstance = VK_NULL_HANDLE;
	mhMessenger = VK_NULL_HANDLE;
	mpfDestroyMessenger = NULL;
	mhPhysicalDevice = VK_NULL_HANDLE;
	memset(&msMemory, 0, sizeof(msMemory));
	mhDevice = VK_NULL_HANDLE;
	mhQueue = VK_NULL_HANDLE;
	muiQueueFamily = 0;
	mhCommandPool = VK_NULL_HANDLE;
	mhCommands = VK_NULL_HANDLE;
	mhFence = VK_NULL_HANDLE;
	mhSetLayout = VK_NULL_HANDLE;
	mhDescriptorPool = VK_NULL_HANDLE;
	mhDescriptorSet = VK_NULL_HANDLE;
	mhTextureLayout = VK_NULL_HANDLE;
	mhTexturePool = VK_NULL_HANDLE;
	mhSampler = VK_NULL_HANDLE;
	memset(&msWhite, 0, sizeof(msWhite));
	masTextures.Init();
	mhPipelineLayout = VK_NULL_HANDLE;
	mhOpaquePipeline = VK_NULL_HANDLE;
	mhTransparentPipeline = VK_NULL_HANDLE;
	ClearVulkanBuffer(&msFrameData);
	ClearVulkanImage(&msColour);
	ClearVulkanImage(&msDepth);
	ClearVulkanBuffer(&msReadback);
	miWidth = 0;
	miHeight = 0;
	mszDeviceName.Init();

	mbReady = mcVulkan.Load() && CreateInstance() && ChooseDevice() && CreateDevice() && CreateCommands() && CreateDescriptors() && CreateTextureDescriptors() && CreatePipelines();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::Class(void)
{
	CRenderer::Class();
	U_Data(CVulkanFunctions, mcVulkan);
	U_Pointer(mhInstance);
	U_Pointer(mhPhysicalDevice);
	U_Pointer(mhDevice);
	U_String(mszDeviceName);
	U_Bool(mbReady);
}


//////////////////////////////////////////////////////////////////////////
//
// Waits for the device, as a frame may never have ended.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::Free(void)
{
	size	i;

	if (mhDevice)
	{
		mcVulkan.vkDeviceWaitIdle(mhDevice);
		DestroyTarget();
		DestroyBuffer(&msFrameData);
		for (i = 0; i < masTextures.NumElements(); i++)
		{
			DestroyTexture(&masTextures.Get(i)->sTexture);
		}
		DestroyTexture(&msWhite);
		if (mhSampler)				mcVulkan.vkDestroySampler(mhDevice, mhSampler, NULL);
		if (mhTexturePool)			mcVulkan.vkDestroyDescriptorPool(mhDevice, mhTexturePool, NULL);
		if (mhTextureLayout)		mcVulkan.vkDestroyDescriptorSetLayout(mhDevice, mhTextureLayout, NULL);
		if (mhOpaquePipeline)		mcVulkan.vkDestroyPipeline(mhDevice, mhOpaquePipeline, NULL);
		if (mhTransparentPipeline)	mcVulkan.vkDestroyPipeline(mhDevice, mhTransparentPipeline, NULL);
		if (mhPipelineLayout)		mcVulkan.vkDestroyPipelineLayout(mhDevice, mhPipelineLayout, NULL);
		if (mhDescriptorPool)		mcVulkan.vkDestroyDescriptorPool(mhDevice, mhDescriptorPool, NULL);
		if (mhSetLayout)			mcVulkan.vkDestroyDescriptorSetLayout(mhDevice, mhSetLayout, NULL);
		if (mhFence)				mcVulkan.vkDestroyFence(mhDevice, mhFence, NULL);
		if (mhCommandPool)			mcVulkan.vkDestroyCommandPool(mhDevice, mhCommandPool, NULL);
		mcVulkan.vkDestroyDevice(mhDevice, NULL);
	}
	if (mhInstance)
	{
		if (mhMessenger && mpfDestroyMessenger)
		{
			mpfDestroyMessenger(mhInstance, mhMessenger, NULL);
		}
		mcVulkan.vkDestroyInstance(mhInstance, NULL);
	}
	masTextures.Kill();
	mszDeviceName.Kill();
	mcVulkan.Kill();
	CRenderer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the validation layer is installed.
//
//////////////////////////////////////////////////////////////////////////
static bool HasVulkanValidation(CVulkanFunctions* pcVulkan)
{
	VkLayerProperties	asLayers[VULKAN_MAX_LAYERS];
	uint32				uiNumLayers;
	uint32				i;

	uiNumLayers = VULKAN_MAX_LAYERS;
	if (pcVulkan->vkEnumerateInstanceLayerProperties(&uiNumLayers, asLayers) < 0)
	{
		return false;
	}
	for (i = 0; i < uiNumLayers; i++)
	{
		if (strcmp(asLayers[i].layerName, VULKAN_VALIDATION_LAYER) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Version 1.3, with no surfaces as it draws offscreen.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateInstance(void)
{
	VkApplicationInfo					sApplication;
	VkInstanceCreateInfo				sCreate;
	VkDebugUtilsMessengerCreateInfoEXT	sMessenger;
	PFN_vkCreateDebugUtilsMessengerEXT	pfCreateMessenger;
	const char*							szLayer;
	const char*							szExtension;
	uint32								uiVersion;
	bool								bValidate;

	uiVersion = 0;
	if ((mcVulkan.vkEnumerateInstanceVersion(&uiVersion) < 0) || (uiVersion < VK_API_VERSION_1_3))
	{
		return false;
	}

#ifdef DEBUG
	bValidate = HasVulkanValidation(&mcVulkan);
#else
	bValidate = false;
#endif
	szLayer = VULKAN_VALIDATION_LAYER;
	szExtension = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

	memset(&sApplication, 0, sizeof(sApplication));
	sApplication.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	sApplication.pApplicationName = "Codaphela";
	sApplication.pEngineName = "Codaphela";
	sApplication.apiVersion = VK_API_VERSION_1_3;

	memset(&sCreate, 0, sizeof(sCreate));
	sCreate.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	sCreate.pApplicationInfo = &sApplication;
	if (bValidate)
	{
		sCreate.enabledLayerCount = 1;
		sCreate.ppEnabledLayerNames = &szLayer;
		sCreate.enabledExtensionCount = 1;
		sCreate.ppEnabledExtensionNames = &szExtension;
	}
	if ((mcVulkan.vkCreateInstance(&sCreate, NULL, &mhInstance) < 0) || !mcVulkan.LoadInstance(mhInstance))
	{
		return false;
	}

	if (bValidate)
	{
		pfCreateMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)mcVulkan.GetInstanceFunction(mhInstance, "vkCreateDebugUtilsMessengerEXT");
		mpfDestroyMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)mcVulkan.GetInstanceFunction(mhInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (pfCreateMessenger)
		{
			memset(&sMessenger, 0, sizeof(sMessenger));
			sMessenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			sMessenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			sMessenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			sMessenger.pfnUserCallback = VulkanValidationMessage;
			pfCreateMessenger(mhInstance, &sMessenger, NULL, &mhMessenger);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A discrete GPU before any other, with version 1.3 and a queue that draws.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::ChooseDevice(void)
{
	VkPhysicalDevice			ahDevices[VULKAN_MAX_PHYSICAL_DEVICES];
	VkQueueFamilyProperties		asFamilies[VULKAN_MAX_QUEUE_FAMILIES];
	VkPhysicalDeviceProperties	sProperties;
	uint32						uiNumDevices;
	uint32						uiNumFamilies;
	uint32						i;
	uint32						j;
	int							iScore;
	int							iBestScore;

	uiNumDevices = VULKAN_MAX_PHYSICAL_DEVICES;
	if (mcVulkan.vkEnumeratePhysicalDevices(mhInstance, &uiNumDevices, ahDevices) < 0)
	{
		return false;
	}

	iBestScore = 0;
	for (i = 0; i < uiNumDevices; i++)
	{
		mcVulkan.vkGetPhysicalDeviceProperties(ahDevices[i], &sProperties);
		if (sProperties.apiVersion < VK_API_VERSION_1_3)
		{
			continue;
		}
		uiNumFamilies = VULKAN_MAX_QUEUE_FAMILIES;
		mcVulkan.vkGetPhysicalDeviceQueueFamilyProperties(ahDevices[i], &uiNumFamilies, asFamilies);
		for (j = 0; j < uiNumFamilies; j++)
		{
			if (asFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				break;
			}
		}
		if (j == uiNumFamilies)
		{
			continue;
		}
		iScore = (sProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ? 3 : ((sProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) ? 2 : 1);
		if (iScore > iBestScore)
		{
			iBestScore = iScore;
			mhPhysicalDevice = ahDevices[i];
			muiQueueFamily = j;
			mszDeviceName.ReInit();
			mszDeviceName.Append(sProperties.deviceName);
		}
	}
	if (mhPhysicalDevice == VK_NULL_HANDLE)
	{
		return false;
	}
	mcVulkan.vkGetPhysicalDeviceMemoryProperties(mhPhysicalDevice, &msMemory);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// With dynamic rendering, which is in 1.3.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateDevice(void)
{
	VkDeviceQueueCreateInfo				sQueue;
	VkDeviceCreateInfo					sCreate;
	VkPhysicalDeviceVulkan13Features	sFeatures13;
	float32								fPriority;

	fPriority = 1.0f;
	memset(&sQueue, 0, sizeof(sQueue));
	sQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	sQueue.queueFamilyIndex = muiQueueFamily;
	sQueue.queueCount = 1;
	sQueue.pQueuePriorities = &fPriority;

	memset(&sFeatures13, 0, sizeof(sFeatures13));
	sFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	sFeatures13.dynamicRendering = VK_TRUE;

	memset(&sCreate, 0, sizeof(sCreate));
	sCreate.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	sCreate.pNext = &sFeatures13;
	sCreate.queueCreateInfoCount = 1;
	sCreate.pQueueCreateInfos = &sQueue;
	if ((mcVulkan.vkCreateDevice(mhPhysicalDevice, &sCreate, NULL, &mhDevice) < 0) || !mcVulkan.LoadDevice(mhDevice))
	{
		return false;
	}
	mcVulkan.vkGetDeviceQueue(mhDevice, muiQueueFamily, 0, &mhQueue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// One command buffer, used again each frame, and the fence a frame's end
// waits for.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateCommands(void)
{
	VkCommandPoolCreateInfo		sPool;
	VkCommandBufferAllocateInfo	sAllocate;
	VkFenceCreateInfo			sFence;

	memset(&sPool, 0, sizeof(sPool));
	sPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	sPool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	sPool.queueFamilyIndex = muiQueueFamily;
	if (mcVulkan.vkCreateCommandPool(mhDevice, &sPool, NULL, &mhCommandPool) < 0)
	{
		return false;
	}

	memset(&sAllocate, 0, sizeof(sAllocate));
	sAllocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	sAllocate.commandPool = mhCommandPool;
	sAllocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	sAllocate.commandBufferCount = 1;
	if (mcVulkan.vkAllocateCommandBuffers(mhDevice, &sAllocate, &mhCommands) < 0)
	{
		return false;
	}

	memset(&sFence, 0, sizeof(sFence));
	sFence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	return mcVulkan.vkCreateFence(mhDevice, &sFence, NULL, &mhFence) >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The frame's uniforms, one set of them as frames don't overlap.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateDescriptors(void)
{
	VkDescriptorSetLayoutBinding	sBinding;
	VkDescriptorSetLayoutCreateInfo	sLayout;
	VkDescriptorPoolSize			sPoolSize;
	VkDescriptorPoolCreateInfo		sPool;
	VkDescriptorSetAllocateInfo		sAllocate;
	VkDescriptorBufferInfo			sBufferInfo;
	VkWriteDescriptorSet			sWrite;

	memset(&sBinding, 0, sizeof(sBinding));
	sBinding.binding = 0;
	sBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sBinding.descriptorCount = 1;
	sBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	memset(&sLayout, 0, sizeof(sLayout));
	sLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	sLayout.bindingCount = 1;
	sLayout.pBindings = &sBinding;
	if (mcVulkan.vkCreateDescriptorSetLayout(mhDevice, &sLayout, NULL, &mhSetLayout) < 0)
	{
		return false;
	}

	sPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sPoolSize.descriptorCount = 1;
	memset(&sPool, 0, sizeof(sPool));
	sPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	sPool.maxSets = 1;
	sPool.poolSizeCount = 1;
	sPool.pPoolSizes = &sPoolSize;
	if (mcVulkan.vkCreateDescriptorPool(mhDevice, &sPool, NULL, &mhDescriptorPool) < 0)
	{
		return false;
	}

	memset(&sAllocate, 0, sizeof(sAllocate));
	sAllocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	sAllocate.descriptorPool = mhDescriptorPool;
	sAllocate.descriptorSetCount = 1;
	sAllocate.pSetLayouts = &mhSetLayout;
	if ((mcVulkan.vkAllocateDescriptorSets(mhDevice, &sAllocate, &mhDescriptorSet) < 0) || !CreateBuffer(&msFrameData, sizeof(SVulkanFrameData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
	{
		return false;
	}

	sBufferInfo.buffer = msFrameData.hBuffer;
	sBufferInfo.offset = 0;
	sBufferInfo.range = sizeof(SVulkanFrameData);
	memset(&sWrite, 0, sizeof(sWrite));
	sWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	sWrite.dstSet = mhDescriptorSet;
	sWrite.dstBinding = 0;
	sWrite.descriptorCount = 1;
	sWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sWrite.pBufferInfo = &sBufferInfo;
	mcVulkan.vkUpdateDescriptorSets(mhDevice, 1, &sWrite, 0, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A set for each texture, of the one image the fragment shader samples,
// with a sampler they all share: smooth between texels, and repeating.
// Then the white texture.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateTextureDescriptors(void)
{
	VkDescriptorSetLayoutBinding	sBinding;
	VkDescriptorSetLayoutCreateInfo	sLayout;
	VkDescriptorPoolSize			sPoolSize;
	VkDescriptorPoolCreateInfo		sPool;
	VkSamplerCreateInfo				sSampler;
	uint8							auiWhite[4];

	memset(&sBinding, 0, sizeof(sBinding));
	sBinding.binding = 0;
	sBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sBinding.descriptorCount = 1;
	sBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	memset(&sLayout, 0, sizeof(sLayout));
	sLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	sLayout.bindingCount = 1;
	sLayout.pBindings = &sBinding;
	if (mcVulkan.vkCreateDescriptorSetLayout(mhDevice, &sLayout, NULL, &mhTextureLayout) < 0)
	{
		return false;
	}

	sPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sPoolSize.descriptorCount = VULKAN_MAX_TEXTURES + 1;
	memset(&sPool, 0, sizeof(sPool));
	sPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	sPool.maxSets = VULKAN_MAX_TEXTURES + 1;
	sPool.poolSizeCount = 1;
	sPool.pPoolSizes = &sPoolSize;
	if (mcVulkan.vkCreateDescriptorPool(mhDevice, &sPool, NULL, &mhTexturePool) < 0)
	{
		return false;
	}

	memset(&sSampler, 0, sizeof(sSampler));
	sSampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sSampler.magFilter = VK_FILTER_LINEAR;
	sSampler.minFilter = VK_FILTER_LINEAR;
	sSampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sSampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	if (mcVulkan.vkCreateSampler(mhDevice, &sSampler, NULL, &mhSampler) < 0)
	{
		return false;
	}

	memset(auiWhite, 0xFF, sizeof(auiWhite));
	return CreateTexture(&msWhite, 1, 1, auiWhite);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the pixels in a buffer into an image the shaders sample, and waits
// for it.  Not while a frame is being drawn, as it uses the frame's
// commands.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::Upload(SVulkanImage* psImage, SVulkanBuffer* psPixels, int32 iWidth, int32 iHeight)
{
	VkCommandBufferBeginInfo	sBegin;
	VkBufferImageCopy			sCopy;
	VkSubmitInfo				sSubmit;
	bool						bResult;

	mcVulkan.vkResetCommandBuffer(mhCommands, 0);
	memset(&sBegin, 0, sizeof(sBegin));
	sBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	sBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	mcVulkan.vkBeginCommandBuffer(mhCommands, &sBegin);

	Barrier(psImage->hImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	memset(&sCopy, 0, sizeof(sCopy));
	sCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sCopy.imageSubresource.layerCount = 1;
	sCopy.imageExtent.width = (uint32)iWidth;
	sCopy.imageExtent.height = (uint32)iHeight;
	sCopy.imageExtent.depth = 1;
	mcVulkan.vkCmdCopyBufferToImage(mhCommands, psPixels->hBuffer, psImage->hImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &sCopy);
	Barrier(psImage->hImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	mcVulkan.vkEndCommandBuffer(mhCommands);

	memset(&sSubmit, 0, sizeof(sSubmit));
	sSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sSubmit.commandBufferCount = 1;
	sSubmit.pCommandBuffers = &mhCommands;
	bResult = (mcVulkan.vkQueueSubmit(mhQueue, 1, &sSubmit, mhFence) >= 0) && (mcVulkan.vkWaitForFences(mhDevice, 1, &mhFence, VK_TRUE, UINT64_MAX) >= 0);
	mcVulkan.vkResetFences(mhDevice, 1, &mhFence);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// From puiRGBA, bytes of red, green, blue and alpha with the top row first.
// False, having made nothing, if it can't be made.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateTexture(SVulkanTexture* psTexture, int32 iWidth, int32 iHeight, uint8* puiRGBA)
{
	SVulkanBuffer				sPixels;
	VkDescriptorSetAllocateInfo	sAllocate;
	VkDescriptorImageInfo		sImageInfo;
	VkWriteDescriptorSet		sWrite;
	bool						bResult;

	memset(psTexture, 0, sizeof(SVulkanTexture));
	if (!CreateImage(&psTexture->sImage, iWidth, iHeight, VULKAN_TEXTURE_FORMAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT))
	{
		return false;
	}
	if (!CreateBuffer(&sPixels, (VkDeviceSize)iWidth * iHeight * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
	{
		DestroyTexture(psTexture);
		return false;
	}
	memcpy(sPixels.pvMapped, puiRGBA, (size)iWidth * iHeight * 4);
	bResult = Upload(&psTexture->sImage, &sPixels, iWidth, iHeight);
	DestroyBuffer(&sPixels);

	memset(&sAllocate, 0, sizeof(sAllocate));
	sAllocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	sAllocate.descriptorPool = mhTexturePool;
	sAllocate.descriptorSetCount = 1;
	sAllocate.pSetLayouts = &mhTextureLayout;
	if (!bResult || (mcVulkan.vkAllocateDescriptorSets(mhDevice, &sAllocate, &psTexture->hSet) < 0))
	{
		DestroyTexture(psTexture);
		return false;
	}

	sImageInfo.sampler = mhSampler;
	sImageInfo.imageView = psTexture->sImage.hView;
	sImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	memset(&sWrite, 0, sizeof(sWrite));
	sWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	sWrite.dstSet = psTexture->hSet;
	sWrite.dstBinding = 0;
	sWrite.descriptorCount = 1;
	sWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sWrite.pImageInfo = &sImageInfo;
	mcVulkan.vkUpdateDescriptorSets(mhDevice, 1, &sWrite, 0, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Its set goes back with the pool.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::DestroyTexture(SVulkanTexture* psTexture)
{
	DestroyImage(&psTexture->sImage);
	psTexture->hSet = VK_NULL_HANDLE;
}


//////////////////////////////////////////////////////////////////////////
//
// An image's texels as bytes of red, green, blue and alpha.  A colour it
// hasn't a channel for is all there, as is alpha.  False for an image that
// isn't bytes.
//
//////////////////////////////////////////////////////////////////////////
static bool GetVulkanTexels(CImage* pcImage, uint8* puiRGBA)
{
	CChannel*	pcChannel;
	uint8*		puiImage;
	size		uiNumChannels;
	size		uiNumPixels;
	size		i;
	size		j;
	int			aiPart[16];

	if ((pcImage->GetPrimitiveType() != PT_uint8) || (pcImage->NumChannels() == 0) || (pcImage->NumChannels() > 16))
	{
		return false;
	}
	uiNumChannels = pcImage->NumChannels();
	for (j = 0; j < uiNumChannels; j++)
	{
		pcChannel = pcImage->GetChannelAtIndex(j);
		aiPart[j] = (pcChannel->iChannel == IMAGE_DIFFUSE_RED) ? 0 : ((pcChannel->iChannel == IMAGE_DIFFUSE_GREEN) ? 1 : ((pcChannel->iChannel == IMAGE_DIFFUSE_BLUE) ? 2 : ((pcChannel->iChannel == IMAGE_OPACITY) ? 3 : -1)));
	}

	uiNumPixels = (size)pcImage->GetWidth() * pcImage->GetHeight();
	memset(puiRGBA, 0xFF, uiNumPixels * 4);
	puiImage = (uint8*)pcImage->GetData();
	for (i = 0; i < uiNumPixels; i++)
	{
		for (j = 0; j < uiNumChannels; j++)
		{
			if (aiPart[j] >= 0)
			{
				puiRGBA[i * 4 + aiPart[j]] = puiImage[i * uiNumChannels + j];
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// White for no image, one that isn't bytes, or one that can't be made.
//
//////////////////////////////////////////////////////////////////////////
VkDescriptorSet CVulkanRenderer::GetTexture(CImage* pcImage)
{
	SVulkanTextureEntry*	psEntry;
	SVulkanTexture			sTexture;
	uint8*					puiRGBA;
	OIndex					oiImage;
	size					i;
	bool					bResult;

	if ((pcImage == NULL) || (pcImage->GetWidth() == 0) || (pcImage->GetHeight() == 0))
	{
		return msWhite.hSet;
	}
	oiImage = pcImage->GetIndex();
	for (i = 0; i < masTextures.NumElements(); i++)
	{
		psEntry = masTextures.Get(i);
		if (psEntry->oiImage == oiImage)
		{
			return psEntry->sTexture.hSet;
		}
	}
	if (masTextures.NumElements() >= VULKAN_MAX_TEXTURES)
	{
		return msWhite.hSet;
	}

	puiRGBA = (uint8*)malloc((size)pcImage->GetWidth() * pcImage->GetHeight() * 4);
	bResult = (puiRGBA != NULL) && GetVulkanTexels(pcImage, puiRGBA) && CreateTexture(&sTexture, (int32)pcImage->GetWidth(), (int32)pcImage->GetHeight(), puiRGBA);
	free(puiRGBA);
	if (!bResult)
	{
		return msWhite.hSet;
	}
	psEntry = masTextures.Add();
	psEntry->oiImage = oiImage;
	psEntry->sTexture = sTexture;
	return sTexture.hSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
VkShaderModule CVulkanRenderer::CreateShader(const uint32* puiCode, size uiSize)
{
	VkShaderModuleCreateInfo	sCreate;
	VkShaderModule				hModule;

	memset(&sCreate, 0, sizeof(sCreate));
	sCreate.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	sCreate.codeSize = uiSize;
	sCreate.pCode = puiCode;
	hModule = VK_NULL_HANDLE;
	mcVulkan.vkCreateShaderModule(mhDevice, &sCreate, NULL, &hModule);
	return hModule;
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh's vertices are a position, a normal, a colour and a UV, each a
// stream of its own.  Back faces are culled, and which way a front face is wound is
// set for each draw, as a mirrored mesh's are wound the other way.  A
// transparent triangle tests depth and doesn't write it, and blends.
// Alpha isn't written: the target's stays as it was cleared, opaque.
//
//////////////////////////////////////////////////////////////////////////
VkPipeline CVulkanRenderer::CreatePipeline(VkShaderModule hVertex, VkShaderModule hFragment, bool bTransparent)
{
	VkPipelineShaderStageCreateInfo				asStages[2];
	VkVertexInputBindingDescription				asBindings[4];
	VkVertexInputAttributeDescription			asAttributes[4];
	VkPipelineVertexInputStateCreateInfo		sVertexInput;
	VkPipelineInputAssemblyStateCreateInfo		sAssembly;
	VkPipelineViewportStateCreateInfo			sViewport;
	VkPipelineRasterizationStateCreateInfo		sRasterisation;
	VkPipelineMultisampleStateCreateInfo		sMultisample;
	VkPipelineDepthStencilStateCreateInfo		sDepth;
	VkPipelineColorBlendAttachmentState			sBlendAttachment;
	VkPipelineColorBlendStateCreateInfo			sBlend;
	VkDynamicState								aeDynamic[3];
	VkPipelineDynamicStateCreateInfo			sDynamic;
	VkPipelineRenderingCreateInfo				sRendering;
	VkGraphicsPipelineCreateInfo				sCreate;
	VkFormat									eColourFormat;
	VkPipeline									hPipeline;
	uint32										i;

	memset(asStages, 0, sizeof(asStages));
	asStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	asStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	asStages[0].module = hVertex;
	asStages[0].pName = "main";
	asStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	asStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	asStages[1].module = hFragment;
	asStages[1].pName = "main";

	for (i = 0; i < 4; i++)
	{
		asBindings[i].binding = i;
		asBindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		asAttributes[i].location = i;
		asAttributes[i].binding = i;
		asAttributes[i].offset = 0;
	}
	asBindings[0].stride = sizeof(float32) * 3;
	asAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	asBindings[1].stride = sizeof(float32) * 3;
	asAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	asBindings[2].stride = sizeof(uint32);
	asAttributes[2].format = VULKAN_COLOUR_FORMAT;
	asBindings[3].stride = sizeof(float32) * 2;
	asAttributes[3].format = VK_FORMAT_R32G32_SFLOAT;
	memset(&sVertexInput, 0, sizeof(sVertexInput));
	sVertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	sVertexInput.vertexBindingDescriptionCount = 4;
	sVertexInput.pVertexBindingDescriptions = asBindings;
	sVertexInput.vertexAttributeDescriptionCount = 4;
	sVertexInput.pVertexAttributeDescriptions = asAttributes;

	memset(&sAssembly, 0, sizeof(sAssembly));
	sAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	sAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	memset(&sViewport, 0, sizeof(sViewport));
	sViewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	sViewport.viewportCount = 1;
	sViewport.scissorCount = 1;

	memset(&sRasterisation, 0, sizeof(sRasterisation));
	sRasterisation.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	sRasterisation.polygonMode = VK_POLYGON_MODE_FILL;
	sRasterisation.cullMode = VK_CULL_MODE_BACK_BIT;
	sRasterisation.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	sRasterisation.lineWidth = 1.0f;

	memset(&sMultisample, 0, sizeof(sMultisample));
	sMultisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	sMultisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	memset(&sDepth, 0, sizeof(sDepth));
	sDepth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	sDepth.depthTestEnable = VK_TRUE;
	sDepth.depthWriteEnable = bTransparent ? VK_FALSE : VK_TRUE;
	sDepth.depthCompareOp = VK_COMPARE_OP_LESS;

	memset(&sBlendAttachment, 0, sizeof(sBlendAttachment));
	sBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
	if (bTransparent)
	{
		sBlendAttachment.blendEnable = VK_TRUE;
		sBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		sBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		sBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		sBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		sBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		sBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	memset(&sBlend, 0, sizeof(sBlend));
	sBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	sBlend.attachmentCount = 1;
	sBlend.pAttachments = &sBlendAttachment;

	aeDynamic[0] = VK_DYNAMIC_STATE_VIEWPORT;
	aeDynamic[1] = VK_DYNAMIC_STATE_SCISSOR;
	aeDynamic[2] = VK_DYNAMIC_STATE_FRONT_FACE;
	memset(&sDynamic, 0, sizeof(sDynamic));
	sDynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	sDynamic.dynamicStateCount = 3;
	sDynamic.pDynamicStates = aeDynamic;

	eColourFormat = VULKAN_COLOUR_FORMAT;
	memset(&sRendering, 0, sizeof(sRendering));
	sRendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	sRendering.colorAttachmentCount = 1;
	sRendering.pColorAttachmentFormats = &eColourFormat;
	sRendering.depthAttachmentFormat = VULKAN_DEPTH_FORMAT;

	memset(&sCreate, 0, sizeof(sCreate));
	sCreate.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	sCreate.pNext = &sRendering;
	sCreate.stageCount = 2;
	sCreate.pStages = asStages;
	sCreate.pVertexInputState = &sVertexInput;
	sCreate.pInputAssemblyState = &sAssembly;
	sCreate.pViewportState = &sViewport;
	sCreate.pRasterizationState = &sRasterisation;
	sCreate.pMultisampleState = &sMultisample;
	sCreate.pDepthStencilState = &sDepth;
	sCreate.pColorBlendState = &sBlend;
	sCreate.pDynamicState = &sDynamic;
	sCreate.layout = mhPipelineLayout;

	hPipeline = VK_NULL_HANDLE;
	mcVulkan.vkCreateGraphicsPipelines(mhDevice, VK_NULL_HANDLE, 1, &sCreate, NULL, &hPipeline);
	return hPipeline;
}


//////////////////////////////////////////////////////////////////////////
//
// One pipeline for opaque triangles and one for transparent, sharing the
// shaders, which are only needed while they're made.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreatePipelines(void)
{
	VkPushConstantRange			sPushRange;
	VkPipelineLayoutCreateInfo	sLayout;
	VkDescriptorSetLayout		ahSetLayouts[2];
	VkShaderModule				hVertex;
	VkShaderModule				hFragment;

	sPushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	sPushRange.offset = 0;
	sPushRange.size = sizeof(SVulkanDraw);
	memset(&sLayout, 0, sizeof(sLayout));
	sLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ahSetLayouts[0] = mhSetLayout;
	ahSetLayouts[1] = mhTextureLayout;
	sLayout.setLayoutCount = 2;
	sLayout.pSetLayouts = ahSetLayouts;
	sLayout.pushConstantRangeCount = 1;
	sLayout.pPushConstantRanges = &sPushRange;
	if (mcVulkan.vkCreatePipelineLayout(mhDevice, &sLayout, NULL, &mhPipelineLayout) < 0)
	{
		return false;
	}

	hVertex = CreateShader(gauiVulkanMeshVertexShader, sizeof(gauiVulkanMeshVertexShader));
	hFragment = CreateShader(gauiVulkanMeshFragmentShader, sizeof(gauiVulkanMeshFragmentShader));
	if (hVertex && hFragment)
	{
		mhOpaquePipeline = CreatePipeline(hVertex, hFragment, false);
		mhTransparentPipeline = CreatePipeline(hVertex, hFragment, true);
	}
	if (hVertex)
	{
		mcVulkan.vkDestroyShaderModule(mhDevice, hVertex, NULL);
	}
	if (hFragment)
	{
		mcVulkan.vkDestroyShaderModule(mhDevice, hFragment, NULL);
	}
	return (mhOpaquePipeline != VK_NULL_HANDLE) && (mhTransparentPipeline != VK_NULL_HANDLE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::AllocateMemory(VkDeviceMemory* phMemory, VkMemoryRequirements* psRequirements, VkMemoryPropertyFlags uiProperties)
{
	VkMemoryAllocateInfo	sAllocate;
	uint32					i;

	for (i = 0; i < msMemory.memoryTypeCount; i++)
	{
		if ((psRequirements->memoryTypeBits & (1u << i)) && ((msMemory.memoryTypes[i].propertyFlags & uiProperties) == uiProperties))
		{
			memset(&sAllocate, 0, sizeof(sAllocate));
			sAllocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			sAllocate.allocationSize = psRequirements->size;
			sAllocate.memoryTypeIndex = i;
			return mcVulkan.vkAllocateMemory(mhDevice, &sAllocate, NULL, phMemory) >= 0;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// In memory the CPU sees, mapped.  False, having made nothing, if it can't
// be made.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateBuffer(SVulkanBuffer* psBuffer, VkDeviceSize uiSize, VkBufferUsageFlags uiUsage)
{
	VkBufferCreateInfo		sCreate;
	VkMemoryRequirements	sRequirements;

	ClearVulkanBuffer(psBuffer);
	memset(&sCreate, 0, sizeof(sCreate));
	sCreate.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	sCreate.size = (uiSize > 0) ? uiSize : 4;
	sCreate.usage = uiUsage;
	sCreate.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (mcVulkan.vkCreateBuffer(mhDevice, &sCreate, NULL, &psBuffer->hBuffer) < 0)
	{
		ClearVulkanBuffer(psBuffer);
		return false;
	}
	mcVulkan.vkGetBufferMemoryRequirements(mhDevice, psBuffer->hBuffer, &sRequirements);
	if (!AllocateMemory(&psBuffer->hMemory, &sRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ||
		(mcVulkan.vkBindBufferMemory(mhDevice, psBuffer->hBuffer, psBuffer->hMemory, 0) < 0) ||
		(mcVulkan.vkMapMemory(mhDevice, psBuffer->hMemory, 0, VK_WHOLE_SIZE, 0, &psBuffer->pvMapped) < 0))
	{
		DestroyBuffer(psBuffer);
		return false;
	}
	psBuffer->uiSize = sCreate.size;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Freeing its memory unmaps it.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::DestroyBuffer(SVulkanBuffer* psBuffer)
{
	if (psBuffer->hBuffer)
	{
		mcVulkan.vkDestroyBuffer(mhDevice, psBuffer->hBuffer, NULL);
	}
	if (psBuffer->hMemory)
	{
		mcVulkan.vkFreeMemory(mhDevice, psBuffer->hMemory, NULL);
	}
	ClearVulkanBuffer(psBuffer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateImage(SVulkanImage* psImage, int32 iWidth, int32 iHeight, VkFormat eFormat, VkImageUsageFlags uiUsage, VkImageAspectFlags uiAspect)
{
	VkImageCreateInfo		sCreate;
	VkImageViewCreateInfo	sView;
	VkMemoryRequirements	sRequirements;

	ClearVulkanImage(psImage);
	memset(&sCreate, 0, sizeof(sCreate));
	sCreate.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	sCreate.imageType = VK_IMAGE_TYPE_2D;
	sCreate.format = eFormat;
	sCreate.extent.width = (uint32)iWidth;
	sCreate.extent.height = (uint32)iHeight;
	sCreate.extent.depth = 1;
	sCreate.mipLevels = 1;
	sCreate.arrayLayers = 1;
	sCreate.samples = VK_SAMPLE_COUNT_1_BIT;
	sCreate.tiling = VK_IMAGE_TILING_OPTIMAL;
	sCreate.usage = uiUsage;
	sCreate.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (mcVulkan.vkCreateImage(mhDevice, &sCreate, NULL, &psImage->hImage) < 0)
	{
		ClearVulkanImage(psImage);
		return false;
	}
	mcVulkan.vkGetImageMemoryRequirements(mhDevice, psImage->hImage, &sRequirements);
	if (!AllocateMemory(&psImage->hMemory, &sRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) || (mcVulkan.vkBindImageMemory(mhDevice, psImage->hImage, psImage->hMemory, 0) < 0))
	{
		DestroyImage(psImage);
		return false;
	}

	memset(&sView, 0, sizeof(sView));
	sView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	sView.image = psImage->hImage;
	sView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	sView.format = eFormat;
	sView.subresourceRange.aspectMask = uiAspect;
	sView.subresourceRange.levelCount = 1;
	sView.subresourceRange.layerCount = 1;
	if (mcVulkan.vkCreateImageView(mhDevice, &sView, NULL, &psImage->hView) < 0)
	{
		DestroyImage(psImage);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::DestroyImage(SVulkanImage* psImage)
{
	if (psImage->hView)
	{
		mcVulkan.vkDestroyImageView(mhDevice, psImage->hView, NULL);
	}
	if (psImage->hImage)
	{
		mcVulkan.vkDestroyImage(mhDevice, psImage->hImage, NULL);
	}
	if (psImage->hMemory)
	{
		mcVulkan.vkFreeMemory(mhDevice, psImage->hMemory, NULL);
	}
	ClearVulkanImage(psImage);
}


//////////////////////////////////////////////////////////////////////////
//
// The colour and depth images drawn into, and the buffer the colour is read
// back into.  Made again when a frame is another size.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::CreateTarget(int32 iWidth, int32 iHeight)
{
	DestroyTarget();
	if (!CreateImage(&msColour, iWidth, iHeight, VULKAN_COLOUR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT) ||
		!CreateImage(&msDepth, iWidth, iHeight, VULKAN_DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT) ||
		!CreateBuffer(&msReadback, (VkDeviceSize)iWidth * iHeight * sizeof(uint32), VK_BUFFER_USAGE_TRANSFER_DST_BIT))
	{
		DestroyTarget();
		return false;
	}
	miWidth = iWidth;
	miHeight = iHeight;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::DestroyTarget(void)
{
	DestroyImage(&msColour);
	DestroyImage(&msDepth);
	DestroyBuffer(&msReadback);
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::Barrier(VkImage hImage, VkImageAspectFlags uiAspect, VkImageLayout eFrom, VkImageLayout eTo, VkAccessFlags uiFromAccess, VkAccessFlags uiToAccess, VkPipelineStageFlags uiFromStage, VkPipelineStageFlags uiToStage)
{
	VkImageMemoryBarrier	sBarrier;

	memset(&sBarrier, 0, sizeof(sBarrier));
	sBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	sBarrier.srcAccessMask = uiFromAccess;
	sBarrier.dstAccessMask = uiToAccess;
	sBarrier.oldLayout = eFrom;
	sBarrier.newLayout = eTo;
	sBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	sBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	sBarrier.image = hImage;
	sBarrier.subresourceRange.aspectMask = uiAspect;
	sBarrier.subresourceRange.levelCount = 1;
	sBarrier.subresourceRange.layerCount = 1;
	mcVulkan.vkCmdPipelineBarrier(mhCommands, uiFromStage, uiToStage, 0, 0, NULL, 0, NULL, 1, &sBarrier);
}


//////////////////////////////////////////////////////////////////////////
//
// Clears the target to uiBackground, 0xAARRGGBB, starts drawing into it, and
// gives the shaders psFrame.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::BeginFrame(int32 iWidth, int32 iHeight, uint32 uiBackground, SVulkanFrameData* psFrame)
{
	VkCommandBufferBeginInfo	sBegin;
	VkRenderingAttachmentInfo	sColour;
	VkRenderingAttachmentInfo	sDepth;
	VkRenderingInfo				sRendering;
	VkViewport					sViewport;
	VkRect2D					sScissor;

	if (!mbReady || (iWidth <= 0) || (iHeight <= 0))
	{
		return false;
	}
	if (((iWidth != miWidth) || (iHeight != miHeight)) && !CreateTarget(iWidth, iHeight))
	{
		return false;
	}
	memcpy(msFrameData.pvMapped, psFrame, sizeof(SVulkanFrameData));

	mcVulkan.vkResetCommandBuffer(mhCommands, 0);
	memset(&sBegin, 0, sizeof(sBegin));
	sBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	sBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	mcVulkan.vkBeginCommandBuffer(mhCommands, &sBegin);

	Barrier(msColour.hImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	Barrier(msDepth.hImage, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

	memset(&sColour, 0, sizeof(sColour));
	sColour.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	sColour.imageView = msColour.hView;
	sColour.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	sColour.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	sColour.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	sColour.clearValue.color.float32[0] = (float32)((uiBackground >> 16) & 0xFF) / 255.0f;
	sColour.clearValue.color.float32[1] = (float32)((uiBackground >> 8) & 0xFF) / 255.0f;
	sColour.clearValue.color.float32[2] = (float32)(uiBackground & 0xFF) / 255.0f;
	sColour.clearValue.color.float32[3] = 1.0f;
	memset(&sDepth, 0, sizeof(sDepth));
	sDepth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	sDepth.imageView = msDepth.hView;
	sDepth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	sDepth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	sDepth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	sDepth.clearValue.depthStencil.depth = 1.0f;

	memset(&sRendering, 0, sizeof(sRendering));
	sRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	sRendering.renderArea.extent.width = (uint32)iWidth;
	sRendering.renderArea.extent.height = (uint32)iHeight;
	sRendering.layerCount = 1;
	sRendering.colorAttachmentCount = 1;
	sRendering.pColorAttachments = &sColour;
	sRendering.pDepthAttachment = &sDepth;
	mcVulkan.vkCmdBeginRendering(mhCommands, &sRendering);

	sViewport.x = 0.0f;
	sViewport.y = 0.0f;
	sViewport.width = (float32)iWidth;
	sViewport.height = (float32)iHeight;
	sViewport.minDepth = 0.0f;
	sViewport.maxDepth = 1.0f;
	mcVulkan.vkCmdSetViewport(mhCommands, 0, 1, &sViewport);
	sScissor.offset.x = 0;
	sScissor.offset.y = 0;
	sScissor.extent.width = (uint32)iWidth;
	sScissor.extent.height = (uint32)iHeight;
	mcVulkan.vkCmdSetScissor(mhCommands, 0, 1, &sScissor);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What's drawn next is opaque, or transparent.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::UseOpaque(void)
{
	mcVulkan.vkCmdBindPipeline(mhCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, mhOpaquePipeline);
	mcVulkan.vkCmdBindDescriptorSets(mhCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, mhPipelineLayout, 0, 1, &mhDescriptorSet, 0, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::UseTransparent(void)
{
	mcVulkan.vkCmdBindPipeline(mhCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, mhTransparentPipeline);
	mcVulkan.vkCmdBindDescriptorSets(mhCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, mhPipelineLayout, 0, 1, &mhDescriptorSet, 0, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// What the triangles drawn next are textured with (GetTexture).
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::UseTexture(VkDescriptorSet hTexture)
{
	mcVulkan.vkCmdBindDescriptorSets(mhCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, mhPipelineLayout, 1, 1, &hTexture, 0, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Some of the bound mesh's triangles.  A triangle that's anticlockwise in
// the scene is anticlockwise as Vulkan sees it, as its winding allows for
// the screen's y being down, and a mirrored one is the other way.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanRenderer::Draw(SVulkanDraw* psDraw, bool bMirrored, uint32 uiFirstIndex, uint32 uiNumIndices)
{
	mcVulkan.vkCmdSetFrontFace(mhCommands, bMirrored ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE);
	mcVulkan.vkCmdPushConstants(mhCommands, mhPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SVulkanDraw), psDraw);
	mcVulkan.vkCmdDrawIndexed(mhCommands, uiNumIndices, 1, uiFirstIndex, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The frame's commands, for its meshes to bind their buffers.
//
//////////////////////////////////////////////////////////////////////////
VkCommandBuffer CVulkanRenderer::GetCommands(void)
{
	return mhCommands;
}


//////////////////////////////////////////////////////////////////////////
//
// Waits for the frame to be drawn, and copies its pixels to puiPixels, a
// row of the frame's width after another.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::EndFrame(uint32* puiPixels)
{
	VkBufferImageCopy	sCopy;
	VkMemoryBarrier		sToHost;
	VkSubmitInfo		sSubmit;
	bool				bResult;

	mcVulkan.vkCmdEndRendering(mhCommands);
	Barrier(msColour.hImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	memset(&sCopy, 0, sizeof(sCopy));
	sCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sCopy.imageSubresource.layerCount = 1;
	sCopy.imageExtent.width = (uint32)miWidth;
	sCopy.imageExtent.height = (uint32)miHeight;
	sCopy.imageExtent.depth = 1;
	mcVulkan.vkCmdCopyImageToBuffer(mhCommands, msColour.hImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, msReadback.hBuffer, 1, &sCopy);
	memset(&sToHost, 0, sizeof(sToHost));
	sToHost.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	sToHost.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	sToHost.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
	mcVulkan.vkCmdPipelineBarrier(mhCommands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &sToHost, 0, NULL, 0, NULL);
	mcVulkan.vkEndCommandBuffer(mhCommands);

	memset(&sSubmit, 0, sizeof(sSubmit));
	sSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sSubmit.commandBufferCount = 1;
	sSubmit.pCommandBuffers = &mhCommands;
	bResult = (mcVulkan.vkQueueSubmit(mhQueue, 1, &sSubmit, mhFence) >= 0) && (mcVulkan.vkWaitForFences(mhDevice, 1, &mhFence, VK_TRUE, UINT64_MAX) >= 0);
	mcVulkan.vkResetFences(mhDevice, 1, &mhFence);
	if (bResult)
	{
		memcpy(puiPixels, msReadback.pvMapped, (size)miWidth * miHeight * sizeof(uint32));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanRenderer::IsReady(void) { return mbReady; }
const char* CVulkanRenderer::GetDeviceName(void) { return mszDeviceName.Text(); }
CVulkanFunctions* CVulkanRenderer::GetFunctions(void) { return &mcVulkan; }
VkDevice CVulkanRenderer::GetDevice(void) { return mhDevice; }

