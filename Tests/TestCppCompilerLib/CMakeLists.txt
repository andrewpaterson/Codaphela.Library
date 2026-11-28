# Linux build of TestCppCompilerLib (BaseLib + TestLib + StandardLib + CppCompilerLib +
# MicroprocessorLib + Assembler65Lib + TestCppCompilerLib).  The 65C64 generator tests
# assemble the generated code and run it on the 65C64 simulator.
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S CommonLibraries/Tests/TestCppCompilerLib -B build/TestCppCompilerLib64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S CommonLibraries/Tests/TestCppCompilerLib -B build/TestCppCompilerLib -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestCppCompilerLib
#   ctest --test-dir build/TestCppCompilerLib -V
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# The targets are read from TestCppCompilerLib.sln (codaphela_solution): the libraries its GCC-x64 rows build, each
# one's sources from its .vcxproj, so Visual Studio stays the source of truth.  Building re-runs the configure step
# when the solution or a project changes.  Test directories can contain .cpp files that are test data, so never glob
# *.cpp to find sources.

cmake_minimum_required(VERSION 3.24)
project(TestCppCompilerLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
# Assembler65Lib (and Libc65) are in the HCP65 repository, beside CommonLibraries.
set(HCP65_LIBRARIES_DIR "${CODAPHELA_ROOT}/HCP65/Libraries")
set(TESTCPPCOMPILERLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestCppCompilerLib")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")

codaphela_options("${COMMON_DIR}" "${HCP65_LIBRARIES_DIR}")

codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/TestCppCompilerLib.sln" TESTS TestCppCompilerLib)

# TestUnit*.cpp are also input to the preprocessor tests, which check line
# numbers in them, so they are left as they are.
set_source_files_properties(
	"${TESTCPPCOMPILERLIB_DIR}/TestUnit1.cpp"
	"${TESTCPPCOMPILERLIB_DIR}/TestUnit2.cpp"
	PROPERTIES COMPILE_OPTIONS -Wno-unused-but-set-variable)
