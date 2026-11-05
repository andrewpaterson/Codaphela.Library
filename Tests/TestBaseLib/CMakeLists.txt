# Linux build of TestBaseLib (BaseLib + TestLib + TestBaseLib).
#
# Builds 32-bit (-m32, needs g++-multilib) by default.  Add -DBASELIB_64=ON
# for a 64-bit build, in its own build directory:
#
#   cmake -S commonlibraries/Tests/TestBaseLib -B build/TestBaseLib64 -G Ninja -DBASELIB_64=ON
#
#   cmake -S commonlibraries/Tests/TestBaseLib -B build/TestBaseLib -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/TestBaseLib
#   ctest --test-dir build/TestBaseLib --output-on-failure
#
# Add -DBASELIB_ASAN=ON for AddressSanitizer and LeakSanitizer.
#
# Add -DBASELIB_SIZE16=ON (32-bit only) to check the W65C816's size model on the host: size is 16 bits and lsize 32,
# and -Wconversion warns of each wider value narrowed to a size ("to 'size'" in the warning).
#
#   cmake -S commonlibraries/Tests/TestBaseLib -B build/TestBaseLibSize16 -G Ninja -DBASELIB_SIZE16=ON
#
# Source lists are read from the .vcxproj files so Visual Studio stays the
# source of truth.  Building re-runs the configure step automatically when a
# .vcxproj changes.  Test directories contain .cpp files that are test data
# (e.g. SDCard.cpp), so never glob *.cpp to find sources.

cmake_minimum_required(VERSION 3.16)
project(TestBaseLib CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

option(BASELIB_ASAN "Build with AddressSanitizer and LeakSanitizer" OFF)
option(BASELIB_64 "Build 64-bit instead of 32-bit" OFF)
option(BASELIB_SIZE16 "Build with a 16-bit size (the W65C816's size model) and -Wconversion" OFF)

get_filename_component(CODAPHELA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMMON_DIR "${CODAPHELA_ROOT}/commonlibraries/Libraries")
set(BASELIB_DIR "${COMMON_DIR}/BaseLib")
set(TESTLIB_DIR "${COMMON_DIR}/TestLib")
set(TESTBASELIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestBaseLib")


# Reads <ClCompile Include="..."/> entries from a .vcxproj and resolves them
# against the files on disk case-insensitively (Windows paths are not
# case-sensitive, so a project entry may differ in case from its file).  Files named in EXCLUDE (paths relative to the project,
# any case) are left out.
function(vcxproj_sources OUT_VAR VCXPROJ)
	cmake_parse_arguments(ARG "" "" "EXCLUDE" ${ARGN})
	get_filename_component(PROJ_DIR "${VCXPROJ}" DIRECTORY)

	# Re-run the configure step when Visual Studio adds or removes files.
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${VCXPROJ}")

	file(GLOB_RECURSE DISK_FILES RELATIVE "${PROJ_DIR}" "${PROJ_DIR}/*.cpp")
	foreach(DISK_FILE ${DISK_FILES})
		string(TOLOWER "${DISK_FILE}" KEY)
		set("DISK_${KEY}" "${DISK_FILE}")
	endforeach()

	set(EXCLUDED "")
	foreach(EXCLUDE_FILE ${ARG_EXCLUDE})
		string(TOLOWER "${EXCLUDE_FILE}" KEY)
		list(APPEND EXCLUDED "${KEY}")
	endforeach()

	file(STRINGS "${VCXPROJ}" LINES REGEX "<ClCompile Include=\"[^\"]+\"")
	set(SOURCES "")
	foreach(LINE ${LINES})
		string(REGEX REPLACE ".*<ClCompile Include=\"([^\"]+)\".*" "\\1" SOURCE "${LINE}")
		string(REPLACE "\\" "/" SOURCE "${SOURCE}")
		string(TOLOWER "${SOURCE}" KEY)
		if(KEY IN_LIST EXCLUDED)
			continue()
		endif()
		if(NOT DEFINED "DISK_${KEY}")
			message(FATAL_ERROR "${VCXPROJ} lists ${SOURCE} but it was not found on disk")
		endif()
		list(APPEND SOURCES "${PROJ_DIR}/${DISK_${KEY}}")
	endforeach()

	set(${OUT_VAR} ${SOURCES} PARENT_SCOPE)
endfunction()


vcxproj_sources(BASELIB_SOURCES "${BASELIB_DIR}/BaseLib.vcxproj")

vcxproj_sources(TESTLIB_SOURCES "${TESTLIB_DIR}/TestLib.vcxproj")

vcxproj_sources(TESTBASELIB_SOURCES "${TESTBASELIB_DIR}/TestBaseLib.vcxproj")


# Matches the Visual Studio configurations, except BREAK_ON_ERROR and
# BREAK_ON_WARNING: on Linux BREAK() raises SIGTRAP, which kills the test run
# when no debugger is attached.
add_library(baselib_options INTERFACE)
target_include_directories(baselib_options INTERFACE "${COMMON_DIR}")
target_compile_definitions(baselib_options INTERFACE
	LINUX_GNU
	$<$<CONFIG:Debug>:_DEBUG>
	$<$<CONFIG:Debug>:DEBUG>
	$<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)
# -m32 -msse2 -mfpmath=sse: MSVC uses SSE2 for 32-bit code.  The x87 FPU keeps float
# constants at long double precision, so float comparisons would differ.
# -D_FILE_OFFSET_BITS=64: a 32-bit stat() fails (EOVERFLOW) on a file whose inode number needs 64 bits, as every
# file on a Windows drive under WSL (/mnt/d) has, so files there would seem not to exist.
# -fno-strict-aliasing: MSVC never optimises on the strict aliasing rule and the
# code relies on that (e.g. (CUnknown**)&pcTyped), so optimised builds break without it.
# -Wno-ignored-attributes: g++ drops ALIGN_64 when int64/uint64/float64 is a
# template argument.  Templates that store such a type by value use MSVC_ALIGN
# instead (see PrimitiveTypes.h), so the warning only reports the drop.
# -Wno-write-strings: string literals are passed as char* throughout; MSVC allows it.
# -Wno-multichar: chunk and file IDs are multi-character constants like 'ABCD'.
# -Wno-class-memaccess: the containers memcpy objects with constructors on purpose.
# -Wno-overloaded-virtual: subclasses overload virtuals with other argument types
# (e.g. Save(CObjectWriter*) hides CUnknown::Save(CFileWriter*)) on purpose.
# -Wno-comment: ASCII diagrams in comments end lines with a backslash.
# -Wno-unused-parameter: virtual overrides and callbacks often ignore parameters.
if(BASELIB_64)
	set(BASELIB_ARCH_OPTIONS -m64)
else()
	set(BASELIB_ARCH_OPTIONS -m32 -msse2 -mfpmath=sse -D_FILE_OFFSET_BITS=64)
endif()
target_compile_options(baselib_options INTERFACE ${BASELIB_ARCH_OPTIONS} -fno-strict-aliasing -Wall -Wextra -Wno-ignored-attributes -Wno-write-strings -Wno-multichar -Wno-class-memaccess -Wno-overloaded-virtual -Wno-comment -Wno-unused-parameter)
target_link_options(baselib_options INTERFACE $<IF:$<BOOL:${BASELIB_64}>,-m64,-m32>)
# The Visual Studio configurations all use /arch:AVX2, which lets MSVC use AVX2, FMA, BMI, LZCNT and POPCNT
# instructions and defines __AVX2__ (PNGWriter.cpp has an AVX2 path).  -march=haswell is the same set.  Floating
# point results don't change: with CMAKE_CXX_EXTENSIONS OFF g++ doesn't contract a * b + c into FMA instructions.
target_compile_options(baselib_options INTERFACE -march=haswell)

# Release options matching the Visual Studio Release configurations.  -O3 (from CMAKE_BUILD_TYPE) covers /O2 /Ob2 /Oi.
# -ffunction-sections -fdata-sections and --gc-sections: /Gy and /OPT:REF, so unused functions are dropped.
# -fno-stack-protector: /GS-.  Many Linux distributions turn the stack protector on by default.
# -U_FORTIFY_SOURCE: MSVC has no equivalent, and many Linux distributions turn it on by default.  Fortified
# strcpy aborts on names stored past the end of their object (CField::Init) once LTO can see the object size.
# -fno-lifetime-dse: MSVC keeps stores made in a destructor.  CObject::~CObject frees embedded objects (CMapObject)
# after their own destructors have run, so g++ dropping those stores as dead crashes the second Kill.
# -fno-math-errno -fno-trapping-math: the parts of /fp:fast that don't change results (sqrt can be inlined, for
# example).  -ffast-math would go further than /fp:fast and assume there are no NaNs or infinities.
# /GL and /LTCG (whole program optimisation) are -flto, turned on by CMAKE_INTERPROCEDURAL_OPTIMIZATION below.
# /Gv (vectorcall) has no g++ equivalent: 64-bit Linux already passes vectors in registers, and changing the
# 32-bit calling convention would break calls into libc.
target_compile_options(baselib_options INTERFACE "$<$<NOT:$<CONFIG:Debug>>:-ffunction-sections;-fdata-sections;-fno-stack-protector;-U_FORTIFY_SOURCE;-fno-lifetime-dse;-fno-math-errno;-fno-trapping-math>")
target_link_options(baselib_options INTERFACE "$<$<NOT:$<CONFIG:Debug>>:-Wl,--gc-sections>")
include(CheckIPOSupported)
check_ipo_supported(RESULT BASELIB_IPO_SUPPORTED LANGUAGES CXX)
if(BASELIB_IPO_SUPPORTED)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
endif()

if(BASELIB_SIZE16)
	if(BASELIB_64)
		message(FATAL_ERROR "BASELIB_SIZE16 is for 32-bit builds.")
	endif()
	target_compile_definitions(baselib_options INTERFACE BASELIB_SIZE16)
	target_compile_options(baselib_options INTERFACE -Wconversion)
endif()

if(BASELIB_ASAN)
	target_compile_options(baselib_options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
	target_link_options(baselib_options INTERFACE -fsanitize=address)
endif()


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
