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
# Source lists are read from the .vcxproj files so Visual Studio stays the
# source of truth.  Building re-runs the configure step automatically when a
# .vcxproj changes.  Test directories can contain .cpp files that are test data,
# so never glob *.cpp to find sources.

cmake_minimum_required(VERSION 3.19)
project(TestCppCompilerLib CXX)


get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/CommonLibraries/Libraries")
set(BASELIB_DIR "${COMMON_DIR}/BaseLib")
set(TESTLIB_DIR "${COMMON_DIR}/TestLib")
set(STANDARDLIB_DIR "${COMMON_DIR}/StandardLib")
set(CPPCOMPILERLIB_DIR "${COMMON_DIR}/CppCompilerLib")
set(MICROPROCESSORLIB_DIR "${COMMON_DIR}/MicroprocessorLib")
set(THREADLIB_DIR "${COMMON_DIR}/ThreadLib")
# Assembler65Lib (and Libc65) are in the HCP65 repository, beside CommonLibraries.
set(HCP65_LIBRARIES_DIR "${CODAPHELA_ROOT}/HCP65/Libraries")
set(ASSEMBLER65LIB_DIR "${HCP65_LIBRARIES_DIR}/Assembler65Lib")
set(TESTCPPCOMPILERLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestCppCompilerLib")

include("${COMMON_DIR}/../CMake/Codaphela.cmake")


vcxproj_sources(BASELIB_SOURCES "${BASELIB_DIR}/BaseLib.vcxproj")

vcxproj_sources(TESTLIB_SOURCES "${TESTLIB_DIR}/TestLib.vcxproj")

vcxproj_sources(STANDARDLIB_SOURCES "${STANDARDLIB_DIR}/StandardLib.vcxproj")

vcxproj_sources(CPPCOMPILERLIB_SOURCES "${CPPCOMPILERLIB_DIR}/CppCompilerLib.vcxproj")

vcxproj_sources(MICROPROCESSORLIB_SOURCES "${MICROPROCESSORLIB_DIR}/MicroprocessorLib.vcxproj")

vcxproj_sources(THREADLIB_SOURCES "${THREADLIB_DIR}/ThreadLib.vcxproj")

vcxproj_sources(ASSEMBLER65LIB_SOURCES "${ASSEMBLER65LIB_DIR}/Assembler65Lib.vcxproj")

vcxproj_sources(TESTCPPCOMPILERLIB_SOURCES "${TESTCPPCOMPILERLIB_DIR}/TestCppCompilerLib.vcxproj")


codaphela_options("${COMMON_DIR}" "${HCP65_LIBRARIES_DIR}")


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

add_library(CppCompilerLib STATIC ${CPPCOMPILERLIB_SOURCES})
target_link_libraries(CppCompilerLib PUBLIC Assembler65Lib MicroprocessorLib ThreadLib StandardLib BaseLib)

# CPP65Builder compiles units on worker threads (CWorkerThreadJobs).
add_library(ThreadLib STATIC ${THREADLIB_SOURCES})
target_link_libraries(ThreadLib PUBLIC BaseLib)

add_library(MicroprocessorLib STATIC ${MICROPROCESSORLIB_SOURCES})
target_link_libraries(MicroprocessorLib PUBLIC StandardLib BaseLib)

add_library(Assembler65Lib STATIC ${ASSEMBLER65LIB_SOURCES})
target_link_libraries(Assembler65Lib PUBLIC MicroprocessorLib StandardLib BaseLib)

add_executable(TestCppCompilerLib ${TESTCPPCOMPILERLIB_SOURCES})

# TestUnit*.cpp are also input to the preprocessor tests, which check line
# numbers in them, so they are left as they are.
set_source_files_properties(
	"${TESTCPPCOMPILERLIB_DIR}/TestUnit1.cpp"
	"${TESTCPPCOMPILERLIB_DIR}/TestUnit2.cpp"
	PROPERTIES COMPILE_OPTIONS -Wno-unused-but-set-variable)
target_link_libraries(TestCppCompilerLib PRIVATE TestLib CppCompilerLib Assembler65Lib MicroprocessorLib StandardLib BaseLib)

# The tests read their input (Input/ ...) relative to the project directory,
# as Visual Studio does.
enable_testing()
add_test(NAME TestCppCompilerLib COMMAND TestCppCompilerLib WORKING_DIRECTORY "${TESTCPPCOMPILERLIB_DIR}")
