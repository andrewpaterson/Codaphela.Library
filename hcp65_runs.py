#!/usr/bin/env python3
"""Moves how the HCP65 programs are debugged out of their configurations files into card files and shared run
configurations, and removes the files (HCP65-IDE's ConfigurationsPlan.txt, C8-6 and question 27).

    python3 hcp65_runs.py [--write] --cards <folder> <program.hcp65build>...

Run after hcp65_groups.py, which leaves a .hcp65build with only its configurations' names and their debugging.

- Each card image the files name gets a card file in the cards folder (BootCard.img: BootCard.hcp65card): the image,
  and each program a configuration puts on it, its file written with $(Platform) and $(Configuration) so one line
  serves Debug and Release.  A <Lines> (another program's line table, read as it is linked: the kernal's) becomes a
  line of the card for that file as written, with Lines.
- Each configuration becomes a shared run configuration in the .run folder beside its solution, called after the
  program and the configuration ("ModPlay Debug"): its build configuration, its card file and its <Load>s.
  BreakpointSpace isn't written where it is what the IDE works out (an executable's User, a module's or image's
  Kernel).
- The .hcp65build is deleted: the solution lists its HCP65 configurations itself (solution_rows.py --platform).

The files are written as the IDE writes them (CXMLWriter), so its saving one changes nothing.

It stops at anything it can't move, and changes nothing without --write.
"""
import os
import re
import sys
from xml.sax.saxutils import quoteattr

CONFIGURATION = re.compile(r'<Configuration\b([^>]*?)(/?)>(.*?)(?=<Configuration\b|</BuildConfigurations>)', re.S)
ATTRIBUTE = re.compile(r'\s+(\w+)="([^"]*)"')
SPACES = {"Executable": "User", "Module": "Kernel", "Image": "Kernel"}


def attributes(text):
	return dict(ATTRIBUTE.findall(text))


def windows(path, start):
	return os.path.relpath(path, start).replace(os.sep, "\\")


def full(directory, path):
	return os.path.normpath(os.path.join(directory, path.replace("\\", os.sep)))


def main(arguments):
	write = "--write" in arguments
	arguments = [a for a in arguments if a != "--write"]
	if len(arguments) < 3 or arguments[0] != "--cards":
		print(__doc__)
		return 1
	cards_folder = os.path.abspath(arguments[1])
	cards = {}			# image -> [(file, path on the card, lines)]
	runs = []			# (file, text)
	removed = []

	def add(image, program, path, lines):
		entries = cards.setdefault(image, [])
		if (program, path, lines) not in entries:
			entries.append((program, path, lines))

	for build in arguments[2:]:
		directory = os.path.dirname(os.path.abspath(build))
		project = open(os.path.splitext(build)[0] + ".vcxproj", encoding="utf-8-sig").read()
		linked = re.search(r"<(Executable|Module|Image)>([^<]+)</\1>", project)
		if not linked:
			sys.exit("%s: its project links nothing (run hcp65_groups.py first)" % build)
		name = re.sub(r"^HCP65", "", os.path.splitext(os.path.basename(build))[0])
		for match in CONFIGURATION.finditer(open(build).read()):
			values = attributes(match.group(1))
			configuration, platform = values["Name"].split("|")
			unknown = [k for k in values if k not in ("Name", "Toolchain", "Solution", "BreakpointSpace", "Card", "CardPath")]
			body = re.sub(r"<(Load|Lines)\b[^>]*/>|</Configuration>|<!--.*?-->", "", match.group(3), flags=re.S).strip()
			if unknown or body:
				sys.exit("%s: %s still has %s" % (build, values["Name"], ", ".join(unknown) or body))

			lines = ['<RunConfiguration Name=%s Configuration=%s' % (quoteattr("%s %s" % (name, configuration)), quoteattr(values["Name"]))]
			image = None
			if "Card" in values:
				image = full(directory, values["Card"])
				card_file = os.path.join(cards_folder, os.path.splitext(os.path.basename(image))[0] + ".hcp65card")
				lines[0] += ' Card=%s' % quoteattr(windows(card_file, directory))
				program = os.path.join(directory, "Output", "$(Platform)", "$(Configuration)", linked.group(2))
				add(image, program, values["CardPath"], "")
			if values.get("BreakpointSpace", "") != SPACES[linked.group(1)]:
				lines[0] += ' BreakpointSpace=%s' % quoteattr(values.get("BreakpointSpace", ""))
			loads = [attributes(l) for l in re.findall(r"<Load\b([^>]*)/>", match.group(3))]
			for other in [attributes(l) for l in re.findall(r"<Lines\b([^>]*)/>", match.group(3))]:
				if image is None or "CardPath" not in other:
					sys.exit("%s: %s has a <Lines> with no card to be on" % (build, values["Name"]))
				add(image, full(directory, other["File"]), other["CardPath"], other.get("Space", "Kernel"))
			if loads:
				lines[0] += ">"
				lines += ['  <Load File=%s Address=%s/>' % (quoteattr(l["File"]), quoteattr(l["Address"])) for l in loads]
				lines.append("</RunConfiguration>")
			else:
				lines[0] += "/>"
			runs.append((os.path.join(directory, ".run", "%s %s.hcp65run" % (name, configuration)), "\n".join(lines) + "\n"))
		removed.append(build)

	for image, entries in sorted(cards.items()):
		card_file = os.path.join(cards_folder, os.path.splitext(os.path.basename(image))[0] + ".hcp65card")
		lines = ['<Card Image=%s>' % quoteattr(windows(image, cards_folder))]
		for program, path, space in sorted(entries, key=lambda e: (e[1].lower(), e[2])):
			lines.append('  <Program File=%s Path=%s%s/>' % (quoteattr(windows(program, cards_folder)), quoteattr(path), (' Lines=%s' % quoteattr(space)) if space else ""))
		lines.append("</Card>")
		runs.append((card_file, "\n".join(lines) + "\n"))

	for path, text in runs:
		print("%s %s" % ("wrote" if write else "would write", os.path.relpath(path)))
		if write:
			os.makedirs(os.path.dirname(path), exist_ok=True)
			open(path, "w", newline="\n").write('<?xml version="1.0" encoding="UTF-8"?>\n' + text)
	for path in removed:
		print("%s %s" % ("removed" if write else "would remove", os.path.relpath(path)))
		if write:
			os.remove(path)
	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
