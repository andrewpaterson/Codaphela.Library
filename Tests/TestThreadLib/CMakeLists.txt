# Linux build of TestThreadLib (BaseLib + TestLib + ThreadLib + TestThreadLib).
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S CommonLibraries/Tests/TestThreadLib -B build/TestThreadLib64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S CommonLibraries/Tests/TestThreadLib -B build/TestThreadLib -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestThreadLib
#   ctest --test-dir build/TestThreadLib -V
#
# TestThreadLib must be run as 'TestThreadLib --test param1 --param:2' (TestMainProcessDirector
# checks those parameters); the other redirects are
# the child processes that the tests start.
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# The targets are read from TestThreadLib.sln (codaphela_solution): the libraries its GCC-x64 rows build, each one's
# sources from its .vcxproj, so Visual Studio stays the source of truth.  Building re-runs the configure step when the
# solution or a project changes.  Test directories can contain .cpp files that are test data, so never glob *.cpp to
# find sources.

cmake_minimum_required(VERSION 3.24)
project(TestThreadLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
set(TESTTHREADLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestThreadLib")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")

codaphela_options("${COMMON_DIR}")

codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/TestThreadLib.sln" PROGRAMS TestThreadLib)

# With the arguments TestMainProcessDirector checks.
add_test(NAME TestThreadLib COMMAND TestThreadLib --test param1 --param:2 WORKING_DIRECTORY "${TESTTHREADLIB_DIR}")
