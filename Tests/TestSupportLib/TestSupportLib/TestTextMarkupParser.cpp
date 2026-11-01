#include "BaseLib/StringHelper.h"
#include "SupportLib/TextMarkupParser.h"
#include "TestLib/Assert.h"


#define TEST_MARKUP_UNDERLINE	1
#define TEST_MARKUP_COLOUR		2


//A markup made up for the tests: "<u>...</u>" underlines, "<r>...</r>" is red and "&lt;" is a "<".
class CTestTextMarkupParser : public CTextMarkupParser
{
protected:
	size ReadOpen(const char* szMarkup, STextStyleOverride* psOverride, int* piKind) override
	{
		if (strncmp(szMarkup, "<u>", 3) == 0)
		{
			psOverride->SetUnderline(true);
			*piKind = TEST_MARKUP_UNDERLINE;
			return 3;
		}
		if (strncmp(szMarkup, "<r>", 3) == 0)
		{
			psOverride->SetColour(0xFFFF0000);
			*piKind = TEST_MARKUP_COLOUR;
			return 3;
		}
		return 0;
	}

	size ReadClose(const char* szMarkup, int iKind) override
	{
		if ((iKind == TEST_MARKUP_UNDERLINE) && (strncmp(szMarkup, "</u>", 4) == 0))
		{
			return 4;
		}
		if ((iKind == TEST_MARKUP_COLOUR) && (strncmp(szMarkup, "</r>", 4) == 0))
		{
			return 4;
		}
		return 0;
	}

	size ReadEscape(const char* szMarkup, CChars* pszText) override
	{
		if (strncmp(szMarkup, "&lt;", 4) == 0)
		{
			pszText->Append('<');
			return 4;
		}
		return 0;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTextStyleRun(size uiStart, size uiLength, uint16 uiSet, STextStyleRun* psRun)
{
	AssertSize(uiStart, psRun->uiStart);
	AssertSize(uiLength, psRun->uiLength);
	AssertInt(uiSet, psRun->sOverride.uiSet);
}


//////////////////////////////////////////////////////////////////////////
//
// Text outside any group is one run that changes nothing, and text
// without marks is its own plain text.
//
//////////////////////////////////////////////////////////////////////////
void TestTextMarkupParserPlain(void)
{
	CTestTextMarkupParser	cParser;
	CChars					szText;
	CArrayTextStyleRun		asRuns;

	cParser.Init();
	szText.Init();
	asRuns.Init();

	AssertTrue(cParser.Parse("Plain text", &szText, &asRuns));
	AssertString("Plain text", szText.Text());
	AssertSize(1, asRuns.NumElements());
	AssertTextStyleRun(0, 10, 0, asRuns.Get(0));

	szText.ReInit();
	asRuns.ReInit();
	AssertTrue(cParser.Parse("", &szText, &asRuns));
	AssertString("", szText.Text());
	AssertSize(0, asRuns.NumElements());

	asRuns.Kill();
	szText.Kill();
	cParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Groups make runs in their changes; nested groups have the changes of
// the groups around them too.  A close of another kind of group is plain
// text, as is a mark the markup doesn't have.
//
//////////////////////////////////////////////////////////////////////////
void TestTextMarkupParserGroups(void)
{
	CTestTextMarkupParser	cParser;
	CChars					szText;
	CArrayTextStyleRun		asRuns;
	STextStyle				sStyle;

	cParser.Init();
	szText.Init();
	asRuns.Init();

	AssertTrue(cParser.Parse("a<u>bc<r>d</r>e</u>f", &szText, &asRuns));
	AssertString("abcdef", szText.Text());
	AssertSize(5, asRuns.NumElements());
	AssertTextStyleRun(0, 1, 0, asRuns.Get(0));
	AssertTextStyleRun(1, 2, TSO_Underline, asRuns.Get(1));
	AssertTextStyleRun(3, 1, TSO_Underline | TSO_Colour, asRuns.Get(2));
	AssertTextStyleRun(4, 1, TSO_Underline, asRuns.Get(3));
	AssertTextStyleRun(5, 1, 0, asRuns.Get(4));

	sStyle.Init(false, false, 10.0f, 0xFFFFFFFF);
	asRuns.Get(2)->sOverride.ApplyTo(&sStyle);
	AssertTrue(sStyle.bUnderline);
	AssertTrue(sStyle.sColour == 0xFFFF0000);

	szText.ReInit();
	asRuns.ReInit();
	AssertTrue(cParser.Parse("<u>x</r>y<b>z</u>", &szText, &asRuns));
	AssertString("x</r>y<b>z", szText.Text());
	AssertSize(1, asRuns.NumElements());
	AssertTextStyleRun(0, 10, TSO_Underline, asRuns.Get(0));

	asRuns.Kill();
	szText.Kill();
	cParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Escapes are plain text, even when they'd be marks.  A group not closed
// by the end is still in its style, and Parse is false.  Runs are
// positions in the text after what was already in it.
//
//////////////////////////////////////////////////////////////////////////
void TestTextMarkupParserEscapesAndUnclosed(void)
{
	CTestTextMarkupParser	cParser;
	CChars					szText;
	CArrayTextStyleRun		asRuns;

	cParser.Init();
	szText.Init();
	asRuns.Init();

	AssertTrue(cParser.Parse("&lt;u>", &szText, &asRuns));
	AssertString("<u>", szText.Text());
	AssertSize(1, asRuns.NumElements());
	AssertTextStyleRun(0, 3, 0, asRuns.Get(0));

	szText.ReInit();
	szText.Append("Pre ");
	asRuns.ReInit();
	AssertFalse(cParser.Parse("x<u>yz", &szText, &asRuns));
	AssertString("Pre xyz", szText.Text());
	AssertSize(2, asRuns.NumElements());
	AssertTextStyleRun(4, 1, 0, asRuns.Get(0));
	AssertTextStyleRun(5, 2, TSO_Underline, asRuns.Get(1));

	asRuns.Kill();
	szText.Kill();
	cParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextMarkupParser(void)
{
	BeginTests();

	TestTextMarkupParserPlain();
	TestTextMarkupParserGroups();
	TestTextMarkupParserEscapesAndUnclosed();

	TestStatistics();
}

