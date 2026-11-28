#!/usr/bin/env python3
"""Renames a Visual Studio solution's own platforms, Win32 to MSVC-x86 and x64 to MSVC-x64 (HCP65-IDE's
ConfigurationsPlan.txt, C11-2): a solution's configurations are names of its own, and MSVC's then say whose they are
as GCC-x64's and HCP65C816's do.

    python3 solution_platforms.py [--write] <solution.sln>...

Only the solution's names change: each line of its SolutionConfigurationPlatforms ("Release|x64 = Release|x64") and
the left side of each row of its projects ("{GUID}.Release|x64.Build.0 = Release|x64": the project's own
configuration, on the right, is as it was).  Each line stays where it is, and the rest of the file byte for byte.
A solution that has the new names already is left as it is.  MSBuild is then given the solution's name:
/p:Platform=MSVC-x64.

to_old and to_new are for the tools that read a solution's rows by the old names (solution_rows.py,
reference_configuration.py): they work on the solution as it was named, and write it as it is named.
"""
import re
import sys

NEW = {"Win32": "MSVC-x86", "x64": "MSVC-x64"}
OLD = {new: old for old, new in NEW.items()}
LISTED = re.compile(r"^(\t\t)([^|=\r\n]+)\|([^|=\r\n]+?) = \2\|\3(\r?)$")
ROW = re.compile(r"^(\t\t\{[0-9A-Fa-f-]+\}\.[^|=\r\n]+)\|([^|=\r\n]+?)(\.(?:ActiveCfg|Build\.0|Deploy\.0) = .*)$")


def renamed(text, names):
	"""The solution's text with its platforms renamed by names, and how many lines changed."""
	lines = text.split("\n")
	section = None
	changed = 0
	for at, line in enumerate(lines):
		if "GlobalSection(SolutionConfigurationPlatforms)" in line:
			section = "listed"
		elif "GlobalSection(ProjectConfigurationPlatforms)" in line:
			section = "rows"
		elif "EndGlobalSection" in line:
			section = None
		elif section == "listed":
			m = LISTED.match(line)
			if m and m.group(3) in names:
				lines[at] = "%s%s|%s = %s|%s%s" % (m.group(1), m.group(2), names[m.group(3)], m.group(2), names[m.group(3)], m.group(4))
				changed += 1
		elif section == "rows":
			m = ROW.match(line)
			if m and m.group(2) in names:
				lines[at] = "%s|%s%s" % (m.group(1), names[m.group(2)], m.group(3))
				changed += 1
	return "\n".join(lines), changed


def to_new(text):
	return renamed(text, NEW)[0]


def to_old(text):
	"""The text as it was named, and whether it had the new names."""
	text, changed = renamed(text, OLD)
	return text, changed != 0


def main(arguments):
	write = "--write" in arguments
	paths = [a for a in arguments if a != "--write"]
	if not paths:
		print(__doc__)
		return 1
	for path in paths:
		data = open(path, "rb").read()
		bom = data[:3] if data.startswith(b"\xef\xbb\xbf") else b""
		text = data[len(bom):].decode("utf-8")
		new, changed = renamed(text, NEW)
		if changed == 0:
			print("%s: has no Win32 or x64 of its own" % path)
			continue
		if write:
			open(path, "wb").write(bom + new.encode("utf-8"))
		print("%s: %s, %d lines" % (path, "renamed" if write else "would be renamed", changed))
	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
