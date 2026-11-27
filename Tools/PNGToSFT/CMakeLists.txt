# Linux build of PNGToSFT (BaseLib + StandardLib + SupportLib + the tool), the same way as the test projects'.
#
#   cmake -S CommonLibraries/Tools/PNGToSFT -B build/PNGToSFT -DCMAKE_BUILD_TYPE=Release
#   cmake --build build/PNGToSFT
#
# Source lists are read from the .vcxproj files so Visual Studio stays the source of truth.

cmake_minimum_required(VERSION 3.19)
project(PNGToSFT CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
set(BASELIB_DIR "${COMMON_DIR}/BaseLib")
set(STANDARDLIB_DIR "${COMMON_DIR}/StandardLib")
set(SUPPORTLIB_DIR "${COMMON_DIR}/SupportLib")
set(PNGTOSFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PNGToSFT")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")


vcxproj_sources(BASELIB_SOURCES "${BASELIB_DIR}/BaseLib.vcxproj")

vcxproj_sources(STANDARDLIB_SOURCES "${STANDARDLIB_DIR}/StandardLib.vcxproj")

vcxproj_sources(SUPPORTLIB_SOURCES "${SUPPORTLIB_DIR}/SupportLib.vcxproj")

vcxproj_sources(PNGTOSFT_SOURCES "${PNGTOSFT_DIR}/PNGToSFT.vcxproj")


codaphela_options("${COMMON_DIR}")


# CMutex uses pthreads on Linux.
find_package(Threads REQUIRED)

add_library(BaseLib STATIC ${BASELIB_SOURCES})
target_link_libraries(BaseLib PUBLIC baselib_options)
target_link_libraries(BaseLib PUBLIC Threads::Threads)

# Uses AVX intrinsics unconditionally; MSVC allows that without a flag, g++ does not.

add_library(StandardLib STATIC ${STANDARDLIB_SOURCES})
target_link_libraries(StandardLib PUBLIC BaseLib)

# The Debug configurations of StandardLib (only) define DEBUG_POINTER.
target_compile_definitions(StandardLib PRIVATE $<$<CONFIG:Debug>:DEBUG_POINTER>)

add_library(SupportLib STATIC ${SUPPORTLIB_SOURCES})
target_link_libraries(SupportLib PUBLIC StandardLib BaseLib)

add_executable(PNGToSFT ${PNGTOSFT_SOURCES})
target_link_libraries(PNGToSFT PRIVATE SupportLib StandardLib BaseLib)
