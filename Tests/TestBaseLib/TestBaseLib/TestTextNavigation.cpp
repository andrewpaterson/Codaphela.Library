#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TextNavigation.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Characters: a two byte code point, a ZWJ sequence (woman, ZWJ, laptop:
// eleven bytes) and a carriage return and line feed are each one; a byte
// that doesn't start valid UTF8 is one by itself.
//
//////////////////////////////////////////////////////////////////////////
void TestTextNavigationCharacters(void)
{
	CChars			szText;
	CTextNavigation	cNavigation;

	szText.Init("a\xC3\xA9" "b\r\n" "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB" "\xFF" "c");
	cNavigation.Init(&szText);

	AssertSize(1, cNavigation.CharacterLength(0));
	AssertSize(2, cNavigation.CharacterLength(1));
	AssertSize(11, cNavigation.CharacterLength(6));
	AssertSize(1, cNavigation.CharacterLength(17));
	AssertSize(0, cNavigation.CharacterLength(szText.Length()));

	AssertSize(1, cNavigation.NextPosition(0));
	AssertSize(3, cNavigation.NextPosition(1));
	AssertSize(6, cNavigation.NextPosition(4));
	AssertSize(17, cNavigation.NextPosition(6));
	AssertSize(szText.Length(), cNavigation.NextPosition(szText.Length()));

	AssertSize(1, cNavigation.PreviousPosition(3));
	AssertSize(4, cNavigation.PreviousPosition(6));
	AssertSize(6, cNavigation.PreviousPosition(17));
	AssertSize(0, cNavigation.PreviousPosition(0));

	//The text is read as it is at each call.
	szText.Set("xy");
	AssertSize(1, cNavigation.PreviousPosition(2));
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Lines end at line feeds; a line's end is before its carriage return.
// Its indent is its first character that isn't a space or tab.  Columns
// count characters.
//
//////////////////////////////////////////////////////////////////////////
void TestTextNavigationLines(void)
{
	CChars			szText;
	CTextNavigation	cNavigation;

	szText.Init("one\r\n  \t\xC3\xA9x\n\nlast");
	cNavigation.Init(&szText);

	AssertSize(0, cNavigation.FindLineStart(2));
	AssertSize(3, cNavigation.FindLineEnd(0));
	AssertSize(5, cNavigation.FindLineStart(9));
	AssertSize(11, cNavigation.FindLineEnd(5));
	AssertSize(12, cNavigation.FindLineStart(12));
	AssertSize(12, cNavigation.FindLineEnd(12));
	AssertSize(17, cNavigation.FindLineEnd(13));

	AssertSize(8, cNavigation.FindLineIndent(5));
	AssertSize(12, cNavigation.FindLineIndent(12));

	AssertSize(4, cNavigation.CountColumns(5, 10));
	AssertSize(10, cNavigation.FindColumn(5, 4));
	AssertSize(11, cNavigation.FindColumn(5, 99));
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Words are runs of letters, digits, underscores and characters that
// aren't ASCII, or of spaces; punctuation is a word of one character.
//
//////////////////////////////////////////////////////////////////////////
void TestTextNavigationWords(void)
{
	CChars			szText;
	CTextNavigation	cNavigation;

	szText.Init("int caf\xC3\xA9_2;  x");
	cNavigation.Init(&szText);

	AssertSize(3, cNavigation.FindWordRight(0));
	AssertSize(4, cNavigation.FindWordRight(3));
	AssertSize(11, cNavigation.FindWordRight(4));
	AssertSize(12, cNavigation.FindWordRight(11));
	AssertSize(14, cNavigation.FindWordRight(12));
	AssertSize(szText.Length(), cNavigation.FindWordRight(szText.Length()));

	AssertSize(14, cNavigation.FindWordLeft(15));
	AssertSize(12, cNavigation.FindWordLeft(14));
	AssertSize(11, cNavigation.FindWordLeft(12));
	AssertSize(4, cNavigation.FindWordLeft(11));
	AssertSize(0, cNavigation.FindWordLeft(3));
	AssertSize(0, cNavigation.FindWordLeft(0));

	AssertTrue(CTextNavigation::IsWordChar('_'));
	AssertTrue(CTextNavigation::IsWordChar((char)0xC3));
	AssertFalse(CTextNavigation::IsWordChar(';'));
	AssertTrue(CTextNavigation::IsWhitespace('\r'));
	AssertFalse(CTextNavigation::IsWhitespace('x'));
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The word around a character, as a double click selects it: a run of
// word characters, a run of spaces and tabs (not past the line), or one
// other character; nothing at a line's end.  The line around a position
// has its line feed, a carriage return and line feed being one.
//
//////////////////////////////////////////////////////////////////////////
void TestTextNavigationAround(void)
{
	CChars			szText;
	CTextNavigation	cNavigation;
	size			uiStart;
	size			uiEnd;

	//p 0, - 1, > 2, cafe 3..8 (e accented: 6, 7), spaces 8..11, x 11, CR LF 12, LF 14, spaces 15, ab 17, end 19.
	szText.Init("p->caf\xC3\xA9 \t x\r\n\n  ab");
	cNavigation.Init(&szText);

	AssertTrue(cNavigation.FindWordAround(0, &uiStart, &uiEnd));
	AssertSize(0, uiStart);
	AssertSize(1, uiEnd);
	AssertTrue(cNavigation.FindWordAround(1, &uiStart, &uiEnd));
	AssertSize(1, uiStart);
	AssertSize(2, uiEnd);
	AssertTrue(cNavigation.FindWordAround(2, &uiStart, &uiEnd));
	AssertSize(2, uiStart);
	AssertSize(3, uiEnd);
	AssertTrue(cNavigation.FindWordAround(3, &uiStart, &uiEnd));
	AssertSize(3, uiStart);
	AssertSize(8, uiEnd);
	AssertTrue(cNavigation.FindWordAround(6, &uiStart, &uiEnd));
	AssertSize(3, uiStart);
	AssertSize(8, uiEnd);
	AssertTrue(cNavigation.FindWordAround(9, &uiStart, &uiEnd));
	AssertSize(8, uiStart);
	AssertSize(11, uiEnd);
	AssertTrue(cNavigation.FindWordAround(11, &uiStart, &uiEnd));
	AssertSize(11, uiStart);
	AssertSize(12, uiEnd);

	//A line's end: its carriage return, line feed or an empty line.
	AssertFalse(cNavigation.FindWordAround(12, &uiStart, &uiEnd));
	AssertSize(12, uiStart);
	AssertSize(12, uiEnd);
	AssertFalse(cNavigation.FindWordAround(14, &uiStart, &uiEnd));
	AssertSize(14, uiStart);

	//Spaces at a line's start don't reach back over the line feed.
	AssertTrue(cNavigation.FindWordAround(15, &uiStart, &uiEnd));
	AssertSize(15, uiStart);
	AssertSize(17, uiEnd);
	AssertTrue(cNavigation.FindWordAround(18, &uiStart, &uiEnd));
	AssertSize(17, uiStart);
	AssertSize(19, uiEnd);
	AssertFalse(cNavigation.FindWordAround(19, &uiStart, &uiEnd));
	AssertSize(19, uiStart);
	AssertSize(19, uiEnd);

	cNavigation.FindLineAround(5, &uiStart, &uiEnd);
	AssertSize(0, uiStart);
	AssertSize(14, uiEnd);
	cNavigation.FindLineAround(12, &uiStart, &uiEnd);
	AssertSize(0, uiStart);
	AssertSize(14, uiEnd);
	cNavigation.FindLineAround(14, &uiStart, &uiEnd);
	AssertSize(14, uiStart);
	AssertSize(15, uiEnd);
	cNavigation.FindLineAround(16, &uiStart, &uiEnd);
	AssertSize(15, uiStart);
	AssertSize(19, uiEnd);
	cNavigation.FindLineAround(19, &uiStart, &uiEnd);
	AssertSize(15, uiStart);
	AssertSize(19, uiEnd);

	szText.Set("");
	AssertFalse(cNavigation.FindWordAround(0, &uiStart, &uiEnd));
	cNavigation.FindLineAround(0, &uiStart, &uiEnd);
	AssertSize(0, uiStart);
	AssertSize(0, uiEnd);

	AssertTrue(CTextNavigation::IsSpaceOrTab('\t'));
	AssertFalse(CTextNavigation::IsSpaceOrTab('\n'));
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A selection's other occurrences: text on one line that isn't only
// spaces and tabs is looked for, case and all, from the start, matches
// not overlapping, up to a most.
//
//////////////////////////////////////////////////////////////////////////
void TestTextNavigationOccurrences(void)
{
	CChars				szText;
	CTextNavigation		cNavigation;
	CArrayTextRange		asFound;

	//foo 4, 17, 23, 31 and in food 39; LF 8 and 27; spaces and a tab 28..31; FOO 35; end 43.
	szText.Init("int foo;\nint x = foo + foo;\n  \tfoo FOO food");
	cNavigation.Init(&szText);
	asFound.Init();

	AssertTrue(cNavigation.IsOccurrenceText(4, 7));
	AssertTrue(cNavigation.IsOccurrenceText(7, 8));
	AssertTrue(cNavigation.IsOccurrenceText(28, 32));
	AssertFalse(cNavigation.IsOccurrenceText(28, 31));
	AssertFalse(cNavigation.IsOccurrenceText(4, 10));
	AssertFalse(cNavigation.IsOccurrenceText(8, 9));
	AssertFalse(cNavigation.IsOccurrenceText(4, 4));
	AssertFalse(cNavigation.IsOccurrenceText(40, 44));

	cNavigation.FindOccurrences(4, 7, 1000, &asFound);
	AssertSize(4, asFound.NumElements());
	AssertSize(17, asFound.Get(0)->uiStart);
	AssertSize(23, asFound.Get(1)->uiStart);
	AssertSize(31, asFound.Get(2)->uiStart);
	AssertSize(39, asFound.Get(3)->uiStart);
	AssertSize(42, asFound.Get(3)->uiEnd);

	//From another of them, the first is found; at most two.
	asFound.ReInit();
	cNavigation.FindOccurrences(23, 26, 2, &asFound);
	AssertSize(2, asFound.NumElements());
	AssertSize(4, asFound.Get(0)->uiStart);
	AssertSize(17, asFound.Get(1)->uiStart);

	//Case matters: FOO has no others.
	asFound.ReInit();
	cNavigation.FindOccurrences(35, 38, 1000, &asFound);
	AssertSize(0, asFound.NumElements());

	//Matches don't overlap: "aa" in "aaaaa" is at 0 and 2.
	szText.Set("aaaaa");
	asFound.ReInit();
	cNavigation.FindOccurrences(3, 5, 1000, &asFound);
	AssertSize(2, asFound.NumElements());
	AssertSize(0, asFound.Get(0)->uiStart);
	AssertSize(2, asFound.Get(1)->uiStart);
	asFound.ReInit();
	cNavigation.FindOccurrences(0, 2, 1000, &asFound);
	AssertSize(1, asFound.NumElements());
	AssertSize(2, asFound.Get(0)->uiStart);

	asFound.Kill();
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The ranges Extend Selection grows through: the word over a selection,
// the quotes and brackets around it, and its comment.
//
//////////////////////////////////////////////////////////////////////////
void TestTextNavigationRanges(void)
{
	CChars			szText;
	CTextNavigation	cNavigation;
	size			uiStart;
	size			uiEnd;

	//f 4, ( 5, "b c" 13..18, ) 18, // note 20..27, LF 27, { 28, LF 29, tab 30, g 31, ( 32, x 33, [ 34, i 35, ] 36, ) 37,
	//; 38, LF 39, } 40, LF 41, /* one 42..48, LF 48, two */ 49..55, end 56..59.
	szText.Init("int f(int a, \"b c\") // note\n{\n\tg(x[i]);\n}\n/* one\ntwo */ end");
	cNavigation.Init(&szText);

	AssertTrue(cNavigation.FindWordOver(35, 35, &uiStart, &uiEnd));
	AssertSize(35, uiStart);
	AssertSize(36, uiEnd);
	AssertTrue(cNavigation.FindWordOver(57, 58, &uiStart, &uiEnd));
	AssertSize(56, uiStart);
	AssertSize(59, uiEnd);
	AssertTrue(cNavigation.FindWordOver(59, 59, &uiStart, &uiEnd));
	AssertSize(56, uiStart);
	AssertFalse(cNavigation.FindWordOver(34, 35, &uiStart, &uiEnd));
	AssertFalse(cNavigation.FindWordOver(37, 37, &uiStart, &uiEnd));

	AssertTrue(cNavigation.FindBracketsAround(35, 35, &uiStart, &uiEnd));
	AssertSize(34, uiStart);
	AssertSize(36, uiEnd);
	AssertTrue(cNavigation.FindBracketsAround(34, 37, &uiStart, &uiEnd));
	AssertSize(32, uiStart);
	AssertSize(37, uiEnd);
	AssertTrue(cNavigation.FindBracketsAround(13, 18, &uiStart, &uiEnd));
	AssertSize(5, uiStart);
	AssertSize(18, uiEnd);
	AssertTrue(cNavigation.FindBracketsAround(28, 40, &uiStart, &uiEnd));
	AssertSize(28, uiStart);
	AssertSize(40, uiEnd);
	//The brackets themselves selected are no bigger: the ones around them, if any.
	AssertFalse(cNavigation.FindBracketsAround(28, 41, &uiStart, &uiEnd));
	AssertFalse(cNavigation.FindBracketsAround(27, 41, &uiStart, &uiEnd));
	AssertFalse(cNavigation.FindBracketsAround(2, 2, &uiStart, &uiEnd));
	AssertFalse(cNavigation.FindBracketsAround(50, 50, &uiStart, &uiEnd));

	AssertTrue(cNavigation.FindCommentAround(24, 24, &uiStart, &uiEnd));
	AssertSize(20, uiStart);
	AssertSize(27, uiEnd);
	AssertTrue(cNavigation.FindCommentAround(46, 46, &uiStart, &uiEnd));
	AssertSize(42, uiStart);
	AssertSize(55, uiEnd);
	AssertTrue(cNavigation.FindCommentAround(42, 55, &uiStart, &uiEnd));
	AssertSize(42, uiStart);
	AssertSize(55, uiEnd);
	AssertFalse(cNavigation.FindCommentAround(46, 57, &uiStart, &uiEnd));
	AssertFalse(cNavigation.FindCommentAround(57, 57, &uiStart, &uiEnd));
	AssertFalse(cNavigation.FindCommentAround(10, 10, &uiStart, &uiEnd));

	AssertTrue(cNavigation.FindQuotesAround(15, 15, &uiStart, &uiEnd));
	AssertSize(13, uiStart);
	AssertSize(17, uiEnd);
	AssertFalse(cNavigation.FindQuotesAround(13, 14, &uiStart, &uiEnd));
	AssertFalse(cNavigation.FindQuotesAround(15, 30, &uiStart, &uiEnd));

	//" 4, a\"b, " 9, ' 13, c, ' 15, " 22, //y, " 26.  An escaped quote doesn't close; a // in quotes isn't a comment.
	szText.Set("x = \"a\\\"b\" + 'c'; s = \"//y\"; z");
	AssertTrue(cNavigation.FindQuotesAround(8, 8, &uiStart, &uiEnd));
	AssertSize(4, uiStart);
	AssertSize(9, uiEnd);
	AssertTrue(cNavigation.FindQuotesAround(14, 15, &uiStart, &uiEnd));
	AssertSize(13, uiStart);
	AssertSize(15, uiEnd);
	AssertTrue(cNavigation.FindQuotesAround(25, 25, &uiStart, &uiEnd));
	AssertSize(22, uiStart);
	AssertSize(26, uiEnd);
	AssertFalse(cNavigation.FindQuotesAround(10, 10, &uiStart, &uiEnd));
	AssertFalse(cNavigation.FindCommentAround(25, 25, &uiStart, &uiEnd));

	//A quote not closed on its line pairs with nothing.
	szText.Set("x = \"abc\ny\"");
	AssertFalse(cNavigation.FindQuotesAround(6, 6, &uiStart, &uiEnd));

	//Brackets closed by another kind aren't around anything.
	szText.Set("(a]");
	AssertFalse(cNavigation.FindBracketsAround(1, 1, &uiStart, &uiEnd));

	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextNavigation(void)
{
	BeginTests();

	TestTextNavigationCharacters();
	TestTextNavigationLines();
	TestTextNavigationWords();
	TestTextNavigationAround();
	TestTextNavigationOccurrences();
	TestTextNavigationRanges();

	TestStatistics();
}

