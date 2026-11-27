#!/usr/bin/env python3
"""Moves Visual Studio projects onto their repository's property sheet (HCP65-IDE's ConfigurationsPlan.txt, C7).

    python3 project_sheets.py <sheet.props> <project.vcxproj>...             what each project would keep
    python3 project_sheets.py --write <sheet.props> <project.vcxproj>...     and rewrite them
    python3 project_sheets.py --cmake <sheet.props>                          the sheet's GCC-x64 groups, as CMake set()s

A project imports the sheet once, with no condition, after the import groups Visual Studio wrote, and loses each
setting of its four configurations that the sheet (and the sheets it imports) already gives that configuration.
What is left is the project's own.  Only those lines change: the rest of the file stays byte for byte.

Decided in the plan's question 8 and done here: the defines _LIB, WIN32, _WINDOWS, _CONSOLE and D3D_DEBUG_INFO go
(nothing tests them), and BREAK_ON_WARNING and BREAK_ON_ERROR from a Release configuration that had them;
Microsoft.CPP.UpgradeFromVC70.props isn't imported; an OutDir or IntDir that is Visual
Studio's default goes; and a project's own DisableSpecificWarnings, ExceptionHandling and LinkIncremental go
whatever they are, the sheet's being every project's.  A project's Configuration groups (its type, toolset and character set) stay: they are read
before a sheet is.
"""
import os
import re
import sys

CONFIGURATIONS = [(c, p) for c in ("Debug", "Release") for p in ("Win32", "x64")]
GONE_DEFINES = {"_LIB", "WIN32", "_WINDOWS", "_CONSOLE", "D3D_DEBUG_INFO"}
GONE_RELEASE_DEFINES = {"BREAK_ON_WARNING", "BREAK_ON_ERROR"}
DEFAULT_DIRECTORIES = {
	("OutDir", "Win32"): "$(SolutionDir)$(Configuration)\\", ("IntDir", "Win32"): "$(Configuration)\\",
	("OutDir", "x64"): "$(SolutionDir)$(Platform)\\$(Configuration)\\", ("IntDir", "x64"): "$(Platform)\\$(Configuration)\\",
}
SHEETS_ALONE = {"ClCompile.DisableSpecificWarnings", "ClCompile.ExceptionHandling", "LinkIncremental"}
TOOLS = {"ClCompile", "Link", "Lib", "Bscmake", "ResourceCompile", "Midl", "PreBuildEvent", "PostBuildEvent", "CustomBuildStep", "Manifest"}
LISTS = {"ClCompile.PreprocessorDefinitions": "%(PreprocessorDefinitions)", "Link.AdditionalDependencies": "%(AdditionalDependencies)",
		 "IncludePath": "$(IncludePath)"}
CONDITION = re.compile(r"""Condition="'(\$\(Configuration\)\|\$\(Platform\)|\$\(Configuration\)|\$\(Platform\))'\s*==\s*'([^']*)'\"""")


def is_for(tag, configuration):
	"""Whether an element's Condition (none, or one of the three kinds the IDE reads) is true of a configuration."""
	if "Condition=" not in tag:
		return True
	m = CONDITION.search(tag)
	if not m:
		return False
	return m.group(2) == {"$(Configuration)|$(Platform)": "%s|%s" % configuration, "$(Configuration)": configuration[0],
						  "$(Platform)": configuration[1]}[m.group(1)]


def read_sheet(path, values, configurations=CONFIGURATIONS):
	"""What a sheet gives each configuration, values[configuration][name], the sheets it imports first where it does."""
	text = open(path, encoding="utf-8-sig").read()
	text = re.sub(r"<!--.*?-->", "", text, flags=re.S)
	here = os.path.dirname(os.path.abspath(path)) + os.sep
	for m in re.finditer(r"<Import ([^>]*)/>|<(PropertyGroup|ItemDefinitionGroup)([^>]*)>(.*?)</\2>", text, re.S):
		if m.group(1) is not None:
			imported = re.search(r'Project="([^"]*)"', m.group(1)).group(1).replace("$(MSBuildThisFileDirectory)", "")
			read_sheet(os.path.join(here, imported.replace("\\", os.sep)), values, configurations)
			continue
		settings = []
		if m.group(2) == "PropertyGroup":
			settings = [(s.group(1), s.group(2), s.group(3)) for s in re.finditer(r"<(\w+)([^>]*)>([^<]*)</\1>", m.group(4))]
		else:
			for tool in re.finditer(r"<(\w+)>(.*?)</\1>", m.group(4), re.S):
				settings += [(tool.group(1) + "." + s.group(1), s.group(2), s.group(3)) for s in re.finditer(r"<(\w+)([^>]*)>([^<]*)</\1>", tool.group(2))]
		for configuration in configurations:
			if is_for(m.group(3), configuration):
				for name, attributes, value in settings:
					value = value.replace("$(MSBuildThisFileDirectory)", here)
					if is_for(attributes, configuration):
						own = LISTS.get(name) or ("%(" + name.split(".")[-1] + ")" if "." in name else "$(" + name + ")")
						old = values[configuration].get(name)
						values[configuration][name] = value.replace(own, old) if (old is not None and own in value) else value


def parts(value, own):
	return [p.strip() for p in value.split(";") if p.strip() and p.strip() != own]


def expand(value, properties):
	"""A sheet's value with the sheet's own properties ($(CommonLibrariesDir)) put in."""
	for _ in range(8):
		m = re.search(r"\$\((\w+)\)", value)
		if not m or m.group(1) not in properties:
			break
		value = value.replace(m.group(0), properties[m.group(1)])
	return value


def directory(part, project_directory):
	"""An include directory as one path, to compare a project's with its sheet's."""
	part = part.replace("$(ProjectDir)", project_directory + os.sep).replace("\\", os.sep)
	return os.path.normcase(os.path.normpath(part if os.path.isabs(part) else os.path.join(project_directory, part)))


def reduce_setting(name, value, configuration, sheet, project_directory, sheet_directory):
	"""What a project's setting becomes on its sheet: None to drop its line, else its value (changed or not)."""
	given = sheet[configuration].get(name)
	if name in SHEETS_ALONE and given is not None:
		return None
	if name in ("OutDir", "IntDir"):
		return None if value == DEFAULT_DIRECTORIES[(name, configuration[1])] else value
	if name == "Link.IgnoreSpecificDefaultLibraries" and not value.strip():
		return None
	if name in LISTS and given is not None:
		own = LISTS[name]
		theirs = parts(given, own)
		mine = parts(value, own)
		if name == "IncludePath":
			theirs = [directory(expand(p, sheet[configuration]), sheet_directory) for p in theirs]
			extra = [p for p in mine if directory(p, project_directory) not in theirs]
		else:
			extra = [p for p in mine if p not in theirs and not (name.endswith("PreprocessorDefinitions") and
					 (p in GONE_DEFINES or (configuration[0] == "Release" and p in GONE_RELEASE_DEFINES)))]
		if not extra:
			return None
		return ";".join(extra + [own])
	return None if (given is not None and given.strip() == value.strip()) else value


def move(path, sheet_path, sheet, write):
	data = open(path, "rb").read()
	bom = data[:3] if data.startswith(b"\xef\xbb\xbf") else b""
	lines = data[len(bom):].decode("utf-8").split("\n")
	project_directory = os.path.dirname(os.path.abspath(path))
	sheet_directory = os.path.dirname(os.path.abspath(sheet_path))
	out = []
	kept = []
	configuration = None		# The configuration of the group being read, if it is one of the four.
	tool = None
	# An element written empty over two lines (<IgnoreSpecificDefaultLibraries> and its end) is read as one.
	joined = []
	for line in lines:
		m = re.match(r"\s*</(\w+)>\s*$", line)
		if m and joined and joined[-1].strip() == "<%s>" % m.group(1) and m.group(1) not in TOOLS:
			joined[-1] = joined[-1].rstrip("\r") + line.strip() + line[len(line.rstrip("\r")):]
		else:
			joined.append(line)
	lines = joined
	for line in lines:
		bare = line.rstrip("\r")
		ending = line[len(bare):]
		stripped = bare.strip()
		m = re.match(r"<(PropertyGroup|ItemDefinitionGroup)\b([^>]*)>$", stripped)
		if m:
			c = CONDITION.search(m.group(2))
			configuration = tuple(c.group(2).split("|")) if (c and c.group(1) == "$(Configuration)|$(Platform)") else None
			if configuration not in CONFIGURATIONS or 'Label="Configuration"' in m.group(2):
				configuration = None if 'Label="Configuration"' in m.group(2) or m.group(2).strip() else "each"
			tool = None
		elif stripped in ("</PropertyGroup>", "</ItemDefinitionGroup>"):
			configuration = None
		elif configuration and stripped[1:-1] in TOOLS and stripped == "<%s>" % stripped[1:-1]:
			tool = stripped[1:-1]
		elif configuration and tool and stripped == "</%s>" % tool:
			tool = None
		if "Microsoft.CPP.UpgradeFromVC70.props" in stripped:
			continue
		s = re.match(r"<(\w+)([^>]*)>([^<]*)</\1>$", stripped)
		if s and configuration:
			# A property with its own Condition says its configuration itself (a group with none holds several).
			c = CONDITION.search(s.group(2))
			own = tuple(c.group(2).split("|")) if (c and c.group(1) == "$(Configuration)|$(Platform)") else (None if s.group(2).strip() else configuration)
			if own in CONFIGURATIONS:
				name = (tool + "." if tool else "") + s.group(1)
				value = reduce_setting(name, s.group(3), own, sheet, project_directory, sheet_directory)
				if value is None:
					continue
				if value != s.group(3):
					line = bare[:len(bare) - len(bare.lstrip())] + "<%s%s>%s</%s>" % (s.group(1), s.group(2), value, s.group(1)) + ending
				kept.append("%s|%s %s = %s" % (own[0], own[1], name, value))
		out.append(line)

	# Elements left with nothing in them, innermost first, then the import of the sheet after the last PropertySheets.
	text = "\n".join(out)
	for _ in range(3):
		text = re.sub(r"[ \t]*<(ClCompile|Link|Lib|Bscmake|ResourceCompile)>\r?\n[ \t]*</\1>\r?\n", "", text)
		text = re.sub(r"[ \t]*<(Lib|Bscmake) />\r?\n", "", text)
		text = re.sub(r"[ \t]*<(PropertyGroup|ItemDefinitionGroup)( Condition=\"[^\"]*\")?>\r?\n[ \t]*</\1>\r?\n", "", text)
	relative = os.path.relpath(os.path.abspath(sheet_path), project_directory).replace(os.sep, "\\")
	if "<Import Project=\"%s\"" % relative not in text:
		groups = list(re.finditer(r"([ \t]*)<ImportGroup[^>]*Label=\"PropertySheets\"[^>]*>.*?</ImportGroup>(\r?\n)", text, re.S))
		assert groups, "%s has no PropertySheets import group" % path
		last = groups[-1]
		indent, newline = last.group(1), last.group(2)
		text = text[:last.end()] + ('%s<ImportGroup Label="PropertySheets">%s%s  <Import Project="%s" />%s%s</ImportGroup>%s'
									% (indent, newline, indent, relative, newline, indent, newline)) + text[last.end():]
	changed = bom + text.encode("utf-8") != data
	if write and changed:
		open(path, "wb").write(bom + text.encode("utf-8"))
	return kept, changed


def cmake(sheet_path):
	"""What CMake/Codaphela.cmake builds with: the sheet's GCC-x64 configurations' defines, options and link options."""
	gcc = [("Debug", "GCC-x64"), ("Release", "GCC-x64")]
	values = {configuration: {} for configuration in gcc}
	read_sheet(sheet_path, values, gcc)
	for configuration in gcc:
		name = configuration[0].upper()
		given = values[configuration]
		print('set(SHEET_DEFINES_%s "%s")' % (name, ";".join(parts(given.get("ClCompile.PreprocessorDefinitions", ""), "%(PreprocessorDefinitions)"))))
		for tool, variable in (("ClCompile", "OPTIONS"), ("Link", "LINK_OPTIONS")):
			options = given.get(tool + ".AdditionalOptions", "").replace("%(AdditionalOptions)", "").split()
			print('set(SHEET_%s_%s "%s")' % (variable, name, ";".join(options)))
	return 0


def main(arguments):
	if arguments[:1] == ["--cmake"] and len(arguments) == 2:
		return cmake(arguments[1])
	write = "--write" in arguments
	arguments = [a for a in arguments if a != "--write"]
	if len(arguments) < 2:
		print(__doc__)
		return 1
	sheet = {configuration: {} for configuration in CONFIGURATIONS}
	read_sheet(arguments[0], sheet)
	for path in arguments[1:]:
		kept, changed = move(path, arguments[0], sheet, write)
		print("%s: %s, keeps %d settings of its own" % (path, ("rewritten" if write else "would change") if changed else "as it is", len(kept)))
		for line in sorted(set(kept)):
			print("    " + line)
	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
