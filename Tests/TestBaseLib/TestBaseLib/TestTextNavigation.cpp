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
	CArrayTextOccurrence	auiStarts;

	//foo 4, 17, 23, 31 and in food 39; LF 8 and 27; spaces and a tab 28..31; FOO 35; end 43.
	szText.Init("int foo;\nint x = foo + foo;\n  \tfoo FOO food");
	cNavigation.Init(&szText);
	auiStarts.Init();

	AssertTrue(cNavigation.IsOccurrenceText(4, 7));
	AssertTrue(cNavigation.IsOccurrenceText(7, 8));
	AssertTrue(cNavigation.IsOccurrenceText(28, 32));
	AssertFalse(cNavigation.IsOccurrenceText(28, 31));
	AssertFalse(cNavigation.IsOccurrenceText(4, 10));
	AssertFalse(cNavigation.IsOccurrenceText(8, 9));
	AssertFalse(cNavigation.IsOccurrenceText(4, 4));
	AssertFalse(cNavigation.IsOccurrenceText(40, 44));

	cNavigation.FindOccurrences(4, 7, 1000, &auiStarts);
	AssertSize(4, auiStarts.NumElements());
	AssertSize(17, *auiStarts.Get(0));
	AssertSize(23, *auiStarts.Get(1));
	AssertSize(31, *auiStarts.Get(2));
	AssertSize(39, *auiStarts.Get(3));

	//From another of them, the first is found; at most two.
	auiStarts.ReInit();
	cNavigation.FindOccurrences(23, 26, 2, &auiStarts);
	AssertSize(2, auiStarts.NumElements());
	AssertSize(4, *auiStarts.Get(0));
	AssertSize(17, *auiStarts.Get(1));

	//Case matters: FOO has no others.
	auiStarts.ReInit();
	cNavigation.FindOccurrences(35, 38, 1000, &auiStarts);
	AssertSize(0, auiStarts.NumElements());

	//Matches don't overlap: "aa" in "aaaaa" is at 0 and 2.
	szText.Set("aaaaa");
	auiStarts.ReInit();
	cNavigation.FindOccurrences(3, 5, 1000, &auiStarts);
	AssertSize(2, auiStarts.NumElements());
	AssertSize(0, *auiStarts.Get(0));
	AssertSize(2, *auiStarts.Get(1));
	auiStarts.ReInit();
	cNavigation.FindOccurrences(0, 2, 1000, &auiStarts);
	AssertSize(1, auiStarts.NumElements());
	AssertSize(2, *auiStarts.Get(0));

	auiStarts.Kill();
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

	TestStatistics();
}

