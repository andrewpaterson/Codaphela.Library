# Linux build of TestJavaParserLib (BaseLib + TestLib + StandardLib + JavaParserLib + TestJavaParserLib).
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S CommonLibraries/Tests/TestJavaParserLib -B build/TestJavaParserLib64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S CommonLibraries/Tests/TestJavaParserLib -B build/TestJavaParserLib -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestJavaParserLib
#   ctest --test-dir build/TestJavaParserLib -V
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# Source lists are read from the .vcxproj files so Visual Studio stays the
# source of truth.  Building re-runs the configure step automatically when a
# .vcxproj changes.  Test directories can contain .cpp files that are test data,
# so never glob *.cpp to find sources.

cmake_minimum_required(VERSION 3.19)
project(TestJavaParserLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
set(BASELIB_DIR "${COMMON_DIR}/BaseLib")
set(TESTLIB_DIR "${COMMON_DIR}/TestLib")
set(STANDARDLIB_DIR "${COMMON_DIR}/StandardLib")
set(JAVAPARSERLIB_DIR "${COMMON_DIR}/JavaParserLib")
set(TESTJAVAPARSERLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestJavaParserLib")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")


vcxproj_sources(BASELIB_SOURCES "${BASELIB_DIR}/BaseLib.vcxproj")

vcxproj_sources(TESTLIB_SOURCES "${TESTLIB_DIR}/TestLib.vcxproj")

vcxproj_sources(STANDARDLIB_SOURCES "${STANDARDLIB_DIR}/StandardLib.vcxproj")

vcxproj_sources(JAVAPARSERLIB_SOURCES "${JAVAPARSERLIB_DIR}/JavaParserLib.vcxproj")

vcxproj_sources(TESTJAVAPARSERLIB_SOURCES "${TESTJAVAPARSERLIB_DIR}/TestJavaParserLib.vcxproj")


codaphela_options("${COMMON_DIR}")


# CMutex uses pthreads on Linux.
find_package(Threads REQUIRED)

add_library(BaseLib STATIC ${BASELIB_SOURCES})
target_link_libraries(BaseLib PUBLIC baselib_options)
target_link_libraries(BaseLib PUBLIC Threads::Threads)

# Uses AVX intrinsics unconditionally; MSVC allows that without a flag, g++ does not.

add_library(TestLib STATIC ${TESTLIB_SOURCES})
target_link_libraries(TestLib PUBLIC BaseLib)

add_library(StandardLib STATIC ${STANDARDLIB_SOURCES})
target_link_libraries(StandardLib PUBLIC BaseLib)

# The Debug configurations of StandardLib (only) define DEBUG_POINTER.
target_compile_definitions(StandardLib PRIVATE $<$<CONFIG:Debug>:DEBUG_POINTER>)

add_library(JavaParserLib STATIC ${JAVAPARSERLIB_SOURCES})
target_link_libraries(JavaParserLib PUBLIC StandardLib BaseLib)

add_executable(TestJavaParserLib ${TESTJAVAPARSERLIB_SOURCES})
target_link_libraries(TestJavaParserLib PRIVATE TestLib JavaParserLib StandardLib BaseLib)

# The tests read their input (Input/ ...) relative to the project directory,
# as Visual Studio does.
enable_testing()
add_test(NAME TestJavaParserLib COMMAND TestJavaParserLib WORKING_DIRECTORY "${TESTJAVAPARSERLIB_DIR}")
