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
# The targets are read from TestVulkanLib.sln (codaphela_solution): the libraries its GCC-x64 rows build, each one's
# sources from its .vcxproj, so Visual Studio stays the source of truth.  Building re-runs the configure step when the
# solution or a project changes.  Test directories can contain .cpp files that are test data, so never glob *.cpp to
# find sources.

cmake_minimum_required(VERSION 3.24)
project(TestVulkanLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")

find_path(VULKAN_INCLUDE_DIR vulkan/vulkan.h HINTS "$ENV{VULKAN_SDK}/include" REQUIRED)
codaphela_options("${COMMON_DIR}" "${VULKAN_INCLUDE_DIR}")

codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/TestVulkanLib.sln" TESTS TestVulkanLib)
