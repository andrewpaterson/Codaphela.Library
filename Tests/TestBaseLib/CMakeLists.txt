# Linux build of TestBaseLib (BaseLib + TestLib + TestBaseLib).
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S CommonLibraries/Tests/TestBaseLib -B build/TestBaseLib64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S CommonLibraries/Tests/TestBaseLib -B build/TestBaseLib -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestBaseLib
#   ctest --test-dir build/TestBaseLib --output-on-failure
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# Add -DBASELIB_MALLOC=ON for gcSystemAllocator to use malloc, which valgrind knows the blocks of (it does not know
# CThreadMallocator's).
#
# Add -DBASELIB_TSAN=ON (64-bit) for ThreadSanitizer, in its own build directory.  Run it with address space
# randomisation off (setarch -R ctest ...): ThreadSanitizer doesn't start where the kernel maps memory high.
#
# Add -DBASELIB_SIZE16=ON (32-bit only) to check the W65C816's size model on the host: size is 16 bits and lsize 32,
# and -Wconversion warns of each wider value narrowed to a size ("to 'size'" in the warning).
#
#   cmake -S CommonLibraries/Tests/TestBaseLib -B build/TestBaseLibSize16 -G Ninja -DBASELIB_SIZE16=ON
#
# Source lists are read from the .vcxproj files so Visual Studio stays the
# source of truth.  Building re-runs the configure step automatically when a
# .vcxproj changes.  Test directories contain .cpp files that are test data
# (e.g. SDCard.cpp), so never glob *.cpp to find sources.

cmake_minimum_required(VERSION 3.19)
project(TestBaseLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
set(BASELIB_DIR "${COMMON_DIR}/BaseLib")
set(TESTLIB_DIR "${COMMON_DIR}/TestLib")
set(TESTBASELIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestBaseLib")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")


vcxproj_sources(BASELIB_SOURCES "${BASELIB_DIR}/BaseLib.vcxproj")

vcxproj_sources(TESTLIB_SOURCES "${TESTLIB_DIR}/TestLib.vcxproj")

vcxproj_sources(TESTBASELIB_SOURCES "${TESTBASELIB_DIR}/TestBaseLib.vcxproj")


codaphela_options("${COMMON_DIR}")


# CMutex uses pthreads on Linux.
find_package(Threads REQUIRED)

add_library(BaseLib STATIC ${BASELIB_SOURCES})
target_link_libraries(BaseLib PUBLIC baselib_options)
target_link_libraries(BaseLib PUBLIC Threads::Threads)

# Uses AVX intrinsics unconditionally; MSVC allows that without a flag, g++ does not.

add_library(TestLib STATIC ${TESTLIB_SOURCES})
target_link_libraries(TestLib PUBLIC BaseLib)

add_executable(TestBaseLib ${TESTBASELIB_SOURCES})
target_link_libraries(TestBaseLib PRIVATE TestLib BaseLib)

# The tests read their input (Input/, Finder/, *.xml ...) relative to the
# project directory, as Visual Studio does.
enable_testing()
add_test(NAME TestBaseLib COMMAND TestBaseLib WORKING_DIRECTORY "${TESTBASELIB_DIR}")
