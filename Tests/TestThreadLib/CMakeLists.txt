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
# Source lists are read from the .vcxproj files so Visual Studio stays the
# source of truth.  Building re-runs the configure step automatically when a
# .vcxproj changes.  Test directories can contain .cpp files that are test data,
# so never glob *.cpp to find sources.

cmake_minimum_required(VERSION 3.19)
project(TestThreadLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
set(BASELIB_DIR "${COMMON_DIR}/BaseLib")
set(TESTLIB_DIR "${COMMON_DIR}/TestLib")
set(THREADLIB_DIR "${COMMON_DIR}/ThreadLib")
set(TESTTHREADLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestThreadLib")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")


vcxproj_sources(BASELIB_SOURCES "${BASELIB_DIR}/BaseLib.vcxproj")

vcxproj_sources(TESTLIB_SOURCES "${TESTLIB_DIR}/TestLib.vcxproj")

vcxproj_sources(THREADLIB_SOURCES "${THREADLIB_DIR}/ThreadLib.vcxproj")

vcxproj_sources(TESTTHREADLIB_SOURCES "${TESTTHREADLIB_DIR}/TestThreadLib.vcxproj")


codaphela_options("${COMMON_DIR}")


# CMutex uses pthreads on Linux.
find_package(Threads REQUIRED)

add_library(BaseLib STATIC ${BASELIB_SOURCES})
target_link_libraries(BaseLib PUBLIC baselib_options)
target_link_libraries(BaseLib PUBLIC Threads::Threads)

# Uses AVX intrinsics unconditionally; MSVC allows that without a flag, g++ does not.

add_library(TestLib STATIC ${TESTLIB_SOURCES})
target_link_libraries(TestLib PUBLIC BaseLib)

add_library(ThreadLib STATIC ${THREADLIB_SOURCES})
target_link_libraries(ThreadLib PUBLIC BaseLib)

add_executable(TestThreadLib ${TESTTHREADLIB_SOURCES})
target_link_libraries(TestThreadLib PRIVATE TestLib ThreadLib BaseLib)

# The tests read their input relative to the project directory, as Visual
# Studio does.
enable_testing()
add_test(NAME TestThreadLib COMMAND TestThreadLib --test param1 --param:2 WORKING_DIRECTORY "${TESTTHREADLIB_DIR}")
