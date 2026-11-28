# The test of codaphela_solution (CommonLibraries/CMake/Codaphela.cmake; HCP65-IDE's ConfigurationsPlan.txt, C13-6):
# a solution's targets made from its rows.  Configuring checks the targets and their sources; ctest runs the programs.
#
#   cmake -S CommonLibraries/CMake/Tests/Solution -B build/TestCodaphelaSolution -DCMAKE_BUILD_TYPE=Release
#   cmake --build build/TestCodaphelaSolution && ctest --test-dir build/TestCodaphelaSolution
#
# Solution.sln has, on GCC-x64, Debug, Release and Reference:
# - Program, built as its own Reference in Reference and as Debug or Release otherwise.  Program_Linux.cpp is kept out
#   of Reference|GCC-x64 and ProgramRef.cpp out of the others, so Both.cpp is what the two compile alike, and
#   Program_Windows.cpp is kept out of the platform.  It links m (its Link group's AdditionalDependencies).
# - Shared, a library every configuration builds; WindowOnly, one Reference's rows don't build; ReferenceOnly, one only
#   Reference's rows build.  WindowOnly and ReferenceOnly define the same function: a program linked with the wrong
#   one says so, and one linked with both doesn't link.
# - Tool, a program named here under another target name; Unnamed, a program not named here, and Command, a project
#   built by its own command: neither is a target, and neither would compile.
# - Tool references Shared, so named again with REFERENCED (as a test, under a third name) it is linked with Shared
#   alone.  Shared defines SHARED_DEBUG in its Debug configuration and SHARED_NOT_DEBUG in the others: its own defines.
cmake_minimum_required(VERSION 3.24)
project(TestCodaphelaSolution CXX)

get_filename_component(COMMON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
include("${COMMON_ROOT}/CMake/Codaphela.cmake")

codaphela_options(ONLY_64)
codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/Solution.sln"
	PROGRAMS Program Tool=tool
	REFERENCE Program=ProgramRef)


# A target's sources, as their names.
function(assert_sources TARGET_NAME)
	get_target_property(SOURCES ${TARGET_NAME} SOURCES)
	set(NAMES "")
	foreach(SOURCE ${SOURCES})
		get_filename_component(NAME "${SOURCE}" NAME)
		list(APPEND NAMES "${NAME}")
	endforeach()
	if(NOT "${NAMES}" STREQUAL "${ARGN}")
		message(FATAL_ERROR "${TARGET_NAME} has the sources [${NAMES}], not [${ARGN}]")
	endif()
endfunction()

foreach(EXPECTED Program ProgramRef ProgramLib tool Shared WindowOnly ReferenceOnly)
	if(NOT TARGET ${EXPECTED})
		message(FATAL_ERROR "codaphela_solution made no target ${EXPECTED}")
	endif()
endforeach()
foreach(UNEXPECTED Tool Unnamed Command)
	if(TARGET ${UNEXPECTED})
		message(FATAL_ERROR "codaphela_solution made a target ${UNEXPECTED}")
	endif()
endforeach()

# What the two configurations of the program compile alike is the library's; each program has its own main.
assert_sources(ProgramLib Both.cpp)
assert_sources(Program Program_Linux.cpp)
assert_sources(ProgramRef ProgramRef.cpp)
assert_sources(tool Tool.cpp)
assert_sources(Shared Shared.cpp)

# Each program is linked with the libraries its own configuration's rows build.
add_test(NAME Program COMMAND Program)
set_tests_properties(Program PROPERTIES PASS_REGULAR_EXPRESSION "^window main, window library, shared\n$")
add_test(NAME ProgramRef COMMAND ProgramRef)
set_tests_properties(ProgramRef PROPERTIES PASS_REGULAR_EXPRESSION "^reference main, reference library, shared\n$")
add_test(NAME tool COMMAND tool)
set_tests_properties(tool PROPERTIES PASS_REGULAR_EXPRESSION "^tool, shared\n$")


# A program named with REFERENCED is linked with the libraries its project references alone, and one of TESTS is a
# test, run in its project's folder.
codaphela_solution("${CMAKE_CURRENT_SOURCE_DIR}/Solution.sln" TESTS Tool=referenced REFERENCED)
get_target_property(LINKED referenced LINK_LIBRARIES)
if(NOT "${LINKED}" MATCHES "RESCAN,Shared>")
	message(FATAL_ERROR "referenced is linked with [${LINKED}], not with Shared alone")
endif()
get_property(REFERENCED_FOLDER TEST referenced PROPERTY WORKING_DIRECTORY)
if(NOT REFERENCED_FOLDER STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/Tool")
	message(FATAL_ERROR "referenced runs in [${REFERENCED_FOLDER}], not in its project's folder")
endif()
set_tests_properties(referenced PROPERTIES PASS_REGULAR_EXPRESSION "^tool, shared\n$")

# A library has its project's own defines for the configuration it is built as.
get_target_property(OWN_DEFINES Shared COMPILE_DEFINITIONS)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(EXPECTED_DEFINES "SHARED_DEBUG")
else()
	set(EXPECTED_DEFINES "SHARED_NOT_DEBUG")
endif()
if(NOT "${OWN_DEFINES}" STREQUAL "${EXPECTED_DEFINES}")
	message(FATAL_ERROR "Shared has the defines [${OWN_DEFINES}], not [${EXPECTED_DEFINES}]")
endif()
