#include "BaseLib/XMLWriter.h"
#include "BaseLib/XMLFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DataMemory.h"
#include "BaseLib/TextFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Elements without children close themselves; text and attributes are
// escaped.
//
//////////////////////////////////////////////////////////////////////////
void TestXMLWriterText(void)
{
	CXMLWriter	cWriter;
	char		szExpected[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<Root version=\"1\">\n\
  <Empty/>\n\
  <Item name=\"a &lt;b&gt; &amp; &quot;c&quot; &apos;d&apos;\" on=\"true\" off=\"false\" big=\"-9000000000\">\n\
    <Child x=\"-3\"/>\n\
    Some &lt;text&gt; &amp; \"quotes\"\n\
  </Item>\n\
</Root>\n";

	cWriter.Init();
	cWriter.Begin("Root");
	cWriter.Attribute("version", (int32)1);
	cWriter.Begin("Empty");
	cWriter.End();
	cWriter.Begin("Item");
	cWriter.Attribute("name", "a <b> & \"c\" 'd'");
	cWriter.Attribute("on", true);
	cWriter.Attribute("off", false);
	cWriter.Attribute("big", (int64)-9000000000LL);
	cWriter.Begin("Child");
	cWriter.Attribute("x", (int32)-3);
	cWriter.End();
	cWriter.Text("Some <text> & \"quotes\"");
	AssertFalse(cWriter.IsComplete());
	cWriter.End();
	cWriter.End();
	AssertTrue(cWriter.IsComplete());
	AssertString(szExpected, cWriter.GetText());

	//Ending more than was begun does nothing.
	cWriter.End();
	AssertString(szExpected, cWriter.GetText());
	cWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What is written reads back through CXMLFile with the same values.
//
//////////////////////////////////////////////////////////////////////////
void TestXMLWriterReadBack(void)
{
	CXMLWriter	cWriter;
	CXMLFile	cFile;
	CFileUtil	cFileUtil;
	CMarkupTag*	pcRoot;
	CMarkupTag*	pcItem;
	CMarkupTag*	pcSecond;
	int32		i32;
	int64		i64;
	bool		b;
	float32		f32;
	float64		f64;
	STagIterator	sIter;

	cWriter.Init();
	cWriter.Begin("Circuit");
	cWriter.Begin("Item");
	cWriter.Attribute("name", "U1 <&> \"x\"");
	cWriter.Attribute("x", (int32)-17);
	cWriter.Attribute("time", (int64)123456789012LL);
	cWriter.Attribute("explicit", true);
	cWriter.Attribute("volts", 3.3f);
	cWriter.Attribute("ratio", 0.1);
	cWriter.End();
	cWriter.Begin("Item");
	cWriter.Attribute("x", "nope");
	cWriter.Attribute("big", (int64)3000000000LL);
	cWriter.End();
	cWriter.End();
	cFileUtil.MakeDir("Output");
	AssertTrue(cWriter.Write("Output" _FS_ "Writer.xml"));
	cWriter.Kill();

	cFile.Init();
	AssertTrue(cFile.Read("Writer.xml", "Output"));
	pcRoot = cFile.mcMarkup.mpcDoc->GetRootTag();
	AssertNotNull(pcRoot);
	AssertTrue(pcRoot->Is("Circuit"));

	pcItem = pcRoot->GetTag("Item", &sIter);
	AssertNotNull(pcItem);
	AssertString("U1 <&> \"x\"", pcItem->GetStringAttribute("name"));
	AssertTrue(pcItem->GetInt32Attribute("x", &i32));
	AssertInt((int32)-17, i32);
	AssertTrue(pcItem->GetInt64Attribute("time", &i64));
	AssertLong(123456789012LL, i64);
	AssertTrue(pcItem->GetBoolAttribute("explicit", &b));
	AssertTrue(b);
	AssertTrue(pcItem->GetFloat32Attribute("volts", &f32));
	AssertTrue(f32 == 3.3f);
	AssertTrue(pcItem->GetFloat64Attribute("ratio", &f64));
	AssertTrue(f64 == 0.1);
	AssertNull(pcItem->GetStringAttribute("missing"));
	AssertFalse(pcItem->GetInt32Attribute("missing", &i32));
	AssertFalse(pcItem->GetBoolAttribute("name", &b));

	pcSecond = pcRoot->GetNextTag(&sIter);
	AssertNotNull(pcSecond);
	AssertFalse(pcSecond->GetInt32Attribute("x", &i32));
	AssertFalse(pcSecond->GetFloat64Attribute("x", &f64));
	AssertFalse(pcSecond->GetInt32Attribute("big", &i32));
	AssertTrue(pcSecond->GetInt64Attribute("big", &i64));
	AssertLong(3000000000LL, i64);
	AssertNull(pcRoot->GetNextTag(&sIter));
	cFile.Kill();

	cFileUtil.Delete("Output" _FS_ "Writer.xml");
	cFileUtil.RemoveDir("Output");
}


//////////////////////////////////////////////////////////////////////////
//
// Entities and character references in attribute values are replaced when
// they're read; an ampersand that isn't one is kept.
//
//////////////////////////////////////////////////////////////////////////
void TestXMLWriterReadAttributeEntities(void)
{
	CTextFile	cText;
	CXMLFile	cFile;
	CFileUtil	cFileUtil;
	CMarkupTag*	pcRoot;

	cFileUtil.MakeDir("Output");
	cText.Init("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Root a=\"&#65;&#x42;&#x63; &lt;&gt;&amp;&quot;&apos;\" b=\"&bogus; &amp &#; &#300;\"/>\n");
	AssertTrue(cText.Write("Output" _FS_ "Entities.xml"));
	cText.Kill();

	cFile.Init();
	AssertTrue(cFile.Read("Entities.xml", "Output"));
	pcRoot = cFile.mcMarkup.mpcDoc->GetRootTag();
	AssertNotNull(pcRoot);
	AssertString("ABc <>&\"'", pcRoot->GetStringAttribute("a"));
	AssertString("&bogus; &amp &#; &#300;", pcRoot->GetStringAttribute("b"));
	cFile.Kill();

	cFileUtil.Delete("Output" _FS_ "Entities.xml");
	cFileUtil.RemoveDir("Output");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestXMLWriter(void)
{
	BeginTests();
	DataMemoryInit();

	TestXMLWriterText();
	TestXMLWriterReadBack();
	TestXMLWriterReadAttributeEntities();

	DataMemoryKill();
	TestStatistics();
}

