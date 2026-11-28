#!/usr/bin/env python3
"""Says what a solution's GCC-x64 configurations build, for CMake (HCP65-IDE's ConfigurationsPlan.txt, C13-3).

    python3 solution_projects.py --cmake <solution.sln> <configuration>...

For each configuration named (Release, Reference) that the solution has on GCC-x64 it reads the solution's rows as
the HCP65 IDE's GCC build does: a project is built if its row has a .Build.0, as the project configuration the row
names, on GCC-x64.  It prints CMake that sets, with <C> the configuration's name:

    SOLUTION_CONFIGURATIONS            the configurations named that the solution has
    SOLUTION_<C>_LIBRARIES             the static libraries it builds, by project name, in the solution's order
    SOLUTION_<C>_PROGRAMS              the applications it builds
    SOLUTION_<C>_<Project>_AS          the project's own configuration it is built as ("Release" in Reference's)
    SOLUTION_<C>_<Project>_LINKS       a program's system libraries: its Link AdditionalDependencies for GCC-x64
    SOLUTION_<Project>_FILE            the project's .vcxproj

A project built by a command of its own (a Makefile project) is neither.  CommonLibraries/CMake/Codaphela.cmake's
codaphela_solution makes the targets from these.
"""
import os
import re
import sys

PLATFORM = "GCC-x64"
BASE = "x64"
PROJECT = re.compile(r'^Project\("\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942\}"\) = "([^"]+)", "([^"]+)", "(\{[0-9A-Fa-f-]+\})"')
ROW = re.compile(r"^\s*(\{[0-9A-Fa-f-]+\})\.(.+)\.(ActiveCfg|Build\.0) = (.+?)\s*$")


def read(path):
	return open(path, encoding="utf-8-sig").read()


def find_file(directory, relative):
	"""The file a solution names, whatever the case of its path on disk."""
	path = directory
	for part in relative.replace("\\", "/").split("/"):
		if part in ("", "."):
			continue
		if part == "..":
			path = os.path.dirname(path)
			continue
		names = os.listdir(path) if os.path.isdir(path) else []
		match = part if part in names else next((n for n in names if n.lower() == part.lower()), part)
		path = os.path.join(path, match)
	return path


def kind(text, configuration):
	"""A project's ConfigurationType as the configuration's base (x64, else any platform's) says it."""
	found = None
	for m in re.finditer(r"<PropertyGroup Condition=\"'\$\(Configuration\)\|\$\(Platform\)'=='([^'|]+)\|([^']+)'\"[^>]*>(.*?)</PropertyGroup>", text, re.S):
		t = re.search(r"<ConfigurationType>([^<]+)</ConfigurationType>", m.group(3))
		if t and m.group(1) == configuration and (found is None or m.group(2) == BASE):
			found = t.group(1)
	return found


def sheets(path, seen=None):
	"""The property sheets a project imports by a path of its own (from the file that imports it; not one of Visual
	Studio's own sheets, whose paths start with its macros), and those they import, each before the file that imports
	it: the order their groups are read in."""
	seen = [] if seen is None else seen
	directory = os.path.dirname(os.path.abspath(path))
	for m in re.finditer(r'<Import Project="(?:\$\(MSBuildThisFileDirectory\)|\$\(ProjectDir\))?([^"$]+\.props)"', read(path)):
		sheet = find_file(directory, m.group(1))
		if os.path.isfile(sheet) and sheet not in seen:
			sheets(sheet, seen)
			seen.append(sheet)
	return seen


def links(path, configuration):
	"""A program's AdditionalDependencies for the configuration on GCC-x64: its sheets' and then its own, the
	platform's group and then the configuration's, each adding to what came before where it says so."""
	result = []
	for file in sheets(path) + [path]:
		text = read(file)
		for condition in ("'$(Platform)'=='%s'" % PLATFORM, "'$(Configuration)|$(Platform)'=='%s|%s'" % (configuration, PLATFORM)):
			for m in re.finditer(r"<ItemDefinitionGroup Condition=\"%s\">(.*?)</ItemDefinitionGroup>" % re.escape(condition), text, re.S):
				link = re.search(r"<Link>(.*?)</Link>", m.group(1), re.S)
				dependencies = re.search(r"<AdditionalDependencies>([^<]*)</AdditionalDependencies>", link.group(1)) if link else None
				if dependencies:
					own = [d for d in dependencies.group(1).split(";") if d and d != "%(AdditionalDependencies)"]
					result = own + (result if "%(AdditionalDependencies)" in dependencies.group(1) else [])
	return result


def solution(path, configurations):
	directory = os.path.dirname(os.path.abspath(path))
	lines = read(path).splitlines()
	projects = [(m.group(1), m.group(2), m.group(3).upper()) for m in map(PROJECT.match, lines) if m]
	listed = set()
	inside = False
	for line in lines:
		if "GlobalSection(SolutionConfigurationPlatforms)" in line:
			inside = True
		elif inside and "EndGlobalSection" in line:
			inside = False
		elif inside and " = " in line:
			listed.add(line.strip().split(" = ")[0])
	mapped = {}
	built = set()
	for m in map(ROW.match, lines):
		if m:
			key = (m.group(1).upper(), m.group(2))
			if m.group(3) == "ActiveCfg":
				mapped[key] = m.group(4)
			else:
				built.add(key)

	out = []
	have = [c for c in configurations if "%s|%s" % (c, PLATFORM) in listed]
	out.append('set(SOLUTION_CONFIGURATIONS "%s")' % ";".join(have))
	files = {}
	for configuration in have:
		name = "%s|%s" % (configuration, PLATFORM)
		libraries = []
		programs = []
		for project, relative, guid in projects:
			if (guid, name) not in built:
				continue
			file = find_file(directory, relative)
			if not os.path.isfile(file):
				sys.exit("%s: %s isn't there" % (path, relative))
			built_as = mapped[(guid, name)].split("|")[0]
			text = read(file)
			project_kind = kind(text, built_as)
			if project_kind == "StaticLibrary":
				libraries.append(project)
			elif project_kind == "Application":
				programs.append(project)
				out.append('set(SOLUTION_%s_%s_LINKS "%s")' % (configuration, project, ";".join(links(file, built_as))))
			else:
				continue
			files[project] = file
			out.append('set(SOLUTION_%s_%s_AS "%s")' % (configuration, project, built_as))
		out.append('set(SOLUTION_%s_LIBRARIES "%s")' % (configuration, ";".join(libraries)))
		out.append('set(SOLUTION_%s_PROGRAMS "%s")' % (configuration, ";".join(programs)))
	for project, file in files.items():
		out.append('set(SOLUTION_%s_FILE "%s")' % (project, file.replace("\\", "/")))
	return "\n".join(out) + "\n"


def main(arguments):
	if len(arguments) < 3 or arguments[0] != "--cmake":
		print(__doc__)
		return 1
	sys.stdout.write(solution(arguments[1], arguments[2:]))
	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
