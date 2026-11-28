#!/usr/bin/env python3
"""Gives a window program's project and solution a Reference configuration, as HCP65IDE's have (HCP65-IDE's
ConfigurationsPlan.txt, C13-5).

    python3 reference_configuration.py [--write] <solution.sln> <program.vcxproj> <WinRefLib.vcxproj>

Reference builds the program with its reference main, <Program>Ref.cpp, which draws into memory on WinRefLib where
<Program>_Windows.cpp and <Program>_Linux.cpp open a window.  It is built as Release is: Common.props gives every
configuration but Debug what Release has, so the project says only what is its own.

The project gains:
- Reference|x64: in its configurations, and a copy of each Release|x64 group it has (its type, its user sheet, what
  it keeps of its own, with SubSystem Console: the reference main is main, not WinMain).
- <Program>Ref.cpp as an item, kept out of every other configuration; <Program>_Windows.cpp kept out of
  Reference|x64 and <Program>_Linux.cpp out of Reference|GCC-x64.
- A reference to WinRefLib.
The solution gains WinRefLib, if it hasn't it, and Reference|x64: the program built as Reference|x64, every other
project as its Release|x64 row says.  solution_rows.py then gives it Reference|GCC-x64.

Only the lines added change.  A project that has Reference|x64 already is left as it is.
"""
import os
import re
import sys

import solution_rows

CONFIGURATION = "Reference|x64"
LIKE = "Release|x64"
OTHERS = ("Debug|Win32", "Debug|x64", "Release|Win32", "Release|x64", "Debug|GCC-x64", "Release|GCC-x64")
PROJECT_LINE = 'Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "%s", "%s", "%s"'


def condition(name):
	return "'$(Configuration)|$(Platform)'=='%s'" % name


def read(path):
	data = open(path, "rb").read()
	bom = data[:3] if data.startswith(b"\xef\xbb\xbf") else b""
	text = data[len(bom):].decode("utf-8")
	return bom, text, ("\r\n" if "\r\n" in text else "\n")


def guid_of(path):
	return re.search(r"<ProjectGuid>(\{[^}]+\})</ProjectGuid>", read(path)[1]).group(1).upper()


def excluded(names, newline):
	return "".join('      <ExcludedFromBuild Condition="%s">true</ExcludedFromBuild>%s' % (condition(n), newline) for n in names)


def project(path, winreflib, write):
	bom, text, nl = read(path)
	if 'Include="%s"' % CONFIGURATION in text:
		return "%s: has %s already" % (path, CONFIGURATION)
	name = os.path.splitext(os.path.basename(path))[0]

	# Its configuration, after Release|x64's.
	entry = re.search(r'    <ProjectConfiguration Include="%s">.*?    </ProjectConfiguration>%s' % (re.escape(LIKE), nl), text, re.S)
	text = text[:entry.end()] + entry.group(0).replace(LIKE, CONFIGURATION).replace("<Configuration>Release<", "<Configuration>Reference<") + text[entry.end():]

	# A copy of each group that is Release|x64's alone, after it.
	pattern = re.compile(r'  <(PropertyGroup|ImportGroup|ItemDefinitionGroup) Condition="%s"[^>]*>.*?  </\1>%s' % (re.escape(condition(LIKE)), nl), re.S)
	at = 0
	while True:
		group = pattern.search(text, at)
		if not group:
			break
		copy = group.group(0).replace(condition(LIKE), condition(CONFIGURATION)).replace("<SubSystem>Windows</SubSystem>", "<SubSystem>Console</SubSystem>")
		text = text[:group.end()] + copy + text[group.end():]
		at = group.end() + len(copy)

	# The three mains.
	windows = '    <ClCompile Include="%s_Windows.cpp" />%s' % (name, nl)
	linux = '    <ClCompile Include="%s_Linux.cpp" />%s' % (name, nl)
	if text.count(windows) != 1 or text.count(linux) != 1 or ('"%sRef.cpp"' % name) in text:
		sys.exit("%s: its mains aren't as expected" % path)
	text = text.replace(windows, '    <ClCompile Include="%s_Windows.cpp">%s%s    </ClCompile>%s' % (name, nl, excluded([CONFIGURATION], nl), nl))
	text = text.replace(linux, '    <ClCompile Include="%s_Linux.cpp">%s%s    </ClCompile>%s    <ClCompile Include="%sRef.cpp">%s%s    </ClCompile>%s'
						% (name, nl, excluded(["Reference|GCC-x64"], nl), nl, name, nl, excluded(OTHERS, nl), nl))

	# WinRefLib, after the last project it references.
	relative = os.path.relpath(os.path.abspath(winreflib), os.path.dirname(os.path.abspath(path))).replace(os.sep, "\\")
	if relative not in text:
		last = text.rfind("    </ProjectReference>" + nl) + len("    </ProjectReference>" + nl)
		text = text[:last] + '    <ProjectReference Include="%s">%s      <Project>%s</Project>%s    </ProjectReference>%s' % (relative, nl, guid_of(winreflib).lower(), nl, nl) + text[last:]
	if write:
		open(path, "wb").write(bom + text.encode("utf-8"))

	# Its filter: where the Linux main is.
	filters = path + ".filters"
	if os.path.exists(filters):
		fbom, ftext, fnl = read(filters)
		item = re.search(r'    <ClCompile Include="%s_Linux.cpp"(>.*?    </ClCompile>| />)%s' % (re.escape(name), fnl), ftext, re.S)
		if item and ('"%sRef.cpp"' % name) not in ftext:
			ftext = ftext[:item.end()] + item.group(0).replace("%s_Linux.cpp" % name, "%sRef.cpp" % name) + ftext[item.end():]
			if write:
				open(filters, "wb").write(fbom + ftext.encode("utf-8"))
	return "%s: %s" % (path, "rewritten" if write else "would change")


def solution(path, program, winreflib, write):
	bom, text, nl = read(path)
	lines = text.split(nl)
	directory = os.path.dirname(os.path.abspath(path))
	program_guid = guid_of(program)
	reference_guid = guid_of(winreflib)
	if not any(reference_guid in l.upper() and l.startswith("Project(") for l in lines):
		relative = os.path.relpath(os.path.abspath(winreflib), directory).replace(os.sep, "\\")
		at = lines.index("Global")
		lines[at:at] = [PROJECT_LINE % ("WinRefLib", relative, reference_guid), "EndProject"]
	start = next(i for i, l in enumerate(lines) if "GlobalSection(SolutionConfigurationPlatforms)" in l)
	end = lines.index("\tEndGlobalSection", start)
	listed = [l.strip().split(" = ")[0] for l in lines[start + 1:end]]
	if CONFIGURATION in listed:
		return "%s: has %s already" % (path, CONFIGURATION)
	rows_start = next(i for i, l in enumerate(lines) if "GlobalSection(ProjectConfigurationPlatforms)" in l)
	rows_end = lines.index("\tEndGlobalSection", rows_start)
	rows = lines[rows_start + 1:rows_end]
	guids = [m.group(1).upper() for m in (re.match(r'Project\("[^"]+"\) = "[^"]+", "[^"]+", "(\{[^}]+\})"', l) for l in lines) if m]
	row = re.compile(r"\t\t(\{[0-9A-Fa-f-]+\})\.(.+)\.(ActiveCfg|Build\.0) = (.+)$")
	# Each new row where Visual Studio would sort it among its project's: no row there already moves.
	new = list(rows)
	for guid in guids:
		own = [m for m in (row.match(r) for r in rows) if m and m.group(1).upper() == guid]
		added = []
		# A project the solution had no rows for (WinRefLib, just added): every configuration, built as its x64 or
		# Win32 one of the name (a GCC-x64 row names an x64 configuration, as solution_rows.py writes them).
		for name in ([] if own else listed):
			added += [(name, suffix, name.replace("|GCC-x64", "|x64")) for suffix in ("ActiveCfg", "Build.0")]
		built = (not own) or (guid == program_guid) or any(m.group(2) == LIKE and m.group(3) == "Build.0" for m in own)
		target = CONFIGURATION if guid == program_guid else LIKE
		added += [(CONFIGURATION, suffix, target) for suffix in (["ActiveCfg", "Build.0"] if built else ["ActiveCfg"])]
		for name, suffix, to in added:
			solution_rows.insert_row(new, "\t\t%s.%s.%s = %s" % (guid, name, suffix, to))
	lines[rows_start + 1:rows_end] = new
	configurations = lines[start + 1:end]
	at = next((i for i, other in enumerate(configurations) if other.strip() > CONFIGURATION), len(configurations))
	configurations.insert(at, "\t\t%s = %s" % (CONFIGURATION, CONFIGURATION))
	lines[start + 1:end] = configurations
	if write:
		open(path, "wb").write(bom + nl.join(lines).encode("utf-8"))
	return "%s: %s" % (path, "rewritten" if write else "would change")


def main(arguments):
	write = "--write" in arguments
	arguments = [a for a in arguments if a != "--write"]
	if len(arguments) != 3:
		print(__doc__)
		return 1
	print(project(arguments[1], arguments[2], write))
	print(solution(arguments[0], arguments[1], arguments[2], write))
	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
