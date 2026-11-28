# What every Codaphela CMake build shares.  include() it after project(), with COMMON_DIR CommonLibraries' Libraries:
#
#	include("${COMMON_DIR}/../CMake/Codaphela.cmake")
#	codaphela_options("${COMMON_DIR}")						# baselib_options, which every target links
#	vcxproj_sources(BASELIB_SOURCES "${BASELIB_DIR}/BaseLib.vcxproj")
#
# The builds read their sources from the Visual Studio projects and their g++ options from the projects' property
# sheet, CommonLibraries/Common.props, so each is written once, for Visual Studio, the HCP65 IDE and CMake alike
# (HCP65-IDE's ConfigurationsPlan.txt, C6).  A CMakeLists.txt keeps its targets.

include_guard(GLOBAL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)		# compile_commands.json, which compare_builds.py reads
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

option(BASELIB_64 "Build 64-bit (otherwise 32-bit, which needs g++-multilib)" OFF)
option(BASELIB_ASAN "Build with AddressSanitizer and LeakSanitizer" OFF)
option(BASELIB_TSAN "Build with ThreadSanitizer (64-bit; run it under setarch -R)" OFF)
option(BASELIB_SIZE16 "32-bit only: size is 16 bits and lsize 32, as on the W65C816, with -Wconversion" OFF)
option(BASELIB_MALLOC "gcSystemAllocator uses malloc, whose blocks valgrind knows" OFF)


# baselib_options: an interface library holding what every target is compiled and linked with.  The include
# directories are the arguments.  ONLY_64 is for a program that is never built 32-bit (it needs X11).
#
# From Common.props' GCC-x64 groups (project_sheets.py --cmake reads them): the defines, which are Visual Studio's for
# the configuration and LINUX_GNU, and g++'s options, which the sheet and TestBaseLib's history explain.  A Debug
# build's defines include BREAK_ON_ERROR and BREAK_ON_WARNING, so ctest runs every test with BASELIB_BREAK=0: a run
# gets through its failures to its totals, and a test run by hand stops at the first.
#
# CMake's own, here once:
# - 32-bit: -m32 -msse2 -mfpmath=sse in place of the sheet's -m64 (MSVC uses SSE2 for 32-bit code; the x87 FPU keeps
#   float constants at long double precision, so float comparisons would differ), and -D_FILE_OFFSET_BITS=64 (a 32-bit
#   stat() fails on a file whose inode number needs 64 bits, as every file on a Windows drive under WSL has).
# - -Wall -Wextra: the warnings a build here is kept clean of.
# - Link time optimisation in Release, as /GL and /LTCG.
# - The switches above.
function(codaphela_options)
	cmake_parse_arguments(ARG "ONLY_64" "" "" ${ARGN})
	if(ARG_ONLY_64)
		set(BASELIB_64 ON)
	endif()

	find_package(Python3 REQUIRED COMPONENTS Interpreter)
	set(SHEET "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../Common.props")
	set(READER "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../project_sheets.py")
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SHEET}" "${READER}")
	execute_process(COMMAND "${Python3_EXECUTABLE}" -I "${READER}" --cmake "${SHEET}" OUTPUT_FILE "${CMAKE_BINARY_DIR}/Sheet.cmake" RESULT_VARIABLE SHEET_RESULT)
	if(NOT SHEET_RESULT EQUAL 0)
		message(FATAL_ERROR "Could not read ${SHEET}")
	endif()
	include("${CMAKE_BINARY_DIR}/Sheet.cmake")

	add_library(baselib_options INTERFACE)
	target_include_directories(baselib_options INTERFACE ${ARG_UNPARSED_ARGUMENTS})
	foreach(KIND DEFINES OPTIONS LINK_OPTIONS)
		if(NOT BASELIB_64)
			list(TRANSFORM SHEET_${KIND}_DEBUG REPLACE "^-m64$" "-m32")
			list(TRANSFORM SHEET_${KIND}_RELEASE REPLACE "^-m64$" "-m32")
		endif()
	endforeach()
	target_compile_definitions(baselib_options INTERFACE "$<$<CONFIG:Debug>:${SHEET_DEFINES_DEBUG}>" "$<$<NOT:$<CONFIG:Debug>>:${SHEET_DEFINES_RELEASE}>")
	target_compile_options(baselib_options INTERFACE "$<$<CONFIG:Debug>:${SHEET_OPTIONS_DEBUG}>" "$<$<NOT:$<CONFIG:Debug>>:${SHEET_OPTIONS_RELEASE}>" -Wall -Wextra)
	target_link_options(baselib_options INTERFACE "$<$<CONFIG:Debug>:${SHEET_LINK_OPTIONS_DEBUG}>" "$<$<NOT:$<CONFIG:Debug>>:${SHEET_LINK_OPTIONS_RELEASE}>")
	if(NOT BASELIB_64)
		target_compile_options(baselib_options INTERFACE -msse2 -mfpmath=sse -D_FILE_OFFSET_BITS=64)
	endif()

	include(CheckIPOSupported)
	check_ipo_supported(RESULT BASELIB_IPO_SUPPORTED LANGUAGES CXX)
	if(BASELIB_IPO_SUPPORTED)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON PARENT_SCOPE)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON PARENT_SCOPE)
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
	if(BASELIB_MALLOC)
		target_compile_definitions(baselib_options INTERFACE SYSTEM_ALLOCATOR_USES_MALLOC)
	endif()
	if(BASELIB_TSAN)
		target_compile_options(baselib_options INTERFACE -fsanitize=thread -fno-omit-frame-pointer)
		target_link_options(baselib_options INTERFACE -fsanitize=thread)
	endif()

	cmake_language(DEFER CALL codaphela_tests_dont_break)
endfunction()


# Every test of the directory runs with BASELIB_BREAK=0 (see codaphela_options).
function(codaphela_tests_dont_break)
	get_property(DIRECTORY_TESTS DIRECTORY PROPERTY TESTS)
	foreach(DIRECTORY_TEST ${DIRECTORY_TESTS})
		set_property(TEST ${DIRECTORY_TEST} APPEND PROPERTY ENVIRONMENT "BASELIB_BREAK=0")
	endforeach()
endfunction()


# Reads <ClCompile Include="..."/> entries from a .vcxproj and resolves them
# against the files on disk case-insensitively (Windows paths are not
# case-sensitive).  A file the project excludes from the configuration's
# build (ExcludedFromBuild, for GCC-x64 or for the configuration on it) is
# left out, and so are the files named in EXCLUDE (paths relative to the
# project, any case).  CONFIGURATION is the project's configuration read
# (Reference, of Reference|GCC-x64); without it, Debug or Release as the
# build is.
function(vcxproj_sources OUT_VAR VCXPROJ)
	cmake_parse_arguments(ARG "" "CONFIGURATION" "EXCLUDE" ${ARGN})
	get_filename_component(PROJ_DIR "${VCXPROJ}" DIRECTORY)
	if(NOT ARG_CONFIGURATION)
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			set(ARG_CONFIGURATION "Debug")
		else()
			set(ARG_CONFIGURATION "Release")
		endif()
	endif()

	# Re-run the configure step when Visual Studio adds or removes files.
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${VCXPROJ}")

	file(GLOB_RECURSE DISK_FILES RELATIVE "${PROJ_DIR}" "${PROJ_DIR}/*.cpp" "${PROJ_DIR}/*.c")
	foreach(DISK_FILE ${DISK_FILES})
		string(TOLOWER "${DISK_FILE}" KEY)
		set("DISK_${KEY}" "${DISK_FILE}")
	endforeach()

	set(EXCLUDED "")
	foreach(EXCLUDE_FILE ${ARG_EXCLUDE})
		string(TOLOWER "${EXCLUDE_FILE}" KEY)
		list(APPEND EXCLUDED "${KEY}")
	endforeach()

	# An item's line, then the lines of its exclusions: one that is this configuration's takes the item out again.
	file(STRINGS "${VCXPROJ}" LINES REGEX "<ClCompile Include=\"[^\"]+\"|<ExcludedFromBuild Condition=\"[^\"]+\">true<")
	set(SOURCES "")
	set(ITEM "")
	foreach(LINE ${LINES})
		if(LINE MATCHES "<ExcludedFromBuild Condition=\"'\\$\\(([A-Za-z|$()]+)\\)'=='([^']*)'\"")
			if((CMAKE_MATCH_1 STREQUAL "Platform" AND CMAKE_MATCH_2 STREQUAL "GCC-x64")
			   OR (CMAKE_MATCH_1 STREQUAL "Configuration" AND CMAKE_MATCH_2 STREQUAL "${ARG_CONFIGURATION}")
			   OR (CMAKE_MATCH_1 STREQUAL "Configuration)|$(Platform" AND CMAKE_MATCH_2 STREQUAL "${ARG_CONFIGURATION}|GCC-x64"))
				list(REMOVE_ITEM SOURCES "${ITEM}")
			endif()
			continue()
		endif()
		set(ITEM "")
		string(REGEX REPLACE ".*<ClCompile Include=\"([^\"]+)\".*" "\\1" SOURCE "${LINE}")
		string(REPLACE "\\" "/" SOURCE "${SOURCE}")
		string(TOLOWER "${SOURCE}" KEY)
		if(KEY IN_LIST EXCLUDED)
			continue()
		endif()
		# A file of another project (..\..\TestBaseLib65\TestTarget65.cpp) is used where it is.
		if(SOURCE MATCHES "^\\.\\./")
			if(NOT EXISTS "${PROJ_DIR}/${SOURCE}")
				message(FATAL_ERROR "${VCXPROJ} lists ${SOURCE} but it was not found on disk")
			endif()
			get_filename_component(ITEM "${PROJ_DIR}/${SOURCE}" ABSOLUTE)
		else()
			if(NOT DEFINED "DISK_${KEY}")
				message(FATAL_ERROR "${VCXPROJ} lists ${SOURCE} but it was not found on disk")
			endif()
			set(ITEM "${PROJ_DIR}/${DISK_${KEY}}")
		endif()
		list(APPEND SOURCES "${ITEM}")
	endforeach()

	set(${OUT_VAR} ${SOURCES} PARENT_SCOPE)
endfunction()


# What two configurations of a project compile alike, and what each compiles alone: a program built two ways (with
# its window, and as its reference program) is a library of the first and a program of each of the others, so no
# CMakeLists.txt names a main (HCP65-IDE's ConfigurationsPlan.txt, C13-2).
#	vcxproj_sources(PROGRAM "${DIR}/HCP65IDE.vcxproj")
#	vcxproj_sources(REFERENCE "${DIR}/HCP65IDE.vcxproj" CONFIGURATION Reference)
#	codaphela_shared_sources(SHARED PROGRAM_OWN REFERENCE_OWN "${PROGRAM}" "${REFERENCE}")
function(codaphela_shared_sources OUT_SHARED OUT_FIRST OUT_SECOND FIRST SECOND)
	set(SHARED "")
	set(FIRST_OWN "")
	foreach(SOURCE ${FIRST})
		if(SOURCE IN_LIST SECOND)
			list(APPEND SHARED "${SOURCE}")
		else()
			list(APPEND FIRST_OWN "${SOURCE}")
		endif()
	endforeach()
	set(SECOND_OWN ${SECOND})
	list(REMOVE_ITEM SECOND_OWN ${SHARED})
	set(${OUT_SHARED} ${SHARED} PARENT_SCOPE)
	set(${OUT_FIRST} ${FIRST_OWN} PARENT_SCOPE)
	set(${OUT_SECOND} ${SECOND_OWN} PARENT_SCOPE)
endfunction()


# The targets of a solution, read from its rows as the HCP65 IDE's GCC build reads them (HCP65-IDE's
# ConfigurationsPlan.txt, C13-3; solution_projects.py): the build's configuration on GCC-x64, Debug|GCC-x64 or
# Release|GCC-x64, says which projects are built and as which of their own configurations.
#
#	codaphela_solution("${DIR}/HCP65IDE.sln"
#		PROGRAMS HCP65IDE HCP65Build=hcp65build
#		REFERENCE HCP65IDE=HCP65IDERef)
#
# - Each static library the configuration builds is a library target of the project's name, its sources the project's
#   (vcxproj_sources).
# - Each of PROGRAMS, a project or Project=Target, is a program: linked with every library the configuration builds,
#   in any order, and its Link group's AdditionalDependencies for GCC-x64 (X11 as -lX11), as the IDE's build links it.
#   The other applications a solution has (its tests) are no targets unless named.
# - Each of REFERENCE, Project=Target, is the project again as Reference|GCC-x64 builds it: its reference main in
#   place of its window's, the libraries that configuration builds (WinRefLib, and no X11Lib).  What the two
#   configurations compile alike is compiled once, as the library <Project>Lib (codaphela_shared_sources).
# A program's own folder is one of its include directories.  codaphela_options comes first: every target links
# baselib_options.
function(codaphela_solution SOLUTION)
	cmake_parse_arguments(ARG "" "" "PROGRAMS;REFERENCE" ${ARGN})
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CONFIGURATION "Debug")
	else()
		set(CONFIGURATION "Release")
	endif()

	find_package(Python3 REQUIRED COMPONENTS Interpreter)
	set(READER "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../solution_projects.py")
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SOLUTION}" "${READER}")
	get_filename_component(SOLUTION_NAME "${SOLUTION}" NAME_WE)
	execute_process(COMMAND "${Python3_EXECUTABLE}" -I "${READER}" --cmake "${SOLUTION}" ${CONFIGURATION} Reference OUTPUT_FILE "${CMAKE_BINARY_DIR}/${SOLUTION_NAME}.solution.cmake" RESULT_VARIABLE SOLUTION_RESULT)
	if(NOT SOLUTION_RESULT EQUAL 0)
		message(FATAL_ERROR "Could not read ${SOLUTION}")
	endif()
	include("${CMAKE_BINARY_DIR}/${SOLUTION_NAME}.solution.cmake")
	if(NOT CONFIGURATION IN_LIST SOLUTION_CONFIGURATIONS)
		message(FATAL_ERROR "${SOLUTION} has no ${CONFIGURATION}|GCC-x64")
	endif()
	if(ARG_REFERENCE AND NOT "Reference" IN_LIST SOLUTION_CONFIGURATIONS)
		message(FATAL_ERROR "${SOLUTION} has no Reference|GCC-x64")
	endif()

	# The libraries: those of the build's configuration and, with a reference program, of Reference's.  Reference's
	# rows build a library as its Release, which here is as the build is: a Debug build has one set of libraries,
	# Debug's, for both its programs.
	set(LIBRARIES ${SOLUTION_${CONFIGURATION}_LIBRARIES})
	foreach(LIBRARY ${LIBRARIES})
		set(BUILT_AS_${LIBRARY} "${SOLUTION_${CONFIGURATION}_${LIBRARY}_AS}")
	endforeach()
	if(ARG_REFERENCE)
		foreach(LIBRARY ${SOLUTION_Reference_LIBRARIES})
			set(REFERENCE_AS "${SOLUTION_Reference_${LIBRARY}_AS}")
			if(REFERENCE_AS STREQUAL "Release")
				set(REFERENCE_AS "${CONFIGURATION}")
			endif()
			if(NOT LIBRARY IN_LIST LIBRARIES)
				list(APPEND LIBRARIES "${LIBRARY}")
				set(BUILT_AS_${LIBRARY} "${REFERENCE_AS}")
			elseif(NOT REFERENCE_AS STREQUAL BUILT_AS_${LIBRARY})
				message(FATAL_ERROR "${SOLUTION} builds ${LIBRARY} as ${REFERENCE_AS} in Reference|GCC-x64 and as ${BUILT_AS_${LIBRARY}} in ${CONFIGURATION}|GCC-x64: one target can't be both")
			endif()
		endforeach()
	endif()
	foreach(LIBRARY ${LIBRARIES})
		if(TARGET ${LIBRARY})
			continue()
		endif()
		vcxproj_sources(LIBRARY_SOURCES "${SOLUTION_${LIBRARY}_FILE}" CONFIGURATION "${BUILT_AS_${LIBRARY}}")
		if(LIBRARY_SOURCES)
			add_library(${LIBRARY} STATIC ${LIBRARY_SOURCES})
			target_link_libraries(${LIBRARY} PUBLIC baselib_options)
		endif()
	endforeach()

	# The programs: Project or Project=Target.
	set(REFERENCE_PROJECTS "")
	foreach(PAIR ${ARG_REFERENCE})
		string(REGEX MATCH "^([^=]+)=(.+)$" MATCHED "${PAIR}")
		if(NOT MATCHED)
			message(FATAL_ERROR "REFERENCE takes Project=Target, not ${PAIR}")
		endif()
		list(APPEND REFERENCE_PROJECTS "${CMAKE_MATCH_1}")
		set(REFERENCE_TARGET_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
	endforeach()
	foreach(PAIR ${ARG_PROGRAMS})
		if(PAIR MATCHES "^([^=]+)=(.+)$")
			set(PROJECT_NAME "${CMAKE_MATCH_1}")
			set(TARGET_NAME "${CMAKE_MATCH_2}")
		else()
			set(PROJECT_NAME "${PAIR}")
			set(TARGET_NAME "${PAIR}")
		endif()
		if(NOT PROJECT_NAME IN_LIST SOLUTION_${CONFIGURATION}_PROGRAMS)
			message(FATAL_ERROR "${SOLUTION} doesn't build a program ${PROJECT_NAME} in ${CONFIGURATION}|GCC-x64")
		endif()
		get_filename_component(PROGRAM_DIR "${SOLUTION_${PROJECT_NAME}_FILE}" DIRECTORY)
		vcxproj_sources(PROGRAM_SOURCES "${SOLUTION_${PROJECT_NAME}_FILE}" CONFIGURATION "${SOLUTION_${CONFIGURATION}_${PROJECT_NAME}_AS}")
		if(PROJECT_NAME IN_LIST REFERENCE_PROJECTS)
			if(NOT PROJECT_NAME IN_LIST SOLUTION_Reference_PROGRAMS)
				message(FATAL_ERROR "${SOLUTION} doesn't build a program ${PROJECT_NAME} in Reference|GCC-x64")
			endif()
			vcxproj_sources(REFERENCE_SOURCES "${SOLUTION_${PROJECT_NAME}_FILE}" CONFIGURATION "${SOLUTION_Reference_${PROJECT_NAME}_AS}")
			codaphela_shared_sources(SHARED_SOURCES PROGRAM_OWN REFERENCE_OWN "${PROGRAM_SOURCES}" "${REFERENCE_SOURCES}")
			add_library(${PROJECT_NAME}Lib STATIC ${SHARED_SOURCES})
			target_include_directories(${PROJECT_NAME}Lib PUBLIC "${PROGRAM_DIR}")
			target_link_libraries(${PROJECT_NAME}Lib PUBLIC baselib_options)
			add_executable(${TARGET_NAME} ${PROGRAM_OWN})
			codaphela_link_program(${TARGET_NAME} "${PROJECT_NAME}Lib;${SOLUTION_${CONFIGURATION}_LIBRARIES}" "${SOLUTION_${CONFIGURATION}_${PROJECT_NAME}_LINKS}")
			add_executable(${REFERENCE_TARGET_${PROJECT_NAME}} ${REFERENCE_OWN})
			codaphela_link_program(${REFERENCE_TARGET_${PROJECT_NAME}} "${PROJECT_NAME}Lib;${SOLUTION_Reference_LIBRARIES}" "${SOLUTION_Reference_${PROJECT_NAME}_LINKS}")
		else()
			add_executable(${TARGET_NAME} ${PROGRAM_SOURCES})
			target_include_directories(${TARGET_NAME} PRIVATE "${PROGRAM_DIR}")
			codaphela_link_program(${TARGET_NAME} "${SOLUTION_${CONFIGURATION}_LIBRARIES}" "${SOLUTION_${CONFIGURATION}_${PROJECT_NAME}_LINKS}")
		endif()
	endforeach()
endfunction()


# A program linked as the IDE's GCC build links one: with every library the configuration builds, in any order (they
# are scanned again until nothing is missing), and its system libraries, a plain name as -l<name>.  A library with no
# sources is no target and is left out.
function(codaphela_link_program PROGRAM LIBRARIES LINKS)
	set(TARGETS "")
	foreach(LIBRARY ${LIBRARIES})
		if(TARGET ${LIBRARY})
			list(APPEND TARGETS "${LIBRARY}")
		endif()
	endforeach()
	list(JOIN TARGETS "," GROUP)
	target_link_libraries(${PROGRAM} PRIVATE baselib_options "$<LINK_GROUP:RESCAN,${GROUP}>" ${LINKS})
endfunction()
