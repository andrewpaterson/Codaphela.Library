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
#include "VulkanFunctions.h"


#ifdef _WIN32
#define VULKAN_LIBRARY	"vulkan-1.dll"
#else
#define VULKAN_LIBRARY	"libvulkan.so.1"
#endif


#define VULKAN_CLEAR_FUNCTION(name)		name = NULL;
#define VULKAN_FIND_GLOBAL(name)		name = (PFN_##name)mpfGetInstanceProcAddr(NULL, #name);  bResult = bResult && (name != NULL);
#define VULKAN_FIND_INSTANCE(name)		name = (PFN_##name)mpfGetInstanceProcAddr(hInstance, #name);  bResult = bResult && (name != NULL);
#define VULKAN_FIND_DEVICE(name)		name = (PFN_##name)vkGetDeviceProcAddr(hDevice, #name);  bResult = bResult && (name != NULL);


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
// What the instance has that it may not, NULL if it hasn't.
//
//////////////////////////////////////////////////////////////////////////
PFN_vkVoidFunction CVulkanFunctions::GetInstanceFunction(VkInstance hInstance, const char* szName)
{
	return mpfGetInstanceProcAddr ? mpfGetInstanceProcAddr(hInstance, szName) : NULL;
}

