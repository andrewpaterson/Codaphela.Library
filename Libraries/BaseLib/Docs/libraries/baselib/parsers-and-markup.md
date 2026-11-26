# Parsers and Markup

BaseLib reads XML, JSON and YAML into one tree, the markup tree, with one parser for each over one text parser.  A
program that reads a configuration doesn't care which of the three it was written in: the JSON and YAML parsers make
the same shape of tree, and code reads it the same way.

## The text parser

[`CTextParser`] walks a text held in memory: skipping white space, parsing identifiers, strings, numbers and exact
text, keeping its line and column, and saving and restoring its position so a parser can try something and back out.
How white space, strings and identifiers are recognised is configurable ([`STextParserConfig`]), so each format
gives it its own (the XML, JSON and YAML white space files, [`BaseLib/YAMLWhitespace.h`] and its neighbours).

- The text must not contain `\r`: [`CTextParser`] logs an error for one.  A file reader passes its newlines first
  (the YAML file turns `\r\n` and `\r` into `\n` before it parses).
- [`CTextParser::SkipUTF8BOM`] skips a UTF-8 byte order mark only at the very start: nothing is skipped before it,
  it takes up no column, and text without one (empty text included) isn't an error.
- [`CTextParser::GetLineLocationMarkerString`] writes the line a parser stopped on with a marker under the column.

## The markup tree

A [`CMarkup`] owns a tree's memory (a [`CDataMemoryAllocator`] of its own) and its documents.  A [`CMarkupDoc`] has
a root [`CMarkupTag`], the comments before the root (so a file written again keeps them), and the references and
substitutes an XML document's entities make ([`CMarkupReference`], [`CMarkupSubstitute`]).  A [`CMarkupTag`] has a
name, attributes by name, and its children in order: tags and text ([`CMarkupText`]).

[`CBaseParser`] is what the three parsers share: the text parser, the file's name for errors, the tag being filled,
and adding string, number ([`CNumber`]), boolean and null attributes to it.  [`CBaseParser::Error`] logs
`file(line,column): message` with the marked line; the line and column are counted from 0.

## XML, JSON and YAML

| Format | Parser | File | Writer |
|---|---|---|---|
| XML | [`CXMLParser`] (prolog, entities) | [`CXMLFile`] | [`CXMLWriter`] |
| JSON | [`CJSONParser`] | none | none |
| YAML | [`CYAMLParser`] | [`CYAMLFile`] | none |

JSON and YAML have the same data, so they make the same tree:

- A mapping is a tag: its scalar values are attributes named by their keys, its mappings and sequences child tags
  named by their keys.
- A sequence is a tag whose scalars are unnamed attributes and whose mappings and sequences are unnamed child tags.
- The root is a tag named "" (and an empty YAML document has no root).

[`CMarkup::Print`] shows a tag with no name and only attributes as `[...]`.

### YAML

[`CYAMLParser`] reads a subset of YAML 1.2, enough for the website's `mkdocs.yml` and files like it.  It was built
so that the HCP65 IDE and the docs tools can read `mkdocs.yml`'s `extra.sources` (its user is HCP65-IDE's
`IndexLib/DocsSite.cpp`).

- Read: block mappings and sequences (nested, compact `- key: value` entries, a sequence at its key's indent), flow
  `[ ]` and `{ }` collections over several lines too, single and double quoted scalars with escapes, comments, and
  one document with an optional `---` and `...`.
- Typed by the core schema: null (`~`, `null`, empty), true and false, numbers, else strings.  A number is decimal
  digits with an optional sign and fraction; hexadecimal, octal, exponents, `.inf` and `.nan` stay strings.  Quoted
  scalars are strings.  A tagged scalar (`!!python/name:x.y`) is kept as text: the tag, then a space and the value.
- Refused with an error naming the line: anchors and aliases, block scalars (`|` and `>`), complex keys (`?`), plain
  or quoted values continuing on the next line, tags on collections, directives (`%`), a second document, tabs as
  indentation, duplicate keys and empty keys.

Its tests are [`Tests/TestBaseLib/TestBaseLib/TestYAMLParser.cpp`], with the website's `mkdocs.yml` (and
`extra.sources`) and a CRLF file in `Input/YAML`.  On 2026-11-24 its tree of the website's `mkdocs.yml` was compared
with PyYAML's (SafeLoader, its Python tags kept as text), printed the same way: identical.

## Decisions

- The YAML parser is built on the XML and JSON parsers' design ([`CBaseParser`], the markup tree in JSON's shape)
  rather than as a reader of its own (the user, 2026-11-24: "build the YAML parser (with tests) based on the
  existing CXMLParser and CJSONParser", once the page-to-repository map was to go in `mkdocs.yml`'s `extra.sources`,
  DocsPlan.txt decision 13).
- [`CTextParser::SkipUTF8BOM`] was fixed at the source rather than worked round in the YAML parser: it skipped white
  space first, failed on empty text and counted the mark as three columns (2026-11-24).

## Not done

- No JSON or YAML writer.
- YAML anchors, aliases, block scalars, multi-line values and several documents aren't read.
