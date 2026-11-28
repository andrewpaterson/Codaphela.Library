# Linux build of PNGToSFT (BaseLib + StandardLib + SupportLib + the tool), the same way as the test projects'.
#
#   cmake -S CommonLibraries/Tools/PNGToSFT -B build/PNGToSFT -DCMAKE_BUILD_TYPE=Release
#   cmake --build build/PNGToSFT
#
# The targets are read from PNGToSFT.sln (codaphela_solution): the libraries its GCC-x64 rows build, each one's
# sources from its .vcxproj, so Visual Studio stays the source of truth.  Building re-runs the configure step when the
# solution or a project changes.

cmake_minimum_required(VERSION 3.24)
project(PNGToSFT CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")

codaphela_options("${COMMON_DIR}")

codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/PNGToSFT.sln" PROGRAMS PNGToSFT)
