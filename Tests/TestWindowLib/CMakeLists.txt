# Linux build of TestWindowLib (BaseLib + TestLib + StandardLib + SupportLib +
# InputLib + WindowLib + WinRefLib + TestWindowLib).
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S CommonLibraries/Tests/TestWindowLib -B build/TestWindowLib64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S CommonLibraries/Tests/TestWindowLib -B build/TestWindowLib -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestWindowLib
#   ctest --test-dir build/TestWindowLib -V
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# The targets are read from TestWindowLib.sln (codaphela_solution): the libraries its GCC-x64 rows build, each one's
# sources from its .vcxproj, so Visual Studio stays the source of truth.  Building re-runs the configure step when the
# solution or a project changes.  Test directories can contain .cpp files that are test data, so never glob *.cpp to
# find sources.

cmake_minimum_required(VERSION 3.24)
project(TestWindowLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")

codaphela_options("${COMMON_DIR}")

codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/TestWindowLib.sln" TESTS TestWindowLib)
