#include <stdlib.h>
#include <string.h>
#include "TestLib/Assert.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/ArrayElementNotFound.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStrCmp(void)
{
    int iResult;

    iResult = StringCompare("Hello", "Hello");
    AssertInt(0, iResult);

    iResult = StringCompare("", NULL);
    AssertInt(1, iResult);

    iResult = StringCompare(NULL, "");
    AssertInt(-1, iResult);

    iResult = StringCompare(NULL, NULL);
    AssertInt(0, iResult);

    iResult = StringCompare("", "");
    AssertInt(0, iResult);

    iResult = StringCompare("ABC", "XYZ");
    AssertInt(-1, iResult);

    iResult = StringCompare("xyz", "abc");
    AssertInt(1, iResult);

    iResult = StringCompare("abc", "abcd");
    AssertInt(-1, iResult);

    iResult = StringCompare("abcd", "abc");
    AssertInt(1, iResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStrICmp(void)
{
    int iResult;

    iResult = StringInsensitiveCompare("Hello", "hello");
    AssertInt(0, iResult);

    iResult = StringInsensitiveCompare("", NULL);
    AssertInt(1, iResult);

    iResult = StringInsensitiveCompare(NULL, "");
    AssertInt(-1, iResult);

    iResult = StringInsensitiveCompare(NULL, NULL);
    AssertInt(0, iResult);

    iResult = StringInsensitiveCompare("", "");
    AssertInt(0, iResult);

    iResult = StringInsensitiveCompare("ABC", "XYZ");
    AssertInt(-1, iResult);

    iResult = StringInsensitiveCompare("xyz", "abc");
    AssertInt(1, iResult);

    iResult = StringInsensitiveCompare("abc", "abcd");
    AssertInt(-1, iResult);

    iResult = StringInsensitiveCompare("abcd", "abc");
    AssertInt(1, iResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStrIStr(void)
{
    char    szFooBar[] = "FooBar";
    char*   sz;

    sz = StrIStr(szFooBar, "bar");
    AssertString("Bar", sz);

	sz = StrIStr(szFooBar, "FOO");
	AssertString("FooBar", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemICmp(void)
{
    char    sz1[] = "Andrew";
    char    sz2[] = "andrew";

    AssertInt(0, MemICmp(sz1, sz2, 6));

    strcpy(sz1, "Bndrew");
    AssertInt(1, MemICmp(sz1, sz2, 6));
    Pass();

    AssertInt(-1, MemICmp(sz2, sz1, 6));
    Pass();

    strcpy(sz1, "Cndrew");
    AssertInt(1, MemICmp(sz1, sz2, 6));
    AssertInt(-1, MemICmp(sz2, sz1, 6));

    AssertInt(0, MemICmp("\xC0", "\xC0", 1));
    AssertInt(0, MemICmp("Z\xE9", "z\xE9", 2));

    //Bytes compare as unsigned, the same as memcmp.  A char difference used to overflow.
    AssertInt(-1, MemICmp("\x00", "\x40", 1));
    AssertInt(-1, MemICmp("\x40", "\xC0", 1));
    AssertInt(-1, MemICmp("\x00", "\xC0", 1));
    AssertInt(1, MemICmp("\xC0", "\x00", 1));
    AssertInt(-1, MemICmp("\x0A", "\xF6", 1));
    AssertInt(1, MemICmp("\x80", "a", 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemSwp(void)
{
    char    sz1[] = "Andrew";
    char    sz2[] = "Claire";

    MemSwp(sz1, sz2, 6);
    AssertString("Claire", sz1);
    AssertString("Andrew", sz2);
    Pass();

    sz1[5] = '\0';
    sz2[5] = '\0';
    MemSwp(sz1, sz2, 5);
    AssertString("Andre", sz1);
    AssertString("Clair", sz2);
    Pass();

    sz1[4] = '\0';
    sz2[4] = '\0';
    MemSwp(sz1, sz2, 4);
    AssertString("Clai", sz1);
    AssertString("Andr", sz2);
    Pass();

    sz1[3] = '\0';
    sz2[3] = '\0';
    MemSwp(sz1, sz2, 3);
    AssertString("And", sz1);
    AssertString("Cla", sz2);
    Pass();

    sz1[2] = '\0';
    sz2[2] = '\0';
    MemSwp(sz1, sz2, 2);
    AssertString("Cl", sz1);
    AssertString("An", sz2);
    Pass();

    sz1[1] = '\0';
    sz2[1] = '\0';
    MemSwp(sz1, sz2, 1);
    AssertString("A", sz1);
    AssertString("C", sz2);
    Pass();

    MemSwp(sz1, sz2, 0);
    AssertString("A", sz1);
    AssertString("C", sz2);
    Pass();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStrRev(void)
{
    char    szAndrew[] = "Andrew";
    char    szCat[] = "cat";
    char    szMinus[] = "1-";

    StrRev(szCat);
    AssertString("tac", szCat);

    StrRev(szAndrew);
    AssertString("werdnA", szAndrew);

    StrRev(szMinus);
    AssertString("-1", szMinus);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIToA(void)
{
    char    sz[256];

    IntToString(sz, 256, 7, 10);
    AssertString("7", sz);

    IntToString(sz, 256, 456, 10);
    AssertString("456", sz);

    IntToString(sz, 256, 12, 10);
    AssertString("12", sz);

    IntToString(sz, 256, -1, 10);
    AssertString("-1", sz);

    IntToString(sz, 256, -32456, 10);
    AssertString("-32456", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestToLower(void)
{
	char	szTest[] = "In The Age of C:\\ to Fractions %^&222";
	char	szExpected[] = "in the age of c:\\ to fractions %^&222";

	ToLower(szTest);
	AssertString(szExpected, szTest);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestToUpper(void)
{
	char	szTest[] = "In The Age of C:\\ to Fractions %^&222";
	char	szExpected[] = "IN THE AGE OF C:\\ TO FRACTIONS %^&222";

	ToUpper(szTest);
	AssertString(szExpected, szTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemCmp(void)
{
    char    szAndrew[] = "Andrew";
    char    szANdrew[] = "ANdrew";
    char    szAndre[] = "Andre";
    int     iResult;

    iResult = MemCmp(szAndrew, 6, szANdrew, 6);
    AssertInt(1, iResult);
    iResult = MemCmp(szANdrew, 6, szAndrew, 6);
    AssertInt(-1, iResult);
    iResult = MemCmp(szAndrew, 6, szAndrew, 6);
    AssertInt(0, iResult);
    iResult = MemCmp(szAndre, 6, szAndrew, 6);
    AssertInt(-1, iResult);
    iResult = MemCmp(szAndrew, 6, szAndre, 6);
    AssertInt(1, iResult);
    iResult = MemCmp(szAndre, 6, szANdrew, 6);
    AssertInt(1, iResult);
    iResult = MemCmp(szANdrew, 6, szAndre, 6);
    AssertInt(-1, iResult);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlagsToString()
{
    char    sz[40];

    FlagsToString(sz, 40, (int32)0b00110010000000001111111101101010);
    AssertString("00110010 00000000 11111111 01101010", sz);

    FlagsToString(sz, 40, (int32)0b10111011110111101011100101101111);
    AssertString("10111011 11011110 10111001 01101111", sz);

    FlagsToString(sz, 40, (int16)0b1011101111011110);
    AssertString("10111011 11011110", sz);

    FlagsToString(sz, 40, (int8)0b11011110);
    AssertString("11011110", sz);

    FlagsToString(sz, 1, (int8)0b11011110);
    AssertString("", sz);

    FlagsToString(sz, 2, (int8)0b11011110);
    AssertString("1", sz);

    FlagsToString(sz, 6, (int8)0b11011110);
    AssertString("11011", sz);
    
    FlagsToString(sz, 8, (int8)0b11011110);
    AssertString("1101111", sz);

    FlagsToString(sz, 9, (int8)0b11011110);
    AssertString("11011110", sz);

    FlagsToString(sz, 10, (int8)0b11011110);
    AssertString("11011110", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharToString(void)
{
    char    sz[40];

    CharToString(sz, 40, 'A');
    AssertString("'A'", sz);

    CharToString(sz, 40, '\0');
    AssertString("'\\0'", sz);

    CharToString(sz, 40, '\n');
    AssertString("'\\n'", sz);

    CharToString(sz, 40, '\x7f');
    AssertString("'\\x7f'", sz);

    CharToString(sz, 40, '\x80');
    AssertString("'€'", sz);

    CharToString(sz, 1, '\x7f');
    AssertString("", sz);

    CharToString(sz, 2, '\x7f');
    AssertString("'", sz);

    CharToString(sz, 3, '\x7f');
    AssertString("'\\", sz);

    CharToString(sz, 4, '\x7f');
    AssertString("'\\x", sz);

    CharToString(sz, 5, '\x7f');
    AssertString("'\\x7", sz);

    CharToString(sz, 6, '\x7f');
    AssertString("'\\x7f", sz);

    CharToString(sz, 7, '\x7f');
    AssertString("'\\x7f'", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharToWideString(void)
{
    char    sz[40];

    CharToString(sz, 40, (char16)'A');
    AssertString("'\\u0041'", sz);

    CharToString(sz, 40, (char16)'\0');
    AssertString("'\\u0000'", sz);

    CharToString(sz, 7, (char16)0x7fff);
    AssertString("'\\u7ff", sz);

    CharToString(sz, 8, (char16)0x7fff);
    AssertString("'\\u7fff", sz);

    CharToString(sz, 9, (char16)0x7fff);
    AssertString("'\\u7fff'", sz);

    CharToString(sz, 10, (char16)0x7fff);
    AssertString("'\\u7fff'", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBoolToString()
{
    char    sz[40];

    BoolToString(sz, 40, true);
    AssertString("true", sz);

    BoolToString(sz, 40, false);
    AssertString("false", sz);

    BoolToString(sz, 1, false);
    AssertString("", sz);

    BoolToString(sz, 2, false);
    AssertString("f", sz);

    BoolToString(sz, 3, false);
    AssertString("fa", sz);

    BoolToString(sz, 4, false);
    AssertString("fal", sz);

    BoolToString(sz, 5, false);
    AssertString("fals", sz);

    BoolToString(sz, 6, false);
    AssertString("false", sz);

    BoolToString(sz, 7, false);
    AssertString("false", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFindChar(void)
{
	char		szDotted[] = "a.b.c";
	char		szGuard[] = "x.\0";
	const char*	sz;

	sz = FindChar(szDotted, '.');
	AssertPointer(&szDotted[1], (void*)sz);

	sz = FindChar(szDotted, '.', true);
	AssertPointer(&szDotted[3], (void*)sz);

	sz = FindChar(szDotted, 'z', true);
	AssertNull((void*)sz);

	//Forwards: not found, the terminator isn't found either, and no string.
	sz = FindChar(szDotted, 'z');
	AssertNull((void*)sz);

	sz = FindChar(szDotted, '\0');
	AssertNull((void*)sz);

	sz = FindChar(NULL, '.');
	AssertNull((void*)sz);

	sz = FindChar(NULL, '.', true);
	AssertNull((void*)sz);

	//Searching backwards in an empty string must not read the '.' before it.
	sz = FindChar(&szGuard[2], '.', true);
	AssertNull((void*)sz);

	sz = FindChar(&szGuard[2], '.');
	AssertNull((void*)sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFindCharIndex(void)
{
	char	szDotted[] = "a.b.c";
    char	szGuard[4];
	char	szEmpty[] = "\0.";
	size	uiIndex;

    szGuard[0] = 'a';
    szGuard[1] = '\0';
    szGuard[1] = '.';
    szGuard[1] = '\0';

	uiIndex = FindCharIndex('.', szDotted, 0);
	AssertSize(1, uiIndex);

	uiIndex = FindCharIndex('.', szDotted, 2);
	AssertSize(1, uiIndex);

	uiIndex = FindCharIndex('a', szDotted, 0);
	AssertSize(0, uiIndex);

	uiIndex = FindCharIndex('z', szDotted, 0);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, uiIndex);

	uiIndex = FindCharIndex('.', szGuard, 1);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, uiIndex);

	uiIndex = FindCharIndex('.', szEmpty, 0);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// StrLen is a natural length: a C string too long for a size is MAX_SIZE
// (strlen is the lsize length).
//
//////////////////////////////////////////////////////////////////////////
void TestStrLen(void)
{
	char*	sz;

	AssertSize(5, StrLen("hello"));
	AssertSize(0, StrLen(""));

	sz = (char*)malloc(70001);
	memset(sz, 'a', 70000);
	sz[70000] = '\0';
#ifdef SIZE_16
	AssertSize(MAX_SIZE, StrLen(sz));
#else
	AssertSize(70000, StrLen(sz));
#endif // SIZE_16
	AssertLSize(70000, strlen(sz));
	free(sz);
}


//////////////////////////////////////////////////////////////////////////
//
// MemICmpLarge and StrIStr past what a size counts.
//
//////////////////////////////////////////////////////////////////////////
void TestMemICmpLarge(void)
{
	char*	sz1;
	char*	sz2;
	lsize	uiLength;

	uiLength = 70000;
	sz1 = (char*)malloc(uiLength + 1);
	sz2 = (char*)malloc(uiLength + 1);
	memset(sz1, 'a', uiLength);
	memset(sz2, 'A', uiLength);
	sz1[uiLength] = '\0';
	sz2[uiLength] = '\0';

	AssertInt(0, MemICmpLarge(sz1, sz2, uiLength));
	sz2[69999] = 'B';
	AssertTrue(MemICmpLarge(sz1, sz2, uiLength) < 0);
	AssertInt(0, MemICmpLarge(sz1, sz2, 69999));
	AssertTrue(StrIStr(sz1, sz2) == NULL);
	sz2[69999] = 'A';
	AssertTrue(StrIStr(sz1, sz2) == sz1);
	sz2[4464] = 'B';
	AssertInt(0, MemICmpLarge(sz1, sz2, 4464));
	AssertTrue(MemICmpLarge(sz1, sz2, 4465) < 0);

	free(sz2);
	free(sz1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringHelper(void)
{
	BeginTests();

    TestStrCmp();
	TestStrICmp();
	TestStrIStr();
	TestStrRev();
	TestMemICmp();
	TestIToA();
	TestToLower();
	TestToUpper();
    TestMemSwp();
    TestStrLen();
    TestMemCmp();
    TestFlagsToString();
    TestCharToString();
    TestCharToWideString();
    TestBoolToString();
    TestFindChar();
    TestFindCharIndex();
	TestMemICmpLarge();

	TestStatistics();
}

