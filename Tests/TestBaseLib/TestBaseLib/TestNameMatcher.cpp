#include "BaseLib/GlobalMemory.h"
#include "BaseLib/NameMatcher.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The match's ranges as "start+length,...".
//
//////////////////////////////////////////////////////////////////////////
static char* PrintRanges(SNameMatch* psMatch, CChars* pszDest)
{
	size	i;

	pszDest->Clear();
	for (i = 0; i < psMatch->uiNumRanges; i++)
	{
		if (i > 0)
		{
			pszDest->Append(",");
		}
		pszDest->Append((int)psMatch->asRanges[i].uiStart);
		pszDest->Append("+");
		pszDest->Append((int)psMatch->asRanges[i].uiLength);
	}
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// True if szPattern matches szName, with the ranges as expected.
//
//////////////////////////////////////////////////////////////////////////
static void AssertNameMatch(const char* szPattern, const char* szName, const char* szRanges)
{
	CNameMatcher	cMatcher;
	SNameMatch		sMatch;
	CChars			szActual;

	cMatcher.Init(szPattern);
	szActual.Init();
	AssertTrue(cMatcher.Match(szName, &sMatch));
	AssertString(szRanges, PrintRanges(&sMatch, &szActual));
	szActual.Kill();
	cMatcher.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertNoNameMatch(const char* szPattern, const char* szName)
{
	CNameMatcher	cMatcher;
	SNameMatch		sMatch;

	cMatcher.Init(szPattern);
	AssertFalse(cMatcher.Match(szName, &sMatch));
	cMatcher.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 NameScore(const char* szPattern, const char* szName)
{
	CNameMatcher	cMatcher;
	SNameMatch		sMatch;
	int32			iScore;

	cMatcher.Init(szPattern);
	iScore = cMatcher.Match(szName, &sMatch) ? sMatch.iScore : -1000000;
	cMatcher.Kill();
	return iScore;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNameMatcherWordStarts(void)
{
	AssertTrue(IsNameWordStart("CTextArea", 0));
	AssertTrue(IsNameWordStart("CTextArea", 1));
	AssertFalse(IsNameWordStart("CTextArea", 2));
	AssertTrue(IsNameWordStart("CTextArea", 5));
	AssertTrue(IsNameWordStart("HTMLParser", 0));
	AssertFalse(IsNameWordStart("HTMLParser", 1));
	AssertTrue(IsNameWordStart("HTMLParser", 4));
	AssertTrue(IsNameWordStart("text_area", 5));
	AssertFalse(IsNameWordStart("text_area", 4));
	AssertTrue(IsNameWordStart("W65C816", 1));
	AssertFalse(IsNameWordStart("W65C816", 2));
	AssertTrue(IsNameWordStart("W65C816", 3));
	AssertTrue(IsNameWordStart("Text.cpp", 5));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNameMatcherMatches(void)
{
	//Initials, word starts and runs.
	AssertNameMatch("CTA", "CTextArea", "0+2,5+1");
	AssertNameMatch("cta", "CTextArea", "0+2,5+1");
	AssertNameMatch("TeArea", "CTextArea", "1+2,5+4");
	AssertNameMatch("textarea", "CTextArea", "1+8");
	AssertNameMatch("tarea", "CTextArea", "1+1,5+4");
	AssertNameMatch("Area", "CTextArea", "5+4");
	AssertNameMatch("hp", "HTMLParser", "0+1,4+1");
	AssertNameMatch("ta", "text_area", "0+1,5+1");
	AssertNameMatch("", "Anything", "");

	//The first character must start a word; later ones follow the one before or start a word.
	AssertNoNameMatch("xt", "CTextArea");
	AssertNoNameMatch("txtar", "CTextArea");
	AssertNoNameMatch("CTAX", "CTextArea");

	//A wildcard lets the next character be anywhere.
	AssertNameMatch("*xt", "CTextArea", "3+2");
	AssertNameMatch("te*ea", "CTextArea", "1+2,7+2");
	AssertNameMatch("te ea", "CTextArea", "1+2,7+2");

	//Backtracking finds a later start when the first doesn't lead to a match.
	AssertNameMatch("tfi", "TextFieldTextField", "0+1,4+2");
	AssertNameMatch("tfie", "TextFileTextField", "0+1,12+3");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNameMatcherScores(void)
{
	//Exact, then the same case, then a prefix, then fewer runs, then shorter.
	AssertTrue(NameScore("Text", "Text") > NameScore("text", "Text"));
	AssertTrue(NameScore("text", "Text") > NameScore("text", "TextArea"));
	AssertTrue(NameScore("text", "TextArea") > NameScore("text", "CTextArea"));
	AssertTrue(NameScore("textarea", "CTextArea") > NameScore("ta", "CTextArea"));
	AssertTrue(NameScore("ta", "TextArea") > NameScore("ta", "TextAreaEdit"));
	AssertTrue(NameScore("tf", "TextField") > NameScore("tf", "CTextField"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNameMatcherPaths(void)
{
	CNameMatcher	cMatcher;
	SNameMatch		sMatch;
	CChars			szRanges;

	szRanges.Init();
	cMatcher.Init("wl/tf");
	AssertTrue(cMatcher.MatchPath("/home/r2d2/Libraries/WindowLib/TextField.h", &sMatch));
	AssertString("0+1,4+1", PrintRanges(&sMatch, &szRanges));
	AssertFalse(cMatcher.MatchPath("/home/r2d2/Libraries/BaseLib/TextField.h", &sMatch));
	AssertFalse(cMatcher.MatchPath("/home/r2d2/Libraries/WindowLib/Label.h", &sMatch));
	cMatcher.Kill();

	//Folders in order, any number skipped between.
	cMatcher.Init("lib/win/text");
	AssertTrue(cMatcher.MatchPath("C:\\Code\\Libraries\\WindowLib\\TextArea.cpp", &sMatch));
	AssertFalse(cMatcher.MatchPath("C:\\Code\\WindowLib\\Libraries\\TextArea.cpp", &sMatch));
	cMatcher.Kill();

	//Without a folder only the file name is matched.
	cMatcher.Init("lib");
	AssertFalse(cMatcher.MatchPath("/Libraries/WindowLib/TextArea.cpp", &sMatch));
	AssertTrue(cMatcher.MatchPath("/Code/Libc65/Libc.h", &sMatch));
	cMatcher.Kill();
	szRanges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNameMatcher(void)
{
	BeginTests();

	TestNameMatcherWordStarts();
	TestNameMatcherMatches();
	TestNameMatcherScores();
	TestNameMatcherPaths();

	TestStatistics();
}
