# Linux build of TestVulkanLib (BaseLib + TestLib + StandardLib + SupportLib +
# RasterLib + RenderLib + RasterRenderLib + VulkanLib + TestVulkanLib).
#
# Needs Vulkan's headers (libvulkan-dev).  VulkanLib loads libvulkan.so.1 while
# it runs, so nothing links against it, and the tests say so and test nothing
# without a device that has Vulkan 1.3.
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S CommonLibraries/Tests/TestVulkanLib -B build/TestVulkanLib64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S CommonLibraries/Tests/TestVulkanLib -B build/TestVulkanLib -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestVulkanLib
#   ctest --test-dir build/TestVulkanLib -V
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# Source lists are read from the .vcxproj files so Visual Studio stays the
# source of truth.  Building re-runs the configure step automatically when a
# .vcxproj changes.  Test directories can contain .cpp files that are test data,
# so never glob *.cpp to find sources.

cmake_minimum_required(VERSION 3.19)
project(TestVulkanLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
set(BASELIB_DIR "${COMMON_DIR}/BaseLib")
set(TESTLIB_DIR "${COMMON_DIR}/TestLib")
set(STANDARDLIB_DIR "${COMMON_DIR}/StandardLib")
set(SUPPORTLIB_DIR "${COMMON_DIR}/SupportLib")
set(RASTERLIB_DIR "${COMMON_DIR}/RasterLib")
set(RENDERLIB_DIR "${COMMON_DIR}/RenderLib")
set(RASTERRENDERLIB_DIR "${COMMON_DIR}/RasterRenderLib")
set(VULKANLIB_DIR "${COMMON_DIR}/VulkanLib")
set(TESTVULKANLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestVulkanLib")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")


vcxproj_sources(BASELIB_SOURCES "${BASELIB_DIR}/BaseLib.vcxproj")

vcxproj_sources(TESTLIB_SOURCES "${TESTLIB_DIR}/TestLib.vcxproj")

vcxproj_sources(STANDARDLIB_SOURCES "${STANDARDLIB_DIR}/StandardLib.vcxproj")

vcxproj_sources(SUPPORTLIB_SOURCES "${SUPPORTLIB_DIR}/SupportLib.vcxproj")

vcxproj_sources(RASTERLIB_SOURCES "${RASTERLIB_DIR}/RasterLib.vcxproj")

vcxproj_sources(RENDERLIB_SOURCES "${RENDERLIB_DIR}/RenderLib.vcxproj")

vcxproj_sources(RASTERRENDERLIB_SOURCES "${RASTERRENDERLIB_DIR}/RasterRenderLib.vcxproj")

vcxproj_sources(VULKANLIB_SOURCES "${VULKANLIB_DIR}/VulkanLib.vcxproj")

vcxproj_sources(TESTVULKANLIB_SOURCES "${TESTVULKANLIB_DIR}/TestVulkanLib.vcxproj")


codaphela_options("${COMMON_DIR}")


# CMutex uses pthreads on Linux.
find_package(Threads REQUIRED)

add_library(BaseLib STATIC ${BASELIB_SOURCES})
target_link_libraries(BaseLib PUBLIC baselib_options)
target_link_libraries(BaseLib PUBLIC Threads::Threads)

# CSharedLibrary uses dlopen.
target_link_libraries(BaseLib PUBLIC ${CMAKE_DL_LIBS})

# Uses AVX intrinsics unconditionally; MSVC allows that without a flag, g++ does not.

add_library(TestLib STATIC ${TESTLIB_SOURCES})
target_link_libraries(TestLib PUBLIC BaseLib)

add_library(StandardLib STATIC ${STANDARDLIB_SOURCES})
target_link_libraries(StandardLib PUBLIC BaseLib)

# The Debug configurations of StandardLib (only) define DEBUG_POINTER.
target_compile_definitions(StandardLib PRIVATE $<$<CONFIG:Debug>:DEBUG_POINTER>)

add_library(SupportLib STATIC ${SUPPORTLIB_SOURCES})
target_link_libraries(SupportLib PUBLIC StandardLib BaseLib)

add_library(RasterLib STATIC ${RASTERLIB_SOURCES})
target_link_libraries(RasterLib PUBLIC StandardLib BaseLib)

add_library(RenderLib STATIC ${RENDERLIB_SOURCES})
target_link_libraries(RenderLib PUBLIC SupportLib StandardLib BaseLib)

add_library(RasterRenderLib STATIC ${RASTERRENDERLIB_SOURCES})
target_link_libraries(RasterRenderLib PUBLIC RenderLib RasterLib SupportLib StandardLib BaseLib)

# Only Vulkan's headers: VulkanLib finds its functions in libvulkan.so.1 while it runs.
find_path(VULKAN_INCLUDE_DIR vulkan/vulkan.h HINTS "$ENV{VULKAN_SDK}/include" REQUIRED)
add_library(VulkanLib STATIC ${VULKANLIB_SOURCES})
target_include_directories(VulkanLib PUBLIC "${VULKAN_INCLUDE_DIR}")
target_link_libraries(VulkanLib PUBLIC RenderLib SupportLib StandardLib BaseLib)

add_executable(TestVulkanLib ${TESTVULKANLIB_SOURCES})
target_link_libraries(TestVulkanLib PRIVATE TestLib VulkanLib RasterRenderLib RenderLib RasterLib SupportLib StandardLib BaseLib)

# The tests read their input (Input/ ...) relative to the project directory,
# as Visual Studio does.
enable_testing()
add_test(NAME TestVulkanLib COMMAND TestVulkanLib WORKING_DIRECTORY "${TESTVULKANLIB_DIR}")
