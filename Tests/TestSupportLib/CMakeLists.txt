# Linux build of TestSupportLib (BaseLib + TestLib + StandardLib + SupportLib +
# TestSupportLib).
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S CommonLibraries/Tests/TestSupportLib -B build/TestSupportLib64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S CommonLibraries/Tests/TestSupportLib -B build/TestSupportLib -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestSupportLib
#   ctest --test-dir build/TestSupportLib -V
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# Add -DBASELIB_TSAN=ON (64-bit) for ThreadSanitizer, in its own build directory.  Run it with address space
# randomisation off (setarch -R ctest ...): ThreadSanitizer doesn't start where the kernel maps memory high.
#
# Add -DBASELIB_SIZE16=ON (32-bit only) to check the W65C816's size model on the host: size is 16 bits and lsize 32,
# and -Wconversion warns of each wider value narrowed to a size ("to 'size'" in the warning).
#
# The targets are read from TestSupportLib.sln (codaphela_solution): the libraries its GCC-x64 rows build, each one's
# sources from its .vcxproj, so Visual Studio stays the source of truth.  Building re-runs the configure step when the
# solution or a project changes.  Test directories can contain .cpp files that are test data, so never glob *.cpp to
# find sources.

cmake_minimum_required(VERSION 3.24)
project(TestSupportLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")

codaphela_options("${COMMON_DIR}")

codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/TestSupportLib.sln" TESTS TestSupportLib)
