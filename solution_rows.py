#!/usr/bin/env python3
"""Gives Visual Studio solutions their GCC-x64 configurations, or with --platform an HCP65 solution its HCP65C816
ones (HCP65-IDE's ConfigurationsPlan.txt, The solution's rows).

    python3 solution_rows.py [--write] [--platform HCP65C816] [--add <project.vcxproj>]... [--build <Project>]... <solution.sln>...

For Debug, Release and Reference (HCP65IDE's, the plan's C13), where a solution has them on x64, it adds the solution configuration <name>|GCC-x64
and a row for each project: mapped to what the project's <name>|x64 row maps to, and built if that row is built,
unless the project is one only Windows builds.  A project only Linux builds has a row for every configuration and is
built in GCC-x64's alone, but not in Reference's, which has no window.  --add puts such a project (X11Lib) into the solutions that haven't it.

The HCP65 IDE builds what the rows say (Visual Studio shows GCC-x64 in its configuration boxes; choosing it there
builds the ticked projects with MSVC, which isn't what it is for).  Only the lines added change: the rest of each
file stays byte for byte, each new row put where Visual Studio would sort it among its project's.  A solution that
has the configurations already is left as it is.

An HCP65 platform's rows are as Win32's, the platform the HCP65 compiler's configurations are read beside: every
project built that Win32 builds.  --build names a project the platform's configurations build though Win32's don't:
Libc65, the runtime, which an HCP65 build takes from the rows (the plan's C4-7, question 23) and Visual Studio never
builds.  Each of its rows on the platform that has no .Build.0 is given one, and nothing else is done: a solution
gets no configuration it hasn't.
"""
import os
import re
import sys

PLATFORM = "GCC-x64"
NAMES = ("Debug", "Release", "Reference")
REFERENCE = "Reference"		# A program drawn into memory on WinRefLib: no window, so nothing of X11Lib's.
BASE = "x64"
BASES = {"GCC-x64": "x64", "HCP65C816": "Win32", "HCP65C64": "Win32"}
WINDOWS_ONLY = {"WinGdiLib", "WinInputLib", "WinWaveoutLib"}
LINUX_ONLY = {"X11Lib", "PulseAudioLib"}
PROJECT = re.compile(r'^Project\("\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942\}"\) = "([^"]+)", "([^"]+)", "(\{[0-9A-Fa-f-]+\})"')


def key(row):
	"""A row's project, solution configuration and kind; None for a line that isn't a row."""
	m = re.match(r"\t\t(\{[0-9A-Fa-f-]+\})\.(.+)\.(ActiveCfg|Build\.0|Deploy\.0) = ", row)
	return (m.group(1).upper(), m.group(2), m.group(3)) if m else None

def insert(rows_, row, same):
	"""Puts row before the first of rows_ that same() says is its kind and sorts after it, else after the last such."""
	last = None
	for at, other in enumerate(rows_):
		if same(other):
			if key(other)[1:] > key(row)[1:]:
				rows_.insert(at, row)
				return
			last = at
	rows_.insert(len(rows_) if last is None else last + 1, row)


def insert_row(rows_, row):
	"""Puts a project's row where Visual Studio would sort it among that project's, leaving the others as they are."""
	guid = key(row)[0]
	insert(rows_, row, lambda other: key(other) is not None and key(other)[0] == guid)


def project_guid(path):
	return re.search(r"<ProjectGuid>(\{[^}]+\})</ProjectGuid>", open(path, encoding="utf-8-sig").read()).group(1).upper()


def rows(text, write, additions, path):
	newline = "\r\n" if "\r\n" in text else "\n"
	lines = text.split(newline)
	directory = os.path.dirname(os.path.abspath(path))
	projects = {m.group(3).upper(): m.group(1) for m in map(PROJECT.match, lines) if m}

	# Projects only Linux builds that the solution hasn't, before Global.
	added = []
	for addition in additions:
		guid = project_guid(addition)
		if guid not in projects:
			name = os.path.splitext(os.path.basename(addition))[0]
			relative = os.path.relpath(os.path.abspath(addition), directory).replace(os.sep, "\\")
			at = lines.index("Global")
			lines[at:at] = ['Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "%s", "%s", "%s"' % (name, relative, guid), "EndProject"]
			projects[guid] = name
			added.append(name)

	start = next(i for i, l in enumerate(lines) if "GlobalSection(SolutionConfigurationPlatforms)" in l)
	end = lines.index("\tEndGlobalSection", start)
	configurations = [l.strip().split(" = ")[0] for l in lines[start + 1:end]]
	names = [c.split("|")[0] for c in configurations if c.endswith("|" + BASE) and c.split("|")[0] in NAMES]
	wanted = [n for n in names if "%s|%s" % (n, PLATFORM) not in configurations]
	rows_start = next(i for i, l in enumerate(lines) if "GlobalSection(ProjectConfigurationPlatforms)" in l)
	rows_end = lines.index("\tEndGlobalSection", rows_start)
	old = lines[rows_start + 1:rows_end]
	mapped = {}			# (guid, solution configuration) -> the project configuration it maps to
	built = set()
	for l in old:
		m = re.match(r"\t\t(\{[0-9A-Fa-f-]+\})\.(.+)\.(ActiveCfg|Build\.0) = (.+)$", l)
		if m:
			mapped[(m.group(1).upper(), m.group(2))] = m.group(4)
			if m.group(3) == "Build.0":
				built.add((m.group(1).upper(), m.group(2)))

	new = list(old)
	for guid, name in projects.items():
		for configuration in configurations + ["%s|%s" % (n, PLATFORM) for n in wanted]:
			new_row = configuration.endswith("|" + PLATFORM)
			gcc = new_row and (PLATFORM == "GCC-x64")
			if (guid, configuration) in mapped:
				continue
			like = configuration.split("|")[0] + "|" + BASE if new_row else configuration
			target = mapped.get((guid, like)) or (like.replace(REFERENCE + "|", "Release|") if name in LINUX_ONLY else None)
			if target is None:
				continue
			build = (gcc and not configuration.startswith(REFERENCE + "|")) if name in LINUX_ONLY else (((guid, like) in built) and ((name not in WINDOWS_ONLY) or (PLATFORM != "GCC-x64")))
			for suffix in ["ActiveCfg"] + (["Build.0"] if build else []):
				insert_row(new, "\t\t%s.%s.%s = %s" % (guid, configuration, suffix, target))
	lines[rows_start + 1:rows_end] = new
	listed = lines[start + 1:end]
	for name in wanted:
		row = "\t\t%s|%s = %s|%s" % (name, PLATFORM, name, PLATFORM)
		at = next((i for i, other in enumerate(listed) if other.strip() > row.strip()), len(listed))
		listed.insert(at, row)
	lines[start + 1:end] = listed
	return newline.join(lines), wanted, added


def build(text, names):
	"""Gives each project named a .Build.0 beside each of its rows on the platform that hasn't one."""
	newline = "\r\n" if "\r\n" in text else "\n"
	lines = text.split(newline)
	guids = {m.group(3).upper() for m in map(PROJECT.match, lines) if m and m.group(1) in names}
	start = next(i for i, l in enumerate(lines) if "GlobalSection(ProjectConfigurationPlatforms)" in l)
	end = lines.index("\tEndGlobalSection", start)
	old = lines[start + 1:end]
	keys = {key(l) for l in old}
	new = list(old)
	for line in old:
		k = key(line)
		if k and k[0] in guids and k[2] == "ActiveCfg" and k[1].endswith("|" + PLATFORM) and (k[0], k[1], "Build.0") not in keys:
			insert_row(new, line.replace(".ActiveCfg = ", ".Build.0 = "))
	lines[start + 1:end] = new
	return newline.join(lines)


def main(arguments):
	global PLATFORM, BASE
	write = "--write" in arguments
	additions = []
	built = []
	solutions = []
	arguments = [a for a in arguments if a != "--write"]
	while arguments:
		argument = arguments.pop(0)
		if argument == "--add":
			additions.append(arguments.pop(0))
		elif argument == "--build":
			built.append(arguments.pop(0))
		elif argument == "--platform":
			PLATFORM = arguments.pop(0)
			BASE = BASES[PLATFORM]
		else:
			solutions.append(argument)
	if not solutions:
		print(__doc__)
		return 1
	for path in solutions:
		data = open(path, "rb").read()
		bom = data[:3] if data.startswith(b"\xef\xbb\xbf") else b""
		if built:
			# Only that: the platform's configurations are the solution's as they are.
			text, wanted, added = build(data[len(bom):].decode("utf-8"), built), [], []
		else:
			text, wanted, added = rows(data[len(bom):].decode("utf-8"), write, additions, path)
		changed = bom + text.encode("utf-8") != data
		print("%s: %s%s%s" % (path, ("rewritten" if write else "would change") if changed else "as it is",
								(", " + " and ".join("%s|%s" % (n, PLATFORM) for n in wanted)) if wanted else "",
								(", with " + " and ".join(added)) if added else ""))
		if write and changed:
			open(path, "wb").write(bom + text.encode("utf-8"))
	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
