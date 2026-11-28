# Linux build of TestStandardLib65 (BaseLib + TestLib + StandardLib + CppCompilerLib +
# MicroprocessorLib + Assembler65Lib + TestStandardLib65).  BaseLib, StandardLib, TestLib
# and the TestStandardLib tests (read from their .vcxproj files) are compiled with the
# 65C64 generator together with Libc65 and each test is run on the 65C64 (CFast65C64).
# TestProject65.cpp, which runs them, is shared with TestBaseLib65.
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S CommonLibraries/Tests/TestStandardLib65 -B build/TestStandardLib65_64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S CommonLibraries/Tests/TestStandardLib65 -B build/TestStandardLib65 -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestStandardLib65
#   ctest --test-dir build/TestStandardLib65 -V
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# The targets are read from TestStandardLib65.sln (codaphela_solution): the libraries its GCC-x64 rows build, each
# one's sources from its .vcxproj, so Visual Studio stays the source of truth.  Building re-runs the configure step
# when the solution or a project changes.  Test directories can contain .cpp files that are test data, so never glob
# *.cpp to find sources.

cmake_minimum_required(VERSION 3.24)
project(TestStandardLib65 CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
# Assembler65Lib (and Libc65) are in the HCP65 repository, beside CommonLibraries.
set(HCP65_LIBRARIES_DIR "${CODAPHELA_ROOT}/HCP65/Libraries")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")

codaphela_options("${COMMON_DIR}" "${HCP65_LIBRARIES_DIR}")

codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/TestStandardLib65.sln" TESTS TestStandardLib65)
