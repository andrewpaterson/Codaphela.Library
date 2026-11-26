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
#ifndef __VULKAN_FUNCTIONS_H__
#define __VULKAN_FUNCTIONS_H__
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "BaseLib/SharedLibrary.h"
#include "BaseLib/SpinLock.h"


//What's found without an instance.
#define VULKAN_GLOBAL_FUNCTIONS(F) \
	F(vkCreateInstance) \
	F(vkEnumerateInstanceVersion) \
	F(vkEnumerateInstanceLayerProperties)

//What's found with the instance.
#define VULKAN_INSTANCE_FUNCTIONS(F) \
	F(vkDestroyInstance) \
	F(vkEnumeratePhysicalDevices) \
	F(vkGetPhysicalDeviceProperties) \
	F(vkGetPhysicalDeviceQueueFamilyProperties) \
	F(vkGetPhysicalDeviceMemoryProperties) \
	F(vkCreateDevice) \
	F(vkGetDeviceProcAddr)

//What's found with the device.
#define VULKAN_DEVICE_FUNCTIONS(F) \
	F(vkDestroyDevice) \
	F(vkGetDeviceQueue) \
	F(vkDeviceWaitIdle) \
	F(vkCreateCommandPool) \
	F(vkDestroyCommandPool) \
	F(vkAllocateCommandBuffers) \
	F(vkBeginCommandBuffer) \
	F(vkEndCommandBuffer) \
	F(vkResetCommandBuffer) \
	F(vkQueueSubmit) \
	F(vkCreateFence) \
	F(vkDestroyFence) \
	F(vkWaitForFences) \
	F(vkResetFences) \
	F(vkCreateBuffer) \
	F(vkDestroyBuffer) \
	F(vkGetBufferMemoryRequirements) \
	F(vkBindBufferMemory) \
	F(vkAllocateMemory) \
	F(vkFreeMemory) \
	F(vkMapMemory) \
	F(vkCreateImage) \
	F(vkDestroyImage) \
	F(vkGetImageMemoryRequirements) \
	F(vkBindImageMemory) \
	F(vkCreateImageView) \
	F(vkDestroyImageView) \
	F(vkCreateShaderModule) \
	F(vkDestroyShaderModule) \
	F(vkCreateDescriptorSetLayout) \
	F(vkDestroyDescriptorSetLayout) \
	F(vkCreateDescriptorPool) \
	F(vkDestroyDescriptorPool) \
	F(vkAllocateDescriptorSets) \
	F(vkFreeDescriptorSets) \
	F(vkUpdateDescriptorSets) \
	F(vkCreatePipelineLayout) \
	F(vkDestroyPipelineLayout) \
	F(vkCreateGraphicsPipelines) \
	F(vkDestroyPipeline) \
	F(vkCreateSampler) \
	F(vkDestroySampler) \
	F(vkCmdCopyBufferToImage) \
	F(vkCmdPipelineBarrier) \
	F(vkCmdBeginRendering) \
	F(vkCmdEndRendering) \
	F(vkCmdSetViewport) \
	F(vkCmdSetScissor) \
	F(vkCmdSetFrontFace) \
	F(vkCmdBindPipeline) \
	F(vkCmdBindDescriptorSets) \
	F(vkCmdBindVertexBuffers) \
	F(vkCmdBindIndexBuffer) \
	F(vkCmdPushConstants) \
	F(vkCmdDrawIndexed) \
	F(vkCmdCopyImageToBuffer)


#define VULKAN_DECLARE_FUNCTION(name)	PFN_##name name;


//Vulkan's functions, found in the library the driver installs (vulkan-1.dll, libvulkan.so.1) while the program runs, so that
//nothing links against Vulkan and a program runs without it, finding it isn't there.  Load opens the library and finds what
//needs no instance, LoadInstance and LoadDevice what needs those.  Each is false if anything is missing.  GetInstanceFunction
//finds what an instance may not have, as an extension's functions.  CopyFunctions takes another's global and instance functions
//but not its library, as a renderer does from the process's instance (CVulkanInstance) before it finds its own device's.
class CVulkanFunctions
{
protected:
	CSharedLibrary				mcLibrary;
	PFN_vkGetInstanceProcAddr	mpfGetInstanceProcAddr;

public:
	VULKAN_GLOBAL_FUNCTIONS(VULKAN_DECLARE_FUNCTION)
	VULKAN_INSTANCE_FUNCTIONS(VULKAN_DECLARE_FUNCTION)
	VULKAN_DEVICE_FUNCTIONS(VULKAN_DECLARE_FUNCTION)

	void	Init(void);
	void	Kill(void);

	bool	Load(void);
	bool	LoadInstance(VkInstance hInstance);
	bool	LoadDevice(VkDevice hDevice);
	void	CopyFunctions(const CVulkanFunctions* pcSource);

	PFN_vkVoidFunction	GetInstanceFunction(VkInstance hInstance, const char* szName);
};


//The process's Vulkan (gcVulkanInstance): the library, what needs no device, and the one instance every renderer makes its
//device on.  The first Get makes them and they're kept until the program ends.  The driver mustn't be unloaded while the program
//runs: NVIDIA's 32-bit driver (nvoglv32.dll, 32.0.15.6094) leaves a thread of its own running that faults when the driver is
//loaded again, as it was twelve times in a TestVulkanLib run when each renderer had an instance of its own (an access violation
//in Win32 builds now and then, never in x64).  Get is false, quietly, if there's no Vulkan 1.3, and stays false.  A Debug build
//uses the validation layer if it's installed, and logs what it reports.  Kill is for a program that wants it gone before it
//ends, after its last renderer; none needs to, as the instance holds nothing the program's memory knows about.
class CVulkanInstance
{
protected:
	CSpinLock							mcLock;		//All zeros is unlocked, so gcVulkanInstance needs no Init.  Held while the instance is made, once.
	bool								mbTried;
	bool								mbReady;
	CVulkanFunctions					mcVulkan;
	VkInstance							mhInstance;
	VkDebugUtilsMessengerEXT			mhMessenger;
	PFN_vkDestroyDebugUtilsMessengerEXT	mpfDestroyMessenger;

public:
	bool	Get(CVulkanFunctions* pcFunctions, VkInstance* phInstance);
	void	Kill(void);

protected:
	bool	Create(void);
	void	Destroy(void);
};


extern CVulkanInstance	gcVulkanInstance;


#endif // __VULKAN_FUNCTIONS_H__

