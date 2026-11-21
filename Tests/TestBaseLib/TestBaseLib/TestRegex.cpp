#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Regex.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// What szPattern first finds in szText, from uiFrom: the text matched,
// then each group's after a |, with ~ for a group that took no part:
// "ab|a|~".  "<none>" if it finds nothing and "<error>" if it isn't a
// pattern.
//
//////////////////////////////////////////////////////////////////////////
static char* RegexFind(const char* szPattern, const char* szText, bool bCaseSensitive, size uiFrom, CChars* pszDest)
{
	CRegex		cRegex;
	SRegexMatch	sMatch;
	uint16		i;

	pszDest->Clear();
	cRegex.Init();
	if (!cRegex.Compile(szPattern, bCaseSensitive))
	{
		pszDest->Append("<error>");
	}
	else if (!cRegex.Find(szText, strlen(szText), uiFrom, &sMatch))
	{
		pszDest->Append("<none>");
	}
	else
	{
		pszDest->Append(&szText[sMatch.uiStart], sMatch.uiEnd - sMatch.uiStart);
		for (i = 1; i <= cRegex.NumGroups(); i++)
		{
			pszDest->Append('|');
			if (sMatch.auiGroupStarts[i] == REGEX_NO_POSITION)
			{
				pszDest->Append('~');
			}
			else
			{
				pszDest->Append(&szText[sMatch.auiGroupStarts[i]], sMatch.auiGroupEnds[i] - sMatch.auiGroupStarts[i]);
			}
		}
	}
	cRegex.Kill();
	return pszDest->Text();
}


#define AssertRegex(e, p, t)			AssertString(e, RegexFind(p, t, true, 0, &sz))
#define AssertRegexNoCase(e, p, t)		AssertString(e, RegexFind(p, t, false, 0, &sz))


//////////////////////////////////////////////////////////////////////////
//
// Where szPattern first matches in szText: its start, or -1.
//
//////////////////////////////////////////////////////////////////////////
static int RegexStart(const char* szPattern, const char* szText)
{
	CRegex		cRegex;
	SRegexMatch	sMatch;
	int			iStart;

	cRegex.Init();
	iStart = -1;
	if (cRegex.Compile(szPattern) && cRegex.Find(szText, strlen(szText), 0, &sMatch))
	{
		iStart = (int)sMatch.uiStart;
	}
	cRegex.Kill();
	return iStart;
}


//////////////////////////////////////////////////////////////////////////
//
// Where szPattern is wrong: its error's position, or -1 if it compiles.
//
//////////////////////////////////////////////////////////////////////////
static int RegexErrorAt(const char* szPattern)
{
	CRegex	cRegex;
	int		iPosition;

	cRegex.Init();
	iPosition = -1;
	if (!cRegex.Compile(szPattern))
	{
		AssertFalse(cRegex.IsCompiled());
		AssertTrue(cRegex.GetError()[0] != '\0');
		iPosition = (int)cRegex.GetErrorPosition();
	}
	cRegex.Kill();
	return iPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegexCharacters(void)
{
	CChars	sz;

	sz.Init();

	AssertRegex("abc", "abc", "xxabcxx");
	AssertRegex("<none>", "abd", "xxabcxx");
	AssertRegex("", "", "abc");
	AssertInt(2, RegexStart("abc", "xxabcxx"));
	AssertInt(-1, RegexStart("abc", "ab"));
	AssertInt(0, RegexStart("", ""));

	//A dot is any character of a line.
	AssertRegex("abc", "a.c", "abc");
	AssertRegex("a c", "a.c", "abd a c");
	AssertRegex("<none>", "a.c", "a\nc");
	AssertRegex("<none>", "a.c", "a\rc");

	//Classes: characters, ranges, and what isn't in them.
	AssertRegex("bcaa", "[abc]+", "xxbcaay");
	AssertRegex("xyz", "[^abc]+", "abcxyzabc");
	AssertRegex("1b2c", "[a-c0-2]+", "zz1b2cz");
	AssertRegex("a-z", "[a\\-z]+", "ba-zy");
	AssertRegex("-a-", "[-a]+", "b-a-b");
	AssertRegex("a-a", "[a-]+", "ba-ab");
	AssertRegex("]]", "[\\]]+", "a]]b");
	AssertRegex("[x]", "[\\[\\]x]+", "a[x]b");
	AssertRegex("ABC", "[\\x41-\\x43]+", "zABCD");
	AssertRegex("12_3", "[\\d_]+", "ab12_3cd");
	AssertRegex("ab", "[^\\d\\s]+", "12 ab 34");
	AssertRegex("a\tb", "[a\\tb]+", "xa\tbx");
	AssertRegex("\xc3\xa9", "[\\x80-\\xff]+", "a\xc3\xa9" "b");
	AssertRegex("^", "[\\^]", "a^b");
	AssertRegex("^a", "[a^]+", "b^ab");
	AssertRegex("<none>", "[]", "abc");
	AssertRegex("a", "[^]", "abc");

	//The sets.
	AssertRegex("123", "\\d+", "abc123def");
	AssertRegex("abc", "\\D+", "abc123def");
	AssertRegex("foo_1", "\\w+", "  foo_1 ");
	AssertRegex(" - ", "\\W+", "a - b");
	AssertRegex(" \t\n", "\\s+", "a \t\nb");
	AssertRegex("ab", "\\S+", "  ab  ");

	//Escapes.
	AssertRegex(".", "\\.", "a.b");
	AssertRegex("a\tb", "a\\tb", "xa\tbx");
	AssertRegex("a\nb", "a\\nb", "xa\nbx");
	AssertRegex("\r\n", "\\r\\n", "a\r\nb");
	AssertRegex("A", "\\x41", "zAz");
	AssertRegex("()", "\\(\\)", "f()");
	AssertRegex("\\", "\\\\", "a\\b");
	AssertRegex("a*b+c?", "a\\*b\\+c\\?", "xa*b+c?x");
	AssertRegex("$^|", "\\$\\^\\|", "a$^|b");
	AssertRegex("a_b", "a\\_b", "a_b");

	//A { that doesn't start a count is a {, so code can be looked for as it is written.
	AssertRegex("foo() {", "foo\\(\\) {", "void foo() {");
	AssertRegex("{}", "{}", "a = {};");
	AssertRegex("a{,3}", "a{,3}", "xa{,3}x");
	AssertRegex("a{2", "a{2", "xa{2x");
	AssertRegex("a{x}", "a{x}", "a{x}");
	AssertRegex("}", "}", "a}");
	AssertRegex("]", "]", "a]");

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegexPlaces(void)
{
	CChars	sz;

	sz.Init();

	//^ and $ are a line's start and end.
	AssertInt(3, RegexStart("^b", "ab\nbc"));
	AssertInt(0, RegexStart("^a", "ab\nbc"));
	AssertInt(-1, RegexStart("^c", "ab\nbc"));
	AssertInt(1, RegexStart("c$", "ac\nbc"));
	AssertInt(4, RegexStart("c$", "acb\nc"));
	AssertInt(1, RegexStart("c$", "ac\r\nb"));
	AssertInt(-1, RegexStart("c$", "ac\rb"));
	AssertInt(2, RegexStart("^$", "a\n\nb"));
	AssertInt(0, RegexStart("^$", ""));
	AssertRegex("", "^.*$", "\nbc\nd");
	AssertString("bc", RegexFind("^.*$", "\nbc\nd", true, 1, &sz));
	AssertString("d", RegexFind("^.*$", "\nbc\nd", true, 2, &sz));
	AssertRegex("one", "^\\w+$", "a b\none\ntwo");

	//\b is where a word starts or ends, \B where one doesn't.
	AssertInt(5, RegexStart("\\bfoo\\b", "afoo foo foob"));
	AssertInt(0, RegexStart("\\bfoo", "foo"));
	AssertInt(-1, RegexStart("\\bfoo\\b", "afoo foob"));
	AssertInt(1, RegexStart("\\Boo", "foo oo"));
	AssertInt(4, RegexStart("\\boo", "foo oo"));
	AssertInt(3, RegexStart("\\b", "   a"));
	AssertInt(0, RegexStart("\\B", "   a"));

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegexGroups(void)
{
	CRegex	cRegex;
	CChars	sz;

	sz.Init();

	AssertRegex("a|a|~", "(a)(b)?", "ac");
	AssertRegex("ab|a|b", "(a)(b)?", "ab");
	AssertRegex("abba|a", "(a|b)+", "xabbax");
	AssertRegex("abab", "(?:ab)+", "ababx");
	AssertRegex("ab|ab|a|b", "((a)(b))", "xab");
	AssertRegex("a1b2|b|2", "(?:(\\w)(\\d))+", "a1b2");

	//One or another: the first that matches, not the longest.
	AssertRegex("dog", "cat|dog", "hotdog");
	AssertRegex("cat", "cat|dog|bird", "a cat and a dog");
	AssertRegex("a", "a|ab", "ab");
	AssertRegex("ab", "ab|a", "ab");
	AssertRegex("abc|ab", "(a|ab)c", "abc");
	AssertRegex("", "a|", "b");
	AssertRegex("", "|a", "a");
	AssertRegex("ac", "a(?:b|)c", "ac");
	AssertRegex("xbz|b", "x(a|b|c)z", "xbz");
	AssertRegex("gray|a", "gr(a|e)y", "grey gray" + 5);

	//What a group matched, again.
	AssertRegex("ll|l", "(\\w)\\1", "hello");
	AssertRegex("abcabc|abc", "(abc)\\1", "xabcabcx");
	AssertRegex("<none>", "(abc)\\1", "abcab");
	AssertRegex("abab|a|b", "(a)(b)\\1\\2", "abab");
	AssertRegex("<none>", "(a)?\\1b", "b");
	AssertRegex("aab|a", "(a)?\\1b", "aab");
	AssertRegex("\"it's\"|\"", "([\"']).*?\\1", "say \"it's\" now");

	cRegex.Init();
	AssertTrue(cRegex.Compile("(a)(?:b)(c)"));
	AssertInt(2, cRegex.NumGroups());
	AssertTrue(cRegex.Compile("abc"));
	AssertInt(0, cRegex.NumGroups());
	cRegex.Kill();

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegexRepeats(void)
{
	CChars	sz;

	sz.Init();

	//As many as it can.
	AssertRegex("aaa", "a*", "aaa");
	AssertRegex("", "a*", "baaa");
	AssertRegex("aaa", "a+", "baaa");
	AssertRegex("<none>", "a+", "bbb");
	AssertRegex("a", "a?", "aa");
	AssertRegex("", "a?", "b");
	AssertRegex("abbbc", "ab*c", "xabbbcx");
	AssertRegex("ac", "ab*c", "xacx");
	AssertRegex("<none>", "ab+c", "xacx");
	AssertRegex("abc", "ab?c", "abc");
	AssertRegex("<a><b>", "<.+>", "x<a><b>x");
	AssertRegex("aaab", "a*ab", "aaab");
	AssertRegex("foo.bar", "\\w+\\.\\w+", "x foo.bar y");

	//Counts.
	AssertRegex("aa", "a{2}", "aaaa");
	AssertRegex("aaaa", "a{2,}", "aaaa");
	AssertRegex("aaa", "a{2,3}", "aaaa");
	AssertRegex("<none>", "a{3}", "aa");
	AssertRegex("", "a{0}", "aa");
	AssertRegex("a", "a{0,1}", "aa");
	AssertRegex("ababab", "(?:ab){3}", "abababab");
	AssertRegex("abab|ab", "(ab){1,2}", "ababab");
	AssertRegex("2026-11-20", "\\d{4}-\\d{2}-\\d{2}", "on 2026-11-20.");
	AssertRegex("aaaaaaaaaa", "a{10}", "aaaaaaaaaaaa");
	AssertRegex("abcabc", "(?:a|b|c){6}", "abcabcabc");
	AssertRegex("xaax", "x(?:a{2}){1,2}x", "xaax");
	AssertRegex("xaaaax", "x(?:a{2}){1,2}x", "xaaaax");
	AssertRegex("<none>", "x(?:a{2}){1,2}x", "xaaax");

	//As few as it can.
	AssertRegex("a", "a+?", "aaa");
	AssertRegex("", "a*?", "aaa");
	AssertRegex("", "a??", "aaa");
	AssertRegex("<a>", "<.+?>", "x<a><b>x");
	AssertRegex("aaab", "a*?b", "aaab");
	AssertRegex("aa", "a{2,3}?", "aaaa");
	AssertRegex("aaa", "a{2,3}?$", "aaa");
	AssertRegex("aa", "a{2,}?", "aaaa");
	AssertRegex("ab", "a??b", "ab");
	AssertRegex("/* a */", "/\\*.*?\\*/", "x /* a */ y /* b */");

	//A loop whose turn matches nothing ends.
	AssertRegex("|", "(a*)*", "b");
	AssertRegex("aa|", "(a*)*", "aa");
	AssertRegex("aa|", "(a*)+", "aa");
	AssertRegex("|", "(a*)+", "b");
	AssertRegex("abc|", "(a|b*)*c", "abc");
	AssertRegex("a|", "(^)*a", "a");
	AssertRegex("a|", "(\\b)+a", "a");
	AssertRegex("ab|a", "(a?)*?b", "ab");
	AssertRegex("", "(?:)*", "a");
	AssertRegex("", "(?:a?){3}", "b");
	AssertRegex("aa", "(?:a?){3}", "aab");
	AssertRegex("xx|", "(x*)+$", "xx");

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegexCase(void)
{
	CChars	sz;

	sz.Init();

	AssertRegexNoCase("ABC", "abc", "xABCx");
	AssertRegexNoCase("abc", "ABC", "xabcx");
	AssertRegexNoCase("aBc", "AbC", "xaBcx");
	AssertRegex("<none>", "abc", "xABCx");
	AssertRegexNoCase("AbC", "[a-c]+", "xAbCx");
	AssertRegexNoCase("abc", "[A-C]+", "xabcx");
	AssertRegexNoCase("b", "[^a]+", "Aab");
	AssertRegexNoCase("aA|a", "(a)\\1", "xaAx");
	AssertRegexNoCase("a1", "A\\d", "a1");
	AssertRegexNoCase("WORD", "\\bword\\b", "a WORD b");
	AssertRegexNoCase("1.2", "1\\.2", "1.2");

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegexErrors(void)
{
	CRegex	cRegex;
	CChars	sz;

	sz.Init();

	AssertInt(-1, RegexErrorAt("abc"));
	AssertInt(1, RegexErrorAt("("));
	AssertInt(3, RegexErrorAt("(ab"));
	AssertInt(0, RegexErrorAt(")"));
	AssertInt(1, RegexErrorAt("a)"));
	AssertInt(3, RegexErrorAt("(a))"));
	AssertInt(2, RegexErrorAt("[a"));
	AssertInt(1, RegexErrorAt("["));
	AssertInt(0, RegexErrorAt("*a"));
	AssertInt(0, RegexErrorAt("+"));
	AssertInt(1, RegexErrorAt("(?a)"));
	AssertInt(2, RegexErrorAt("a**"));
	AssertInt(2, RegexErrorAt("a+*"));
	AssertInt(3, RegexErrorAt("a??+"));
	AssertInt(6, RegexErrorAt("a{3,2}"));
	AssertInt(1, RegexErrorAt("\\"));
	AssertInt(1, RegexErrorAt("\\q"));
	AssertInt(1, RegexErrorAt("\\1"));
	AssertInt(4, RegexErrorAt("(a)\\2"));
	AssertInt(2, RegexErrorAt("[\\q]"));
	AssertInt(4, RegexErrorAt("[b-a]"));
	AssertInt(5, RegexErrorAt("[a-\\d]"));
	AssertInt(3, RegexErrorAt("\\x4"));
	AssertInt(2, RegexErrorAt("\\xg1"));
	AssertInt(2, RegexErrorAt("a\rb") + 1);
	AssertTrue(RegexErrorAt("a{1001}") >= 0);
	AssertTrue(RegexErrorAt("(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)") >= 0);
	AssertInt(-1, RegexErrorAt("(a)(b)(c)(d)(e)(f)(g)(h)(i)(?:j)"));
	AssertInt(-1, RegexErrorAt("a{1000}"));
	AssertTrue(RegexErrorAt("(?:a{1000}){1000}") >= 0);
	AssertTrue(RegexErrorAt("(?:(?:(?:a{30}){30}){30}){30}") >= 0);

	//A pattern that didn't compile matches nothing, and the next one compiled takes its place.
	cRegex.Init();
	AssertFalse(cRegex.IsCompiled());
	AssertFalse(cRegex.Find("abc", 3, 0, NULL));
	AssertFalse(cRegex.Compile("a("));
	AssertString("A ( has no ) after it.", cRegex.GetError());
	AssertFalse(cRegex.Find("a(", 2, 0, NULL));
	AssertTrue(cRegex.Compile("a"));
	AssertString("", cRegex.GetError());
	AssertTrue(cRegex.Find("a(", 2, 0, NULL));
	cRegex.Kill();

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegexFinding(void)
{
	CRegex		cRegex;
	SRegexMatch	sMatch;
	CChars		szLong;
	size		uiLong;
	CChars		sz;
	size		uiFrom;
	int			iCount;

	sz.Init();
	cRegex.Init();

	//From a place, and at a place.
	AssertTrue(cRegex.Compile("an"));
	AssertTrue(cRegex.Find("banana", 6, 0, &sMatch));
	AssertSize(1, sMatch.uiStart);
	AssertSize(3, sMatch.uiEnd);
	AssertTrue(cRegex.Find("banana", 6, 2, &sMatch));
	AssertSize(3, sMatch.uiStart);
	AssertFalse(cRegex.Find("banana", 6, 4, &sMatch));
	AssertFalse(cRegex.Find("banana", 6, 7, &sMatch));
	AssertTrue(cRegex.MatchAt("banana", 6, 3, &sMatch));
	AssertSize(5, sMatch.uiEnd);
	AssertFalse(cRegex.MatchAt("banana", 6, 2, &sMatch));
	AssertTrue(cRegex.MatchAt("banana", 6, 1, NULL));

	//Every match: one that is of nothing is stepped past.
	AssertTrue(cRegex.Compile("\\d*"));
	iCount = 0;
	for (uiFrom = 0; cRegex.Find("a12b345", 7, uiFrom, &sMatch); uiFrom = (sMatch.uiEnd > sMatch.uiStart) ? sMatch.uiEnd : sMatch.uiEnd + 1)
	{
		iCount++;
	}
	AssertInt(5, iCount);

	//The text is as long as it is said to be: it needn't end with a 0.
	AssertTrue(cRegex.Compile("b$"));
	AssertTrue(cRegex.Find("abc", 2, 0, &sMatch));
	AssertSize(1, sMatch.uiStart);
	AssertFalse(cRegex.Find("abc", 3, 0, &sMatch));
	AssertTrue(cRegex.Compile("\\w+"));
	AssertTrue(cRegex.Find("abcdef", 3, 0, &sMatch));
	AssertSize(3, sMatch.uiEnd);

	//A long text doesn't overflow anything: the matcher's stack is its own, on the heap.  (The suite's debug limit on
	//one allocation keeps the text to ten thousand characters: a turn of a loop takes up to four frames.  With a 16
	//bit size, the W65C816's, the stack is kept to what a size can count, a few thousand frames, so the text is
	//five hundred.)
	uiLong = (sizeof(size) > 2) ? 10000 : 500;
	szLong.Init();
	szLong.Append('a', uiLong);
	szLong.Append("b\n");
	AssertTrue(cRegex.Compile(".*"));
	AssertTrue(cRegex.Find(szLong.Text(), szLong.Length(), 0, &sMatch));
	AssertSize(uiLong + 1, sMatch.uiEnd);
	AssertTrue(cRegex.Compile("(a|b)*$"));
	AssertTrue(cRegex.Find(szLong.Text(), szLong.Length(), 0, &sMatch));
	AssertSize(0, sMatch.uiStart);
	AssertSize(uiLong + 1, sMatch.uiEnd);
	AssertTrue(cRegex.Compile("a*?b"));
	AssertTrue(cRegex.Find(szLong.Text(), szLong.Length(), 0, &sMatch));
	AssertSize(uiLong + 1, sMatch.uiEnd);
	AssertFalse(cRegex.GaveUp());
	AssertTrue(cRegex.Compile("b\\n"));
	AssertTrue(cRegex.Find(szLong.Text(), szLong.Length(), 0, &sMatch));
	AssertSize(uiLong, sMatch.uiStart);
	szLong.Kill();

	//With a 16 bit size, a run with more ways to go back to than the stack is let hold is given up on, not run past
	//what a size can count.
	if (sizeof(size) == 2)
	{
		szLong.Init();
		szLong.Append('a', 10000);
		szLong.Append("b\n");
		AssertTrue(cRegex.Compile(".*"));
		AssertFalse(cRegex.MatchAt(szLong.Text(), szLong.Length(), 0, &sMatch));
		AssertTrue(cRegex.GaveUp());
		AssertTrue(cRegex.Compile("b\\n"));
		AssertTrue(cRegex.Find(szLong.Text(), szLong.Length(), 0, &sMatch));
		AssertFalse(cRegex.GaveUp());
		AssertSize(10000, sMatch.uiStart);
		szLong.Kill();
	}

	//A pattern that backtracks without end is given up on.
	AssertTrue(cRegex.Compile("(a+)+b"));
	AssertFalse(cRegex.Find("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac", 51, 0, &sMatch));
	AssertTrue(cRegex.GaveUp());
	AssertTrue(cRegex.Find("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab", 51, 0, &sMatch));
	AssertFalse(cRegex.GaveUp());
	AssertSize(51, sMatch.uiEnd);

	cRegex.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegexReplacement(void)
{
	CRegex		cRegex;
	SRegexMatch	sMatch;
	CChars		sz;
	const char*	szText;

	sz.Init();
	cRegex.Init();

	szText = "x user@host y";
	AssertTrue(cRegex.Compile("(\\w+)@(\\w+)"));
	AssertTrue(cRegex.Find(szText, strlen(szText), 0, &sMatch));
	AssertTrue(cRegex.CheckReplacement("$2 at $1"));
	cRegex.AppendReplacement(szText, &sMatch, "$2 at $1", &sz);
	AssertString("host at user", sz.Text());

	sz.Clear();
	cRegex.AppendReplacement(szText, &sMatch, "[$0] \\$1 \\\\ a\\nb\\tc $", &sz);
	AssertString("[user@host] $1 \\ a\nb\tc $", sz.Text());

	//A group the pattern doesn't have is wrong, and is nothing.
	AssertFalse(cRegex.CheckReplacement("$1 $3"));
	AssertString("The replacement names group $3, which the pattern doesn't have.", cRegex.GetError());
	AssertSize(3, cRegex.GetErrorPosition());
	AssertTrue(cRegex.CheckReplacement("\\$3 $$ $x"));
	sz.Clear();
	cRegex.AppendReplacement(szText, &sMatch, "<$3>", &sz);
	AssertString("<>", sz.Text());

	//A group that took no part is nothing.
	AssertTrue(cRegex.Compile("(a)|(b)"));
	AssertTrue(cRegex.Find("b", 1, 0, &sMatch));
	sz.Clear();
	cRegex.AppendReplacement("b", &sMatch, "<$1><$2>", &sz);
	AssertString("<><b>", sz.Text());

	cRegex.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRegex(void)
{
	BeginTests();

	TestRegexCharacters();
	TestRegexPlaces();
	TestRegexGroups();
	TestRegexRepeats();
	TestRegexCase();
	TestRegexErrors();
	TestRegexFinding();
	TestRegexReplacement();

	TestStatistics();
}

