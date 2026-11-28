#!/usr/bin/env python3
"""Compares what the HCP65 IDE's GCC build and a CMake build do for one solution (HCP65-IDE's ConfigurationsPlan.txt,
C13-6): both read the solution's rows and the projects, so they should compile the same files the same way and link
the same libraries.

    python3 compare_builds.py <solution.sln> <configuration> <CMake build folder> <Project=Target[:Configuration]>...

Build both first: the IDE's build with hcp65build (<solution> -config "<configuration>|GCC-x64", and Reference|GCC-x64
for a reference program), which leaves each command beside what it made (Output/GCC-x64/<configuration>/**/*.cmd);
CMake's with the same build type, which writes compile_commands.json and each program's link.txt.

It compares, and says each difference:
- The sources compiled: those of the libraries and of the programs named (the IDE's build also builds the solution's
  other programs, its tests, which a CMakeLists.txt doesn't name).
- Each source's defines and options.  CMake's own are left out: -Wall -Wextra, link time optimisation, and what its
  build type adds that the projects' sheet already says (-O3, -g).
- Each source's include directories: every one the IDE's build has, CMake's must have.  CMake gives every target the
  directories its CMakeLists.txt names, so it may have more.
- Each named program's link: the libraries (CMake's <Project>Lib, what a program's two configurations share, is the
  program's own objects in the IDE's build), the system libraries and the options.
Exits 1 if anything differs.
"""
import json
import os
import shlex
import sys

PLATFORM = "GCC-x64"
CMAKE_OWN = {"-Wall", "-Wextra", "-flto", "-flto=auto", "-fno-fat-lto-objects", "-ffat-lto-objects", "-fdiagnostics-color=always"}
BUILD_TYPE = {"-O3", "-O2", "-O0", "-g", "-DNDEBUG"}
SKIPPED = {"-c", "-MMD", "-MD", "-MP", "-Wl,--start-group", "-Wl,--end-group"}
WITH_VALUE = {"-o", "-MF", "-MT", "-MQ", "-isystem"}


def read_command(path):
	"""A command the IDE's build recorded: an argument on each line, the last its working directory."""
	lines = open(path, encoding="utf-8").read().split("\n")
	while lines and lines[-1] == "":
		lines.pop()
	return lines[:-1]


def split(arguments):
	"""A compiler command's sources, defines, include directories, options, libraries and system libraries."""
	parts = {"sources": [], "defines": set(), "includes": set(), "options": set(), "libraries": set(), "system": set()}
	at = 1
	while at < len(arguments):
		argument = arguments[at]
		at += 1
		if argument in WITH_VALUE:
			at += 1
		elif argument in SKIPPED or argument in CMAKE_OWN:
			pass
		elif argument.startswith("-I"):
			# A directory that isn't there has nothing to include (a macro nothing sets: $(VULKAN_SDK)\\Include).
			if os.path.isdir(argument[2:]):
				parts["includes"].add(os.path.realpath(argument[2:]))
		elif argument.startswith("-D") or argument.startswith("-U"):
			parts["defines"].add(argument)
		elif argument.startswith("-l"):
			parts["system"].add(argument)
		elif argument.startswith("-"):
			parts["options"].add(argument)
		elif argument.endswith(".a"):
			parts["libraries"].add(os.path.basename(argument))
		elif argument.endswith((".cpp", ".c")):
			parts["sources"].append(os.path.realpath(argument))
	return parts


def ide_build(solution, configurations):
	"""The IDE's build's compiles by source, with the project each is of, its libraries and its links by project."""
	compiles = {}
	libraries = set()
	links = {}
	for configuration in configurations:
		folder = os.path.join(os.path.dirname(os.path.abspath(solution)), "Output", PLATFORM, configuration)
		if not os.path.isdir(folder):
			sys.exit("%s isn't there: build it with hcp65build first" % folder)
		for root, _, names in os.walk(folder):
			for name in names:
				if not name.endswith(".cmd"):
					continue
				arguments = read_command(os.path.join(root, name))
				if name.endswith(".o.cmd"):
					parts = split(arguments)
					for source in parts["sources"]:
						compiles[source] = (os.path.basename(root), parts)
				elif name.endswith(".a.cmd"):
					libraries.add(name[3:-6])
				else:
					links[(name[:-4], configuration)] = split(arguments)
	return compiles, libraries, links


def cmake_build(folder):
	path = os.path.join(folder, "compile_commands.json")
	if not os.path.isfile(path):
		sys.exit("%s isn't there: configure the CMake build first" % path)
	compiles = {}
	for entry in json.load(open(path, encoding="utf-8")):
		arguments = shlex.split(entry["command"]) if "command" in entry else entry["arguments"]
		compiles[os.path.realpath(entry["file"])] = split(arguments)
	return compiles


def cmake_link(folder, target):
	path = os.path.join(folder, "CMakeFiles", target + ".dir", "link.txt")
	if not os.path.isfile(path):
		sys.exit("%s isn't there" % path)
	return split(shlex.split(open(path, encoding="utf-8").read()))


def main(arguments):
	if len(arguments) < 4:
		print(__doc__)
		return 1
	solution, configuration, folder = arguments[0], arguments[1], arguments[2]
	programs = []
	for pair in arguments[3:]:
		project, _, target = pair.partition("=")
		target, _, built_as = target.partition(":")
		programs.append((project, target or project, built_as or configuration))
	configurations = [configuration] + sorted({built_as for _, _, built_as in programs if built_as != configuration})

	ide_compiles, ide_libraries, ide_links = ide_build(solution, configurations)
	cmake_compiles = cmake_build(folder)
	wanted = ide_libraries | {project for project, _, _ in programs}
	differences = []

	for source, (project, parts) in sorted(ide_compiles.items()):
		if project not in wanted:
			continue
		if source not in cmake_compiles:
			differences.append("%s (%s): only the IDE's build compiles it" % (source, project))
			continue
		other = cmake_compiles[source]
		for kind in ("defines", "options"):
			if parts[kind] != other[kind]:
				differences.append("%s: %s differ: the IDE's alone %s, CMake's alone %s" % (source, kind, sorted(parts[kind] - other[kind]), sorted(other[kind] - parts[kind])))
		if not parts["includes"] <= other["includes"]:
			differences.append("%s: CMake's build hasn't the include directories %s" % (source, sorted(parts["includes"] - other["includes"])))
	ide_sources = {source for source, (project, _) in ide_compiles.items() if project in wanted}
	for source in sorted(set(cmake_compiles) - ide_sources):
		differences.append("%s: only CMake's build compiles it" % source)

	for project, target, built_as in programs:
		if (project, built_as) not in ide_links:
			differences.append("%s: the IDE's build didn't link it in %s" % (project, built_as))
			continue
		ide = ide_links[(project, built_as)]
		cmake = cmake_link(folder, target)
		shared = "lib%sLib.a" % project		# or <Project>Shared, where a library of the solution has that name
		cmake["libraries"].discard("lib%sShared.a" % project if shared in ide["libraries"] else shared)
		cmake["options"] -= BUILD_TYPE		# CMake links with its build type's compile flags too.
		cmake["defines"] -= BUILD_TYPE
		for kind in ("libraries", "system", "options"):
			if ide[kind] != cmake[kind]:
				differences.append("%s as %s: link %s differ: the IDE's alone %s, CMake's alone %s" % (target, built_as, kind, sorted(ide[kind] - cmake[kind]), sorted(cmake[kind] - ide[kind])))

	compared = len([1 for _, (project, _) in ide_compiles.items() if project in wanted])
	print("%d sources and %d links compared, %d differences" % (compared, len(programs), len(differences)))
	for difference in differences:
		print(difference)
	return 1 if differences else 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
