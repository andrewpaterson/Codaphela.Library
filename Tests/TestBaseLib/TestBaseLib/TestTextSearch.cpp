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
//
//////////////////////////////////////////////////////////////////////////
void TestTextSearch(void)
{
	BeginTests();

	TestTextSearchCase();
	TestTextSearchWholeWords();
	TestTextSearchCount();

	TestStatistics();
}

