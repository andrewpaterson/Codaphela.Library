#include "BaseLib/Logger.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/YAMLParser.h"
#include "BaseLib/YAMLFile.h"
#include "BaseLib/JSONParser.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Parses szYAML and prints the markup it made into psz (Init'ed here).
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseAndPrintYAML(const char* szYAML, CMarkup* pcMarkup, CChars* psz)
{
	CYAMLParser		cYAMLParser;
	TRISTATE		tResult;

	pcMarkup->Init();
	cYAMLParser.Init(szYAML, "InMemory", pcMarkup, &gcLogger);
	tResult = cYAMLParser.Parse();
	cYAMLParser.Kill();

	psz->Init();
	pcMarkup->Print(psz);
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
// szYAML fails to parse and the logged error contains szMessage.
//
//////////////////////////////////////////////////////////////////////////
void AssertYAMLError(const char* szYAML, const char* szMessage)
{
	CYAMLParser		cYAMLParser;
	CMarkup			cMarkup;
	CLogger			cLogger;
	CMemoryFile		cMemory;
	SLogConfig		sLogConfig;
	TRISTATE		tResult;
	char			szZero[1] = { 0 };
	const char*		szLog;

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cLogger.Init(&cMemory, NULL);
	sLogConfig = cLogger.SetSilent();
	cMarkup.Init();
	cYAMLParser.Init(szYAML, "InMemory", &cMarkup, &cLogger);
	tResult = cYAMLParser.Parse();
	cMemory.Write(szZero, 1, 1);

	if (tResult != TRIERROR)
	{
		//Parsed without the error: name the document that should have failed.
		AssertString(szMessage, szYAML);
	}
	szLog = (const char*)cMemory.GetBufferPointer();
	if (StrIStr(szLog, szMessage) == NULL)
	{
		AssertString(szMessage, szLog);
	}

	cLogger.SetConfig(&sLogConfig);
	cYAMLParser.Kill();
	cMarkup.Kill();
	cLogger.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserScalars(void)
{
	CMarkup			cMarkup;
	CChars			sz;
	CMarkupTag*		pcRoot;
	const char*		szYAML = "\
name: Codaphela\n\
count: 42\n\
negative: -7\n\
ratio: 1.25\n\
on: true\n\
off: False\n\
nothing: ~\n\
null word: null\n\
empty:\n\
word: 'true'\n\
quoted: \"a: b # c\"\n\
hex: 0x1F\n\
version: 1.2.3\n\
spaced: some words here   # a comment\n\
hash: a#b\n\
url: https://example.com/a?b=c\n";

	AssertTritrue(ParseAndPrintYAML(szYAML, &cMarkup, &sz));
	AssertString(
		"[name=\"Codaphela\", count=42, negative=-7, ratio=1.25, on=true, off=false, nothing=null, null word=null, empty=null, word=\"true\", "
		"quoted=\"a: b # c\", hex=\"0x1F\", version=\"1.2.3\", spaced=\"some words here\", hash=\"a#b\", url=\"https://example.com/a?b=c\"]\n", sz.Text());
	sz.Kill();

	pcRoot = cMarkup.mpcDoc->GetRootTag();
	AssertString("Codaphela", pcRoot->GetStringAttribute("name"));
	AssertString("some words here", pcRoot->GetStringAttribute("spaced"));
	AssertString("https://example.com/a?b=c", pcRoot->GetStringAttribute("url"));

	cMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The shape of mkdocs.yml: nested mappings, sequences, and sequences of
// compact "- key: value" mappings.
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserNested(void)
{
	CMarkup			cMarkup;
	CChars			sz;
	CMarkupTag*		pcRoot;
	CMarkupTag*		pcSources;
	CMarkupTag*		pcSource;
	STagIterator	sIter;
	const char*		szYAML = "\
site_name: Codaphela\n\
theme:\n\
  name: material\n\
  features:\n\
    - navigation.tabs\n\
    - navigation.top\n\
\n\
# The map of docs folders.\n\
extra:\n\
  sources:\n\
    - folder: HCP65-IDE/HCP65IDE/Docs\n\
      site: ide\n\
      branch: main\n\
    - folder: CommonLibraries/Libraries/CppCompilerLib/Docs\n\
      site: compiler\n\
      branch: master\n\
nav:\n\
  - index.md\n\
  - HCP65 IDE:\n\
      - ide/index.md\n\
      - Editor: ide/editor.md\n\
  -\n\
    nested: on the next line\n";

	AssertTritrue(ParseAndPrintYAML(szYAML, &cMarkup, &sz));
	AssertString(
		"<site_name=\"Codaphela\">\n"
		"  <theme name=\"material\">\n"
		"    <features \"navigation.tabs\", \"navigation.top\"/>\n"
		"  </theme>\n"
		"  <extra>\n"
		"    <sources>\n"
		"      [folder=\"HCP65-IDE/HCP65IDE/Docs\", site=\"ide\", branch=\"main\"]\n"
		"      [folder=\"CommonLibraries/Libraries/CppCompilerLib/Docs\", site=\"compiler\", branch=\"master\"]\n"
		"    </sources>\n"
		"  </extra>\n"
		"  <nav \"index.md\">\n"
		"    <>\n"
		"      <HCP65 IDE \"ide/index.md\">\n"
		"        [Editor=\"ide/editor.md\"]\n"
		"      </HCP65 IDE>\n"
		"    </>\n"
		"    [nested=\"on the next line\"]\n"
		"  </nav>\n"
		"</>\n", sz.Text());
	sz.Kill();

	pcRoot = cMarkup.mpcDoc->GetRootTag();
	pcSources = pcRoot->GetTag("extra")->GetTag("sources");
	AssertNotNull(pcSources);
	pcSource = pcSources->GetTag(&sIter);
	AssertString("HCP65-IDE/HCP65IDE/Docs", pcSource->GetStringAttribute("folder"));
	AssertString("ide", pcSource->GetStringAttribute("site"));
	pcSource = pcSources->GetNextTag(&sIter);
	AssertString("compiler", pcSource->GetStringAttribute("site"));
	AssertString("master", pcSource->GetStringAttribute("branch"));
	AssertNull(pcSources->GetNextTag(&sIter));

	cMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A sequence may sit at its key's own indent.
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserSameIndentSequence(void)
{
	CMarkup			cMarkup;
	CChars			sz;
	const char*		szYAML = "\
plugins:\n\
- search\n\
- glightbox\n\
markdown_extensions:\n\
- toc:\n\
    permalink: true\n\
- admonition\n\
- - inner\n\
  - list\n\
last: 1\n";

	AssertTritrue(ParseAndPrintYAML(szYAML, &cMarkup, &sz));
	AssertString(
		"<last=1>\n"
		"  <plugins \"search\", \"glightbox\"/>\n"
		"  <markdown_extensions \"admonition\">\n"
		"    <>\n"
		"      <toc permalink=true/>\n"
		"    </>\n"
		"    [\"inner\", \"list\"]\n"
		"  </markdown_extensions>\n"
		"</>\n", sz.Text());
	sz.Kill();
	cMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserFlow(void)
{
	CMarkup			cMarkup;
	CChars			sz;
	const char*		szYAML = "\
list: [a, 'b c', \"d\", 1, true, null]\n\
map: {x: 1, \"y\": [2, 3], z: }\n\
empty_list: []\n\
empty_map: {}\n\
multi: [\n\
  one,   # first\n\
  two,\n\
]\n";

	AssertTritrue(ParseAndPrintYAML(szYAML, &cMarkup, &sz));
	AssertString(
		"<>\n"
		"  <list \"a\", \"b c\", \"d\", 1, true, null/>\n"
		"  <map x=1, z=null>\n"
		"    <y 2, 3/>\n"
		"  </map>\n"
		"  <empty_list/>\n"
		"  <empty_map/>\n"
		"  <multi \"one\", \"two\"/>\n"
		"</>\n", sz.Text());
	sz.Kill();
	cMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserQuoted(void)
{
	CMarkup			cMarkup;
	CChars			sz;
	CMarkupTag*		pcRoot;
	const char*		szYAML = "\
double: \"tab\\there \\\"quoted\\\" back\\\\slash \\x41\\u00e9\\U0001F600\"\n\
single: 'it''s # not a comment'\n\
colon: \"key: inside\"\n\
\"quoted key\": 1\n\
'single key': 2\n\
\"#includes\": ide/refactoring/includes.md\n";

	AssertTritrue(ParseAndPrintYAML(szYAML, &cMarkup, &sz));
	sz.Kill();

	pcRoot = cMarkup.mpcDoc->GetRootTag();
	AssertString("tab\there \"quoted\" back\\slash A\xC3\xA9\xF0\x9F\x98\x80", pcRoot->GetStringAttribute("double"));
	AssertString("it's # not a comment", pcRoot->GetStringAttribute("single"));
	AssertString("key: inside", pcRoot->GetStringAttribute("colon"));
	AssertTrue(pcRoot->GetAttribute("quoted key", NULL, NULL));
	AssertTrue(pcRoot->GetAttribute("single key", NULL, NULL));
	AssertString("ide/refactoring/includes.md", pcRoot->GetStringAttribute("#includes"));

	cMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Tagged scalars are kept as text: the tag, a space and the value.
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserTags(void)
{
	CMarkup			cMarkup;
	CChars			sz;
	CMarkupTag*		pcRoot;
	CMarkupTag*		pcFence;
	STagIterator	sIter;
	const char*		szYAML = "\
markdown_extensions:\n\
  - pymdownx.superfences:\n\
      custom_fences:\n\
        - name: mermaid\n\
          class: mermaid\n\
          format: !!python/name:pymdownx.superfences.fence_code_format\n\
text: !!str 12\n\
quoted: !custom 'a b'\n";

	AssertTritrue(ParseAndPrintYAML(szYAML, &cMarkup, &sz));
	sz.Kill();

	pcRoot = cMarkup.mpcDoc->GetRootTag();
	pcFence = pcRoot->GetTag("markdown_extensions")->GetTag(&sIter)->GetTag("pymdownx.superfences")->GetTag("custom_fences")->GetTag(&sIter);
	AssertNotNull(pcFence);
	AssertString("mermaid", pcFence->GetStringAttribute("class"));
	AssertString("!!python/name:pymdownx.superfences.fence_code_format", pcFence->GetStringAttribute("format"));
	AssertString("!!str 12", pcRoot->GetStringAttribute("text"));
	AssertString("!custom a b", pcRoot->GetStringAttribute("quoted"));

	cMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserDocuments(void)
{
	CMarkup			cMarkup;
	CChars			sz;

	AssertTritrue(ParseAndPrintYAML("---\na: 1\n...\n", &cMarkup, &sz));
	AssertString("[a=1]\n", sz.Text());
	sz.Kill();
	cMarkup.Kill();

	AssertTritrue(ParseAndPrintYAML("--- # a comment\n- x\n- y\n", &cMarkup, &sz));
	AssertString("[\"x\", \"y\"]\n", sz.Text());
	sz.Kill();
	cMarkup.Kill();

	AssertTritrue(ParseAndPrintYAML("hello world\n", &cMarkup, &sz));
	AssertString("[\"hello world\"]\n", sz.Text());
	sz.Kill();
	cMarkup.Kill();

	AssertTritrue(ParseAndPrintYAML("[1, 2]", &cMarkup, &sz));
	AssertString("[1, 2]\n", sz.Text());
	sz.Kill();
	cMarkup.Kill();

	AssertTritrue(ParseAndPrintYAML("", &cMarkup, &sz));
	AssertNull(cMarkup.mpcDoc->GetRootTag());
	sz.Kill();
	cMarkup.Kill();

	AssertTritrue(ParseAndPrintYAML("# only a comment\n\n", &cMarkup, &sz));
	AssertNull(cMarkup.mpcDoc->GetRootTag());
	sz.Kill();
	cMarkup.Kill();

	AssertTritrue(ParseAndPrintYAML("a: no final line end", &cMarkup, &sz));
	AssertString("[a=\"no final line end\"]\n", sz.Text());
	sz.Kill();
	cMarkup.Kill();

	AssertTritrue(ParseAndPrintYAML("\xEF\xBB\xBF" "a: 1\nb:\n  - c\n", &cMarkup, &sz));
	AssertString("<a=1>\n  <b \"c\"/>\n</>\n", sz.Text());
	sz.Kill();
	cMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// JSON is YAML: the two parsers make the same markup from it.
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserReadsJSON(void)
{
	CMarkup			cJSONMarkup;
	CMarkup			cYAMLMarkup;
	CJSONParser		cJSONParser;
	CChars			szJSON;
	CChars			szYAML;
	size			i;
	const char*		aszDocuments[] = {
		"{\"widget\": {\"on\": true, \"off\": false, \"parent\": null, \"word\": \"true\", \"list\": [true, false, null]}}\n",
		"{\n\"glossary\": {\n\t\"title\": \"example glossary\",\n\t\t\"GlossDiv\" : {\n\t\t\t\"title\": \"S\",\n\t\t\t\"GlossList\": [\"GML\", \"XML\", {\"para\": \"A \\\"meta\\\" language\"}]\n\t\t}\n\t}\n}\n",
		"{\"painting\": [{\"img@src\": \"madonna.jpg\", \"size\": 12.5}, {\"caption\": [\"This is\", {\"date\": \"1511\"}, \"-\", {\"date\": 1512}]}]}",
	};

	for (i = 0; i < 3; i++)
	{
		cJSONMarkup.Init();
		cJSONParser.Init(aszDocuments[i], "", &cJSONMarkup, &gcLogger);
		AssertTritrue(cJSONParser.Parse());
		cJSONParser.Kill();
		szJSON.Init();
		cJSONMarkup.Print(&szJSON);

		AssertTritrue(ParseAndPrintYAML(aszDocuments[i], &cYAMLMarkup, &szYAML));
		AssertString(szJSON.Text(), szYAML.Text());

		szYAML.Kill();
		szJSON.Kill();
		cYAMLMarkup.Kill();
		cJSONMarkup.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParserErrors(void)
{
	AssertYAMLError("a: 1\na: 2\n", "InMemory(1,2): Duplicate key 'a'");
	AssertYAMLError("a:\n  b: 1\n  b:\n    c: 2\n", "Duplicate key 'b'");
	AssertYAMLError("a: &x 1\n", "Anchors (&) are not supported");
	AssertYAMLError("a: *x\n", "Aliases (*) are not supported");
	AssertYAMLError("a: |\n  text\n", "Block scalars (| and >) are not supported");
	AssertYAMLError("a: >\n  text\n", "Block scalars (| and >) are not supported");
	AssertYAMLError("? a\n: b\n", "Complex keys (?) are not supported");
	AssertYAMLError("a: one\n  two\n", "Unexpected indentation");
	AssertYAMLError("a:\n  one\n  two\n", "Unexpected indentation");
	AssertYAMLError("a:\n  - x\n  y: 1\n", "Unexpected indentation");
	AssertYAMLError("a:\n\tb: 1\n", "Tabs can't be used to indent");
	AssertYAMLError("a: 1\n---\nb: 2\n", "Only one document per file is supported");
	AssertYAMLError("%YAML 1.2\n---\na: 1\n", "Directives (%) are not supported");
	AssertYAMLError("a: 'open\nb: 1\n", "A quoted value must end on its own line");
	AssertYAMLError("a: \"open\n", "A quoted value must end on its own line");
	AssertYAMLError("a: \"bad \\q\"\n", "Unknown escape");
	AssertYAMLError("a: \"\\u00G1\"\n", "Expected a hexadecimal digit");
	AssertYAMLError("a: [1, 2\n", "Expected the end of a [ ] or { } collection");
	AssertYAMLError("a: [1 2 :]\n", "Expected ',' or ']'");
	AssertYAMLError("a: {x: [1] y}\n", "Expected ',' or '}'");
	AssertYAMLError("a: [x, !tag y]\n", "Tags inside [ ] and { } are not supported");
	AssertYAMLError("a: !tag [x]\n", "Tags on [ ] and { } collections are not supported");
	AssertYAMLError("a: b: c\n", "A value can't contain ': '");
	AssertYAMLError("a: [x, y] z\n", "Expected the end of the line");
	AssertYAMLError("a: 'x'#y\n", "A comment needs a space before its #");
	AssertYAMLError("- a\nb: 1\n", "Expected the end of the document");
	AssertYAMLError("a: - b\n", "A '- ' entry can't start on its key's line");
	AssertYAMLError("a: @b\n", "A value can't start with %, @ or `");
	AssertYAMLError("'': 1\n", "Empty keys are not supported");
	AssertYAMLError("--- a\n", "A value on the --- line is not supported");
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the website's mkdocs.yml with an extra.sources map, and a
// file with Windows line ends.
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLFileRead(void)
{
	CYAMLFile		cYAMLFile;
	CMarkupTag*		pcRoot;
	CMarkupTag*		pcTag;
	CMarkupTag*		pcFence;
	CMarkupTag*		pcExtension;
	STagIterator	sIter;
	STagIterator	sFenceIter;
	size			uiCount;

	cYAMLFile.Init();
	AssertTrue(cYAMLFile.Read("mkdocs.yml", "Input/YAML"));
	pcRoot = cYAMLFile.mcMarkup.mpcDoc->GetRootTag();
	AssertNotNull(pcRoot);
	AssertString("Codaphela", pcRoot->GetStringAttribute("site_name"));
	AssertString("material", pcRoot->GetTag("theme")->GetStringAttribute("name"));

	pcTag = pcRoot->GetTag("nav");
	AssertNotNull(pcTag);
	AssertString("index.md", pcTag->GetTag(&sIter)->GetStringAttribute("Home"));
	uiCount = 1;
	while (pcTag->GetNextTag(&sIter))
	{
		uiCount++;
	}
	AssertSize(6, uiCount);

	pcTag = pcRoot->GetTag("markdown_extensions");
	AssertNotNull(pcTag);
	pcFence = NULL;
	for (pcExtension = pcTag->GetTag(&sIter); pcExtension; pcExtension = pcTag->GetNextTag(&sIter))
	{
		if (pcExtension->GetTag("pymdownx.superfences"))
		{
			pcFence = pcExtension->GetTag("pymdownx.superfences")->GetTag("custom_fences")->GetTag(&sFenceIter);
		}
	}
	AssertNotNull(pcFence);
	AssertString("mermaid", pcFence->GetStringAttribute("name"));
	AssertString("!!python/name:pymdownx.superfences.fence_code_format", pcFence->GetStringAttribute("format"));

	pcTag = pcRoot->GetTag("extra")->GetTag("sources");
	AssertNotNull(pcTag);
	AssertString("HCP65-IDE/HCP65IDE/Docs", pcTag->GetTag(&sIter)->GetStringAttribute("folder"));
	AssertString("master", pcTag->GetNextTag(&sIter)->GetStringAttribute("branch"));
	cYAMLFile.Kill();

	cYAMLFile.Init();
	AssertTrue(cYAMLFile.Read("CRLF.yml", "Input/YAML"));
	pcRoot = cYAMLFile.mcMarkup.mpcDoc->GetRootTag();
	AssertString("Windows", pcRoot->GetStringAttribute("name"));
	pcTag = pcRoot->GetTag("list");
	AssertNotNull(pcTag);
	AssertSize(2, pcTag->GetNumAttributes());
	cYAMLFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestYAMLParser(void)
{
	BeginTests();
	DataMemoryInit();

	TestYAMLParserScalars();
	TestYAMLParserNested();
	TestYAMLParserSameIndentSequence();
	TestYAMLParserFlow();
	TestYAMLParserQuoted();
	TestYAMLParserTags();
	TestYAMLParserDocuments();
	TestYAMLParserReadsJSON();
	TestYAMLParserErrors();
	TestYAMLFileRead();

	DataMemoryKill();
	TestStatistics();
}

