#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TextSearch.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Every place szPattern is found in szText, as "position,...".
//
//////////////////////////////////////////////////////////////////////////
static void AssertTextSearch(const char* szExpected, const char* szPattern, bool bMatchCase, bool bWholeWords, const char* szText)
{
	CTextSearch	cSearch;
	CChars		szActual;
	size		uiLength;
	size		uiPosition;

	cSearch.Init(szPattern, bMatchCase, bWholeWords);
	szActual.Init();
	uiLength = strlen(szText);
	uiPosition = cSearch.Find(szText, uiLength);
	while (uiPosition != TEXT_SEARCH_NOT_FOUND)
	{
		if (!szActual.Empty())
		{
			szActual.Append(",");
		}
		szActual.Append((uint32)uiPosition);
		uiPosition = cSearch.Find(szText, uiLength, uiPosition + 1);
	}
	AssertString(szExpected, szActual.Text());
	szActual.Kill();
	cSearch.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextSearchCase(void)
{
	AssertTextSearch("0,3,6", "ab", false, false, "ab AB aB");
	AssertTextSearch("0", "ab", true, false, "ab AB aB");
	AssertTextSearch("3", "AB", true, false, "ab AB aB");
	AssertTextSearch("0,3,6", "Ab", false, false, "ab AB aB");
	AssertTextSearch("", "abc", false, false, "ab AB aB");
	AssertTextSearch("0,1,2", "aa", false, false, "aAaa");
	AssertTextSearch("2", "aa", true, false, "aAaa");
	AssertTextSearch("3,8", "(1", false, false, "F  (1) G(1)");
	AssertTextSearch("5", "xyz", false, false, "xy xYxYz");
	AssertTextSearch("0", "whole", false, false, "whole");
	AssertTextSearch("", "wholes", false, false, "whole");
	AssertTextSearch("", "", false, false, "whole");
	AssertTextSearch("", "a", false, false, "");
	AssertTextSearch("3", "B", false, false, "AAAb");
	AssertTextSearch("0,2,3", "b", false, false, "BAbB");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextSearchWholeWords(void)
{
	AssertTextSearch("0,20", "size", false, true, "size sizes my_size (Size)asize size2");
	AssertTextSearch("0,5,14,20,26,31", "size", false, false, "size sizes my_size (Size)asize size2");
	AssertTextSearch("6", "i", true, true, "if i_ i ii");
	AssertTextSearch("0", "a.b", false, true, "a.b xa.b a.bx");
	AssertTextSearch("1,6", "->", false, true, "a->b a->b");
	AssertTextSearch("0,3", "x", false, true, "x+=x");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextSearchCount(void)
{
	CTextSearch	cSearch;
	char		acText[] = { 'a', 'b', '\0', 'A', 'B', 'a', 'b' };

	cSearch.Init("aa", false, false);
	AssertSize(2, cSearch.Count("aaaaa", 5));
	AssertSize(0, cSearch.Count("a", 1));
	AssertFalse(cSearch.IsEmpty());
	AssertSize(2, cSearch.Length());
	cSearch.Kill();

	//The text needn't end with a zero, and can have them.
	cSearch.Init("ab", false, false);
	AssertSize(3, cSearch.Count(acText, sizeof(acText)));
	AssertSize(2, cSearch.Count(acText, sizeof(acText) - 1));
	AssertSize(3, cSearch.Find(acText, sizeof(acText), 1));
	AssertSize(5, cSearch.Find(acText, sizeof(acText), 4));
	AssertSize(TEXT_SEARCH_NOT_FOUND, cSearch.Find(acText, sizeof(acText), 6));
	AssertSize(TEXT_SEARCH_NOT_FOUND, cSearch.Find(acText, sizeof(acText), 100));
	cSearch.Kill();

	cSearch.Init("", false, false);
	AssertTrue(cSearch.IsEmpty());
	AssertSize(0, cSearch.Count("abc", 3));
	cSearch.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every place the regular expression szPattern is found in szText, as
// "position+length,...".
//
//////////////////////////////////////////////////////////////////////////
static void AssertRegexSearch(const char* szExpected, const char* szPattern, bool bMatchCase, bool bWholeWords, const char* szText)
{
	CTextSearch	cSearch;
	CChars		szActual;
	size		uiLength;
	size		uiPosition;

	cSearch.Init(szPattern, bMatchCase, bWholeWords, true);
	szActual.Init();
	uiLength = strlen(szText);
	uiPosition = cSearch.Find(szText, uiLength);
	while (uiPosition != TEXT_SEARCH_NOT_FOUND)
	{
		if (!szActual.Empty())
		{
			szActual.Append(",");
		}
		szActual.Append((uint32)uiPosition);
		szActual.Append("+");
		szActual.Append((uint32)cSearch.GetFoundLength());
		uiPosition = cSearch.Find(szText, uiLength, uiPosition + cSearch.GetFoundLength());
	}
	AssertString(szExpected, szActual.Text());
	szActual.Kill();
	cSearch.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextSearchRegex(void)
{
	CTextSearch	cSearch;
	CChars		sz;
	const char*	szText;

	AssertRegexSearch("1+2,6+3", "\\d+", true, false, "a12 b 345c");
	AssertRegexSearch("0+3,4+3", "ab+", false, false, "aBb ABB");
	AssertRegexSearch("", "ab+", true, false, "aBb ABB");
	AssertRegexSearch("0+3", "\\w+", true, false, "abc");

	//A match of nothing isn't found: what is found is always something.
	AssertRegexSearch("1+1,3+2", "\\d*", true, false, "a1b22");
	AssertRegexSearch("", "x*", true, false, "abc");
	AssertRegexSearch("", "^", true, false, "abc");

	//Whole words: what it matched isn't part of a longer word.
	AssertRegexSearch("0+4,20+4", "[sS]ize", true, true, "size sizes my_size (Size)asize size2" + 0);
	AssertRegexSearch("0+4,5+4,14+4,20+4,26+4,31+4", "[sS]ize", true, false, "size sizes my_size (Size)asize size2");
	AssertRegexSearch("4+3", "\\d+", true, true, "a12 345 6b");
	AssertRegexSearch("1+2,6+2", "->", true, true, "a->b a->b");

	//One that isn't a regular expression finds nothing, and says why.
	cSearch.Init("a(", true, false, true);
	AssertTrue(cSearch.IsRegex());
	AssertFalse(cSearch.IsValid());
	AssertString("A ( has no ) after it.", cSearch.GetError());
	AssertSize(2, cSearch.GetErrorPosition());
	AssertSize(TEXT_SEARCH_NOT_FOUND, cSearch.Find("a(", 2));
	AssertSize(0, cSearch.Count("a(a(", 4));
	cSearch.Kill();

	//The same as text is found.
	cSearch.Init("a(", true, false, false);
	AssertFalse(cSearch.IsRegex());
	AssertTrue(cSearch.IsValid());
	AssertSize(0, cSearch.Find("a(", 2));
	AssertSize(2, cSearch.GetFoundLength());
	AssertSize(2, cSearch.Count("a(a(", 4));
	cSearch.Kill();

	//An empty one is valid and finds nothing.
	cSearch.Init("", true, false, true);
	AssertTrue(cSearch.IsValid());
	AssertTrue(cSearch.IsEmpty());
	AssertSize(TEXT_SEARCH_NOT_FOUND, cSearch.Find("abc", 3));
	cSearch.Kill();

	//What a find is replaced with: a regular expression's groups, or the replacement as it is.
	sz.Init();
	szText = "x = Get(a); y = Get(bc);";
	cSearch.Init("Get\\((\\w+)\\)", true, false, true);
	AssertSize(4, cSearch.Find(szText, strlen(szText)));
	AssertSize(6, cSearch.GetFoundLength());
	AssertTrue(cSearch.CheckReplacement("$1.Get()"));
	AssertFalse(cSearch.CheckReplacement("$2"));
	cSearch.AppendReplacement(szText, "$1.Get()", &sz);
	AssertString("a.Get()", sz.Text());
	AssertSize(16, cSearch.Find(szText, strlen(szText), 10));
	sz.Clear();
	cSearch.AppendReplacement(szText, "$1.Get()", &sz);
	AssertString("bc.Get()", sz.Text());
	AssertSize(2, cSearch.Count(szText, strlen(szText)));
	cSearch.Kill();

	cSearch.Init("Get", true, false, false);
	AssertSize(4, cSearch.Find(szText, strlen(szText)));
	AssertTrue(cSearch.CheckReplacement("$2"));
	sz.Clear();
	cSearch.AppendReplacement(szText, "$1\\n", &sz);
	AssertString("$1\\n", sz.Text());
	cSearch.Kill();
	sz.Kill();

	//One that backtracks without end is given up on.
	cSearch.Init("(a+)+b", true, false, true);
	AssertSize(TEXT_SEARCH_NOT_FOUND, cSearch.Find("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac", 51));
	AssertTrue(cSearch.GaveUp());
	cSearch.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextSearch(void)
{
	BeginTests();

	TestTextSearchCase();
	TestTextSearchWholeWords();
	TestTextSearchCount();
	TestTextSearchRegex();

	TestStatistics();
}

