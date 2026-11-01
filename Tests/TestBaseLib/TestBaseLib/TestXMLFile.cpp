#include "TestLib/Assert.h"
#include "BaseLib/XMLFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestXMLFileRead(void)
{
	CXMLFile	cXMLFile;
	CChars		szDoc;
	size		iLine;
	char		szExpected[] = "<InputDevices>\n\
  This is text &lt; and doom stuff\n\
  <RootInSecond>\n\
    Hello\n\
  </RootInSecond>\n\
  Sweet &gt; other stuff\n\
  <ThirdsRoot>\n\
    World\n\
  </ThirdsRoot>\n\
  <Impact>Genesis Evangelion</Impact>\n\
</InputDevices>\n";
	CMarkupTag* pcTag;
	CMarkupTag* pcSecondTag;
	CMarkupTag* pcThirdTag;
	CChars		szText;

	cXMLFile.Init();
	cXMLFile.Read("FirstExpected.xml", ".");

	szDoc.Init();
	iLine = cXMLFile.mcMarkup.mpcDoc->Print(&szDoc);
	AssertSize(11, iLine);
	AssertString(szExpected, szDoc.Text());
	szDoc.Kill();

	pcTag = cXMLFile.mcMarkup.mpcDoc->GetRootTag();
	pcSecondTag = pcTag->GetTag("RootInSecond");
	AssertNotNull(pcSecondTag);
	szText.Init();
	pcSecondTag->GetText(&szText);
	szText.StripWhitespace(true);
	AssertString("Hello", szText.Text());
	szText.Kill();

	pcTag = cXMLFile.mcMarkup.mpcDoc->GetRootTag();
	pcThirdTag = pcTag->GetTag("ThirdsRoot");
	AssertNotNull(pcThirdTag);
	szText.Init();
	pcThirdTag->GetText(&szText);
	szText.StripWhitespace(true);
	AssertString("World", szText.Text());
	szText.Kill();

	cXMLFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestXMLFileReadExternalReference(void)
{
	CXMLFile	cXMLFile;
	CChars		szDoc;
	size		iLine;
	char		szExpected[] = "<InputDevices>\n\
  This is text &gt; and doom stuff\n\
  <RootInSecond>\n\
    Hello\n\
  </RootInSecond>\n\
  Sweet &lt; other stuff\n\
  <ThirdsRoot>\n\
    World\n\
  </ThirdsRoot>\n\
  <Impact>Genesis Evangelion</Impact>\n\
</InputDevices>\n";
	CMarkupTag*	pcTag;
	CMarkupTag*	pcSecondTag;
	CMarkupTag*	pcThirdTag;
	CChars		szText;

	cXMLFile.Init();
	cXMLFile.Read("First.xml", ".");

	szDoc.Init();
	iLine = cXMLFile.mcMarkup.mpcDoc->Print(&szDoc);
	AssertSize(11, iLine);
	AssertString(szExpected, szDoc.Text());
	szDoc.Kill();

	pcTag = cXMLFile.mcMarkup.mpcDoc->GetRootTag();
	pcSecondTag = pcTag->GetTag("RootInSecond");
	AssertNotNull(pcSecondTag);
	szText.Init();
	pcSecondTag->GetText(&szText);
	szText.StripWhitespace(true);
	AssertString("Hello", szText.Text());
	szText.Kill();

	pcTag = cXMLFile.mcMarkup.mpcDoc->GetRootTag();
	pcThirdTag = pcTag->GetTag("ThirdsRoot");
	AssertNotNull(pcThirdTag);
	szText.Init();
	pcThirdTag->GetText(&szText);
	szText.StripWhitespace(true);
	AssertString("World", szText.Text());
	szText.Kill();

	cXMLFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestXMLFileRepeatedExternalTags(void)
{
	CXMLFile		cXMLFile;
	CChars			szDoc;
	size			iLine;
	char			szExpected[] = "<Root>\n\
  <ExtTag>\n\
    <SubTag>Look at me!  I&apos;m going to be an elite pilot.</SubTag>\n\
    <SubTag>Also *very* messed up.</SubTag>\n\
  </ExtTag>\n\
  <ExtTag>\n\
    <SubTag>Look at me!  I&apos;m going to be an elite pilot.</SubTag>\n\
    <SubTag>Also *very* messed up.</SubTag>\n\
  </ExtTag>\n\
  <ExtTag>\n\
    <SubTag>Look at me!  I&apos;m going to be an elite pilot.</SubTag>\n\
    <SubTag>Also *very* messed up.</SubTag>\n\
  </ExtTag>\n\
  <ExtTag>\n\
    <SubTag>Look at me!  I&apos;m going to be an elite pilot.</SubTag>\n\
    <SubTag>Also *very* messed up.</SubTag>\n\
  </ExtTag>\n\
</Root>\n";
	CMarkupTag*		pcRoot;
	size			iCount;
	CMarkupTag*		pcExtTag;
	CMarkupTag*		pcSubTag1;
	CMarkupTag*		pcSubTag2;
	CMarkupTag*		pcSubTag3;
	CChars			szText;
	STagIterator	sIter;

	cXMLFile.Init();
	cXMLFile.Read("File.xml", ".");

	szDoc.Init();
	iLine = cXMLFile.mcMarkup.mpcDoc->Print(&szDoc);
	AssertSize(18, iLine);
	AssertString(szExpected, szDoc.Text());
	szDoc.Kill();

	pcRoot = cXMLFile.mcMarkup.mpcDoc->GetRootTag();

	iCount = 0;
	pcExtTag = pcRoot->GetTag("ExtTag", &sIter);
	while (pcExtTag)
	{
		iCount++;

		if (iCount > 4)
		{
			//This is to take care of the case where GetNextTag always get's the first tag.
			break;
		}

		pcSubTag1 = pcExtTag->GetTag("SubTag", (size)0);
		pcSubTag2 = pcExtTag->GetTag("SubTag", 1);
		pcSubTag3 = pcExtTag->GetTag("SubTag", 2);

		szText.Init();
		pcSubTag1->GetText(&szText);
		AssertString("Look at me!  I'm going to be an elite pilot.", szText.Text());
		szText.Kill();		
		
		szText.Init();
		pcSubTag2->GetText(&szText);
		AssertString("Also *very* messed up.", szText.Text());
		szText.Kill();

		AssertNull(pcSubTag3);

		pcExtTag = pcRoot->GetNextTag(&sIter);
	}

	AssertSize(4, iCount);
	cXMLFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestXMLFile(void)
{
	BeginTests();
	DataMemoryInit();

	TestXMLFileRead();
	TestXMLFileReadExternalReference();
	TestXMLFileRepeatedExternalTags();

	DataMemoryKill();
	TestStatistics();
}

