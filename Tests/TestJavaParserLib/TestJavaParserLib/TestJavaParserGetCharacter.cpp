#include "BaseLib/TextParser.h"
#include "BaseLib/Numbers.h"
#include "TestLib/Assert.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseCharacter(char* sz, uint16* pc, uint16* piWidth)
{
	CTextParser		cParser;
	TRISTATE		tResult;

	cParser.Init(sz);
	tResult = cParser.GetCharacterLiteral(pc, true, piWidth, false);
	cParser.Kill();

	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void AssertCharacter(TRISTATE tResult, uint16 cActual, uint16 cExpected, uint16 iWidthActual, uint16 iWidthExpected)
{
	AssertTristate(TRITRUE, tResult);
	AssertShort(cExpected, cActual);
	AssertInt(iWidthExpected, iWidthActual);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetCharacterSimple(void)
{
	TRISTATE	tResult;
	uint16		c;
	uint16		iWidth;

	tResult = ParseCharacter("'A'", &c, &iWidth);
	AssertCharacter(tResult, c, 'A', iWidth, 1);

	tResult = ParseCharacter("'A';", &c, &iWidth);
	AssertCharacter(tResult, c, 'A', iWidth, 1);

	tResult = ParseCharacter("'0'", &c, &iWidth);
	AssertCharacter(tResult, c, '0', iWidth, 1);

	tResult = ParseCharacter("'a'", &c, &iWidth);
	AssertCharacter(tResult, c, 'a', iWidth, 1);

	tResult = ParseCharacter("'Â'", &c, &iWidth);
	AssertCharacter(tResult, c, 'Â', iWidth, 1);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetCharacterEscapeCode(void)
{
	TRISTATE	tResult;
	uint16		c;
	uint16		iWidth;

	tResult = ParseCharacter("'\\a'", &c, &iWidth);
	AssertCharacter(tResult, c, '\a', iWidth, 1);

	tResult = ParseCharacter("'\\b'", &c, &iWidth);
	AssertCharacter(tResult, c, '\b', iWidth, 1);

	tResult = ParseCharacter("'\\f'", &c, &iWidth);
	AssertCharacter(tResult, c, '\f', iWidth, 1);

	tResult = ParseCharacter("'\\n'", &c, &iWidth);
	AssertCharacter(tResult, c, '\n', iWidth, 1);

	tResult = ParseCharacter("'\\r'", &c, &iWidth);
	AssertCharacter(tResult, c, '\r', iWidth, 1);

	tResult = ParseCharacter("'\\t'", &c, &iWidth);
	AssertCharacter(tResult, c, '\t', iWidth, 1);

	tResult = ParseCharacter("'\\v'", &c, &iWidth);
	AssertCharacter(tResult, c, '\v', iWidth, 1);

	tResult = ParseCharacter("'\\\\'", &c, &iWidth);
	AssertCharacter(tResult, c, '\\', iWidth, 1);

	tResult = ParseCharacter("'\\\''", &c, &iWidth);
	AssertCharacter(tResult, c, '\'', iWidth, 1);

	tResult = ParseCharacter("'\\\"'", &c, &iWidth);
	AssertCharacter(tResult, c, '\"', iWidth, 1);

	tResult = ParseCharacter("'\\?'", &c, &iWidth);
	AssertCharacter(tResult, c, '\?', iWidth, 1);

	tResult = ParseCharacter("'\\0'", &c, &iWidth);
	AssertCharacter(tResult, c, 0, iWidth, 1);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetCharacterEscapeOctal(void)
{
	TRISTATE	tResult;
	uint16		c;
	uint16		iWidth;

	tResult = ParseCharacter("'\\00'", &c, &iWidth);
	AssertCharacter(tResult, c, 0, iWidth, 1);

	tResult = ParseCharacter("'\\01'", &c, &iWidth);
	AssertCharacter(tResult, c, 1, iWidth, 1);

	tResult = ParseCharacter("'\\10'", &c, &iWidth);
	AssertCharacter(tResult, c, 8, iWidth, 1);

	tResult = ParseCharacter("'\\377'", &c, &iWidth);
	AssertCharacter(tResult, c, 255, iWidth, 1);

	tResult = ParseCharacter("'\\00'\n", &c, &iWidth);
	AssertCharacter(tResult, c, 0, iWidth, 1);

	tResult = ParseCharacter("'\\01'\n", &c, &iWidth);
	AssertCharacter(tResult, c, 1, iWidth, 1);

	tResult = ParseCharacter("'\\10'\n", &c, &iWidth);
	AssertCharacter(tResult, c, 8, iWidth, 1);

	tResult = ParseCharacter("'\\377'\n", &c, &iWidth);
	AssertCharacter(tResult, c, 255, iWidth, 1);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetCharacterEscapeHexadecimal(void)
{
	TRISTATE	tResult;
	uint16		c;
	uint16		iWidth;

	tResult = ParseCharacter("'\\x0'", &c, &iWidth);
	AssertCharacter(tResult, c, 0, iWidth, 1);

	tResult = ParseCharacter("'\\x10'", &c, &iWidth);
	AssertCharacter(tResult, c, 16, iWidth, 1);

	tResult = ParseCharacter("'\\x01'", &c, &iWidth);
	AssertCharacter(tResult, c, 1, iWidth, 1);

	tResult = ParseCharacter("'\\xFFFF'", &c, &iWidth);
	AssertCharacter(tResult, c, 65535, iWidth, 2);

	tResult = ParseCharacter("'\\x100'", &c, &iWidth);
	AssertCharacter(tResult, c, 256, iWidth, 2);
	
	tResult = ParseCharacter("'\\x0';", &c, &iWidth);
	AssertCharacter(tResult, c, 0, iWidth, 1);

	tResult = ParseCharacter("'\\x10';", &c, &iWidth);
	AssertCharacter(tResult, c, 16, iWidth, 1);

	tResult = ParseCharacter("'\\x01';", &c, &iWidth);
	AssertCharacter(tResult, c, 1, iWidth, 1);

	tResult = ParseCharacter("'\\xFFFF';", &c, &iWidth);
	AssertCharacter(tResult, c, 65535, iWidth, 2);

	tResult = ParseCharacter("'\\x100';", &c, &iWidth);
	AssertCharacter(tResult, c, 256, iWidth, 2);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetCharacterEscapeUnicode(void)
{
	TRISTATE	tResult;
	uint16		c;
	uint16		iWidth;

	tResult = ParseCharacter("'\\u0000'", &c, &iWidth);
	AssertCharacter(tResult, c, 0, iWidth, 2);

	tResult = ParseCharacter("'\\u0041'", &c, &iWidth);
	AssertCharacter(tResult, c, 'A', iWidth, 2);

	tResult = ParseCharacter("'\\uFFFF'", &c, &iWidth);
	AssertCharacter(tResult, c, 65535, iWidth, 2);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetCharacter(void)
{
	BeginTests();

	TestJavaParserGetCharacterSimple();
	TestJavaParserGetCharacterEscapeCode();
	TestJavaParserGetCharacterEscapeOctal();
	TestJavaParserGetCharacterEscapeHexadecimal();
	TestJavaParserGetCharacterEscapeUnicode();

	TestStatistics();
}

