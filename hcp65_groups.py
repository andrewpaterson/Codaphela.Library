#!/usr/bin/env python3
"""Moves an HCP65 program's compile options and linking from its configurations file into its project
(HCP65-IDE's ConfigurationsPlan.txt, C4-5).

    python3 hcp65_groups.py [--write] <program.hcp65build>...

The project is the .vcxproj of the file's name beside it.  What the file's configurations say of the whole program
(NoFloats, MathTables, PreferSize, Exceptions, KeepUnusedObjects, StubMissing) becomes the project's HCP65Compile
settings and their <Image>, <Executable> or <Module> its HCP65Link settings, in an ItemDefinitionGroup for the
platform (HCP65C816) where every configuration of it says the same and for each configuration where they differ.
Debug="true" isn't moved: a Debug configuration's projects are compiled for the debugger by the sheet the project is
made to import, CommonLibraries/HCP65Compile.props.  The file keeps its configurations' names and what debugging
needs (cards, <Load>, <Lines>), which are the plan's C8.

It stops at anything it doesn't know how to move, and changes nothing without --write.
"""
import os
import re
import sys

PLATFORMS = {"W65C816": "HCP65C816", "65C64": "HCP65C64"}
PROGRAM = ("NoFloats", "MathTables", "PreferSize", "Exceptions", "KeepUnusedObjects", "StubMissing")
OWN = ("Unoptimised", "WarnMissingOverride", "WarnStringToPointer", "MultiWordCasts", "CConversions")
KEPT = ("Name", "Toolchain", "Solution", "BreakpointSpace", "Card", "CardPath")
SHEET = "HCP65Compile.props"
CONFIGURATION = re.compile(r'(<Configuration\b)([^>]*?)(/?>)(.*?)(?=<Configuration\b|</BuildConfigurations>)', re.S)
ATTRIBUTE = re.compile(r'\s+(\w+)="([^"]*)"')


def attributes(text):
	return dict(ATTRIBUTE.findall(text))


def link_settings(body, name):
	"""The HCP65Link settings of a configuration's <Image>, <Executable> or <Module>, in the order they are written."""
	settings = []
	tags = re.findall(r'<(Image|Executable|Module)\b([^>]*?)(/>|>(.*?)</\1>)', body, re.S)
	if len(tags) > 1:
		sys.exit("%s links more than one program" % name)
	for tag, text, _, inner in tags:
		values = attributes(text)
		settings.append((tag, values.pop("File")))
		for key in ("Entry", "Limit", "Size", "Fill", "Load", "Text"):
			if key in values:
				settings.append((key, values.pop(key)))
		if values:
			sys.exit("%s: <%s> has %s" % (name, tag, ", ".join(values)))
		symbols = [attributes(s) for s in re.findall(r'<Symbol\b([^>]*)/>', inner)]
		regions = [attributes(s) for s in re.findall(r'<Region\b([^>]*)/>', inner)]
		if symbols:
			settings.append(("Symbols", ";".join("%s=%s" % (s["Name"], s["Value"]) for s in symbols)))
		if regions:
			settings.append(("Regions", ";".join("%s-%s@%s" % (r["Start"], r["End"], r["Offset"]) for r in regions)))
		if re.sub(r'<(Symbol|Region)\b[^>]*/>', "", inner).strip():
			sys.exit("%s: <%s> holds something else" % (name, tag))
	return settings


def group(condition, compile_settings, link, newline):
	if not compile_settings and not link:
		return ""
	lines = ['  <ItemDefinitionGroup Condition="%s">' % condition]
	for tool, settings in (("HCP65Compile", compile_settings), ("HCP65Link", link)):
		if settings:
			lines.append("    <%s>" % tool)
			lines += ["      <%s>%s</%s>" % (key, value, key) for key, value in settings]
			lines.append("    </%s>" % tool)
	lines.append("  </ItemDefinitionGroup>")
	return newline.join(lines) + newline


def move(path, write):
	text = open(path, newline="").read()
	project_path = os.path.splitext(path)[0] + ".vcxproj"
	project = open(project_path, "rb").read()
	bom = project[:3] if project.startswith(b"\xef\xbb\xbf") else b""
	project = project[len(bom):].decode("utf-8")
	newline = "\r\n" if "\r\n" in project else "\n"
	if "<HCP65Compile>" in project or "<HCP65Link>" in project:
		return "%s: its project has HCP65 groups already" % path

	by_platform = {}		# platform -> [(configuration, compile settings, link settings)]
	kept = text
	for match in CONFIGURATION.finditer(text):
		values = attributes(match.group(2))
		name = values["Name"]
		configuration, platform = name.split("|")
		if values["Toolchain"] not in PLATFORMS:
			continue
		if platform != PLATFORMS[values["Toolchain"]]:
			sys.exit("%s: %s isn't on its toolchain's platform" % (path, name))
		if (values.get("Debug") == "true") != (configuration == "Debug"):
			sys.exit("%s: %s: Debug isn't as the sheet has it" % (path, name))
		unknown = [key for key in values if key not in KEPT + PROGRAM + ("Debug",)]
		if unknown:
			sys.exit("%s: %s has %s" % (path, name, ", ".join(unknown)))
		compile_settings = [(key, values[key]) for key in PROGRAM if key in values]
		by_platform.setdefault(platform, []).append((configuration, compile_settings, link_settings(match.group(4), name)))

		head = match.group(1) + "".join(' %s="%s"' % (key, value) for key, value in ATTRIBUTE.findall(match.group(2)) if key in KEPT) + match.group(3)
		body = re.sub(r'[ \t]*<(Image|Executable|Module)\b[^>]*?(/>|>.*?</\1>)\r?\n', "", match.group(4), flags=re.S)
		kept = kept.replace(match.group(0), head + body)

	groups = ""
	for platform, configurations in by_platform.items():
		shared_compile = [s for s in configurations[0][1] if all(s in c[1] for c in configurations)]
		same_link = all(c[2] == configurations[0][2] for c in configurations)
		groups += group("'$(Platform)'=='%s'" % platform, shared_compile, configurations[0][2] if same_link else [], newline)
		for configuration, compile_settings, link in configurations:
			own = [s for s in compile_settings if s not in shared_compile]
			if own or (link and not same_link):
				groups += group("'$(Configuration)|$(Platform)'=='%s|%s'" % (configuration, platform), own, [] if same_link else link, newline)

	sheet = os.path.relpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), SHEET), os.path.dirname(os.path.abspath(project_path))).replace(os.sep, "\\")
	imports = '  <ImportGroup Label="PropertySheets">%s    <Import Project="%s" />%s  </ImportGroup>%s' % (newline, sheet, newline, newline)
	anchor = '  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />' + newline
	last = project.rfind("  </ItemDefinitionGroup>" + newline)
	if project.count(anchor) != 1 or last < 0 or 'Label="PropertySheets"' in project:
		sys.exit("%s isn't laid out as the others are" % project_path)
	last += len("  </ItemDefinitionGroup>" + newline)
	project = project[:last] + groups + project[last:]
	project = project.replace(anchor, anchor + imports)

	if write:
		open(project_path, "wb").write(bom + project.encode("utf-8"))
		open(path, "w", newline="").write(kept)
	return "%s: %s, %d lines of groups" % (path, "moved" if write else "would move", groups.count(newline))


def main(arguments):
	write = "--write" in arguments
	paths = [a for a in arguments if a != "--write"]
	if not paths:
		print(__doc__)
		return 1
	for path in paths:
		print(move(path, write))
	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
