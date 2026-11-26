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
#include "BaseLib/Logger.h"
#include "VulkanFunctions.h"


#ifdef _WIN32
#define VULKAN_LIBRARY	"vulkan-1.dll"
#else
#define VULKAN_LIBRARY	"libvulkan.so.1"
#endif


#define VULKAN_VALIDATION_LAYER		"VK_LAYER_KHRONOS_validation"
#define VULKAN_MAX_LAYERS			64


CVulkanInstance	gcVulkanInstance;


#ifdef __SANITIZE_ADDRESS__
//The driver Vulkan's loader finds may register itself on D-Bus (NVIDIA's does), and what libdbus allocated for it is
//still there when the driver has been unloaded: memory this library never had, and nothing a program that uses it
//can free.  LeakSanitizer, which would otherwise fail every such program over it, is told to pass over libdbus's.
extern "C" const char* __lsan_default_suppressions(void)
{
	return "leak:libdbus-1.so\n";
}
#endif // __SANITIZE_ADDRESS__


#define VULKAN_CLEAR_FUNCTION(name)		name = NULL;
#define VULKAN_FIND_GLOBAL(name)		name = (PFN_##name)mpfGetInstanceProcAddr(NULL, #name);  bResult = bResult && (name != NULL);
#define VULKAN_FIND_INSTANCE(name)		name = (PFN_##name)mpfGetInstanceProcAddr(hInstance, #name);  bResult = bResult && (name != NULL);
#define VULKAN_FIND_DEVICE(name)		name = (PFN_##name)vkGetDeviceProcAddr(hDevice, #name);  bResult = bResult && (name != NULL);
#define VULKAN_COPY_FUNCTION(name)		name = pcSource->name;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFunctions::Init(void)
{
	mcLibrary.Init();
	mpfGetInstanceProcAddr = NULL;
	VULKAN_GLOBAL_FUNCTIONS(VULKAN_CLEAR_FUNCTION)
	VULKAN_INSTANCE_FUNCTIONS(VULKAN_CLEAR_FUNCTION)
	VULKAN_DEVICE_FUNCTIONS(VULKAN_CLEAR_FUNCTION)
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFunctions::Kill(void)
{
	mcLibrary.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// False, quietly, if there's no Vulkan, as a program can draw without it.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanFunctions::Load(void)
{
	bool	bResult;

	if (!mcLibrary.Open(VULKAN_LIBRARY))
	{
		return false;
	}
	mpfGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)mcLibrary.GetFunction("vkGetInstanceProcAddr");
	if (mpfGetInstanceProcAddr == NULL)
	{
		return false;
	}
	bResult = true;
	VULKAN_GLOBAL_FUNCTIONS(VULKAN_FIND_GLOBAL)
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanFunctions::LoadInstance(VkInstance hInstance)
{
	bool	bResult;

	bResult = true;
	VULKAN_INSTANCE_FUNCTIONS(VULKAN_FIND_INSTANCE)
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanFunctions::LoadDevice(VkDevice hDevice)
{
	bool	bResult;

	bResult = true;
	VULKAN_DEVICE_FUNCTIONS(VULKAN_FIND_DEVICE)
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The source's global and instance functions; its library stays its own,
// so killing this closes nothing.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFunctions::CopyFunctions(const CVulkanFunctions* pcSource)
{
	mpfGetInstanceProcAddr = pcSource->mpfGetInstanceProcAddr;
	VULKAN_GLOBAL_FUNCTIONS(VULKAN_COPY_FUNCTION)
	VULKAN_INSTANCE_FUNCTIONS(VULKAN_COPY_FUNCTION)
}


//////////////////////////////////////////////////////////////////////////
//
// What the instance has that it may not, NULL if it hasn't.
//
//////////////////////////////////////////////////////////////////////////
PFN_vkVoidFunction CVulkanFunctions::GetInstanceFunction(VkInstance hInstance, const char* szName)
{
	return mpfGetInstanceProcAddr ? mpfGetInstanceProcAddr(hInstance, szName) : NULL;
}


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
// The process's instance and its functions, made the first time it's
// asked for (under the lock, as renderers may be made on more than one
// thread).  False if there's no Vulkan 1.3.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanInstance::Get(CVulkanFunctions* pcFunctions, VkInstance* phInstance)
{
	bool	bReady;

	mcLock.Lock();
	if (!mbTried)
	{
		mbTried = true;
		mbReady = Create();
		if (!mbReady)
		{
			Destroy();
		}
	}
	bReady = mbReady;
	mcLock.Unlock();

	if (bReady)
	{
		pcFunctions->CopyFunctions(&mcVulkan);
		*phInstance = mhInstance;
	}
	return bReady;
}


//////////////////////////////////////////////////////////////////////////
//
// After the program's last renderer has been freed, as they all use it.
// The next Get makes it again.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanInstance::Kill(void)
{
	mcLock.Lock();
	Destroy();
	mbTried = false;
	mbReady = false;
	mcLock.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// The library, and version 1.3, with no surfaces as it draws offscreen.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanInstance::Create(void)
{
	VkApplicationInfo					sApplication;
	VkInstanceCreateInfo				sCreate;
	VkDebugUtilsMessengerCreateInfoEXT	sMessenger;
	PFN_vkCreateDebugUtilsMessengerEXT	pfCreateMessenger;
	const char*							szLayer;
	const char*							szExtension;
	uint32								uiVersion;
	bool								bValidate;

	mcVulkan.Init();
	mhInstance = VK_NULL_HANDLE;
	mhMessenger = VK_NULL_HANDLE;
	mpfDestroyMessenger = NULL;
	if (!mcVulkan.Load())
	{
		return false;
	}

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
// Whatever Create made, under the lock.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanInstance::Destroy(void)
{
	if (mhInstance)
	{
		if (mhMessenger && mpfDestroyMessenger)
		{
			mpfDestroyMessenger(mhInstance, mhMessenger, NULL);
		}
		mcVulkan.vkDestroyInstance(mhInstance, NULL);
	}
	mhInstance = VK_NULL_HANDLE;
	mhMessenger = VK_NULL_HANDLE;
	mpfDestroyMessenger = NULL;
	mcVulkan.Kill();
}

